from __future__ import annotations

import logging
import os
import threading
import time
import uuid
from dataclasses import dataclass, field
from datetime import datetime, timezone
from typing import Any

logger = logging.getLogger("visivo.tasks")

_TASK_TTL_SECONDS = max(1, int(os.environ.get("VISIVO_TASK_TTL_SECONDS", "1800")))


def _now_iso() -> str:
    return datetime.now(tz=timezone.utc).isoformat()


@dataclass
class TaskRecord:
    task_id: str
    operation: str
    status: str = "pending"
    created_at: str = field(default_factory=_now_iso)
    updated_at: str = field(default_factory=_now_iso)
    progress: float = 0.0
    result: dict[str, Any] | None = None
    error: str = ""
    cache_hit: bool = False
    last_touched_monotonic: float = field(default_factory=time.monotonic, repr=False)


class TaskRegistry:
    def __init__(self, ttl_seconds: int = _TASK_TTL_SECONDS) -> None:
        self._tasks: dict[str, TaskRecord] = {}
        self._lock = threading.Lock()
        self._ttl_seconds = max(1, int(ttl_seconds))

    def _evict_expired_locked(self) -> int:
        now = time.monotonic()
        expired_task_ids = [
            task_id
            for task_id, record in self._tasks.items()
            if (now - record.last_touched_monotonic) > self._ttl_seconds
        ]
        for task_id in expired_task_ids:
            record = self._tasks.pop(task_id)
            logger.info(
                "[tasks] evicted task_id=%s operation=%s age_seconds=%.1f",
                task_id,
                record.operation,
                now - record.last_touched_monotonic,
            )
        return len(expired_task_ids)

    def create(self, operation: str) -> TaskRecord:
        record = TaskRecord(task_id=f"task_{uuid.uuid4().hex[:16]}", operation=operation)
        with self._lock:
            self._evict_expired_locked()
            self._tasks[record.task_id] = record
        return record

    def get(self, task_id: str) -> TaskRecord | None:
        with self._lock:
            self._evict_expired_locked()
            record = self._tasks.get(task_id)
            if record is None:
                return None
            record.last_touched_monotonic = time.monotonic()
            return record

    def update(self, task_id: str, **changes: Any) -> TaskRecord | None:
        with self._lock:
            self._evict_expired_locked()
            record = self._tasks.get(task_id)
            if record is None:
                return None
            for key, value in changes.items():
                setattr(record, key, value)
            record.updated_at = _now_iso()
            record.last_touched_monotonic = time.monotonic()
            return record

    def stats(self) -> dict[str, Any]:
        with self._lock:
            self._evict_expired_locked()
            return {
                "entries": len(self._tasks),
                "ttl_enabled": True,
                "ttl_seconds": self._ttl_seconds,
            }


TASKS = TaskRegistry()
