"""
sessions.py – R3: Per-session dataset registry with TTL eviction.

Each call to POST /v1/datasets/open returns a dataset_id that belongs
to a Session.  Sessions are identified by a UUID returned in the
OpenDatasetResponse and must be included in the X-Visivo-Session header
of all subsequent requests for that dataset.

If the session header is absent a default "anonymous" session is used
for backward compatibility with the current Qt client, but a warning is
emitted.  Future client versions should always send the session header.

Stale sessions (idle beyond VISIVO_SESSION_TTL seconds, default 1800)
are evicted by a background daemon thread.
"""

from __future__ import annotations

import logging
import os
import threading
import time
import uuid
from pathlib import Path
from typing import Any

logger = logging.getLogger("visivo.sessions")

_TTL_SECONDS = int(os.environ.get("VISIVO_SESSION_TTL", "1800"))
_ANON_SESSION_ID = "anonymous"


class Session:
    """Holds all datasets opened by a single client connection."""

    def __init__(self, session_id: str) -> None:
        self.session_id = session_id
        # dataset_id → {path, kind, width, height, depth, …}
        self.datasets: dict[str, dict[str, Any]] = {}
        self.created_at: float = time.monotonic()
        self.last_accessed: float = time.monotonic()

    def touch(self) -> None:
        self.last_accessed = time.monotonic()

    def idle_seconds(self) -> float:
        return time.monotonic() - self.last_accessed


class SessionRegistry:
    """Thread-safe registry of active sessions with background TTL eviction."""

    def __init__(self, ttl_seconds: int = _TTL_SECONDS) -> None:
        self._sessions: dict[str, Session] = {}
        self._lock = threading.Lock()
        self._ttl = ttl_seconds
        self._start_eviction_thread()

    # ── Public API ────────────────────────────────────────────────────────────

    def create(self) -> Session:
        """Create a new session and register it."""
        session_id = str(uuid.uuid4())
        session = Session(session_id)
        with self._lock:
            self._sessions[session_id] = session
        logger.info("[sessions] created session_id=%s", session_id)
        return session

    def get(self, session_id: str) -> Session | None:
        """Return the session or None if it does not exist / has been evicted."""
        with self._lock:
            session = self._sessions.get(session_id)
            if session is not None:
                session.touch()
            return session

    def get_or_create_anon(self) -> Session:
        """Return (or lazily create) the anonymous backward-compat session."""
        with self._lock:
            if _ANON_SESSION_ID not in self._sessions:
                self._sessions[_ANON_SESSION_ID] = Session(_ANON_SESSION_ID)
                logger.warning(
                    "[sessions] Created anonymous session (no X-Visivo-Session header). "
                    "Multi-user isolation is disabled for this request."
                )
            session = self._sessions[_ANON_SESSION_ID]
            session.touch()
            return session

    def stats(self) -> dict[str, Any]:
        with self._lock:
            return {
                "active_sessions": len(self._sessions),
                "ttl_seconds": self._ttl,
                "sessions": [
                    {
                        "session_id": s.session_id,
                        "datasets": len(s.datasets),
                        "idle_seconds": round(s.idle_seconds()),
                    }
                    for s in self._sessions.values()
                ],
            }

    # ── Eviction ─────────────────────────────────────────────────────────────

    def _evict_stale(self) -> None:
        while True:
            time.sleep(min(300, self._ttl // 2))
            with self._lock:
                stale = [
                    sid
                    for sid, s in self._sessions.items()
                    if sid != _ANON_SESSION_ID and s.idle_seconds() > self._ttl
                ]
            for sid in stale:
                with self._lock:
                    self._sessions.pop(sid, None)
                logger.info("[sessions] evicted stale session_id=%s", sid)

    def _start_eviction_thread(self) -> None:
        t = threading.Thread(target=self._evict_stale, daemon=True, name="visivo-session-eviction")
        t.start()


# Module-level singleton.
REGISTRY: SessionRegistry = SessionRegistry()


# ── FastAPI dependency ────────────────────────────────────────────────────────

from fastapi import Header  # noqa: E402 – placed here to avoid circular imports


async def get_session(x_visivo_session: str | None = Header(None)) -> Session:
    """
    FastAPI dependency: resolve (or lazily create) the caller's session.

    The Qt client should send the session_id received in OpenDatasetResponse
    as the X-Visivo-Session request header.  If absent the anonymous session
    is used with a warning.
    """
    if x_visivo_session is None:
        return REGISTRY.get_or_create_anon()

    session = REGISTRY.get(x_visivo_session)
    if session is None:
        # Session expired or invalid – create a fresh one so the client can
        # recover by re-opening datasets rather than crashing.
        logger.warning("[sessions] session_id=%s not found, creating new session.", x_visivo_session)
        session = REGISTRY.create()
    return session
