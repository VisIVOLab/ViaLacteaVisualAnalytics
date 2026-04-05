"""
auth.py – R1: Bearer-token authentication for the VisIVO backend.

A token is loaded from the VISIVO_TOKEN environment variable.
If the variable is absent a cryptographically-random token is generated
at startup, printed to stdout AND written to ~/.visivo_token so the
Qt client can read it automatically.

Every API route that uses `Depends(verify_token)` will return 401
if the caller omits or supplies the wrong `X-Visivo-Token` header.
"""

from __future__ import annotations

import logging
import os
import secrets
from pathlib import Path

from fastapi import HTTPException, Security, status
from fastapi.security import APIKeyHeader

logger = logging.getLogger("visivo.auth")

_TOKEN_FILE = Path.home() / ".visivo_token"
_HEADER_NAME = "X-Visivo-Token"

_api_key_header = APIKeyHeader(name=_HEADER_NAME, auto_error=False)


def _resolve_token() -> str:
    """Return the active bearer token, generating one if necessary."""
    env_token = os.environ.get("VISIVO_TOKEN", "").strip()
    if env_token:
        logger.info("[auth] Using VISIVO_TOKEN from environment.")
        return env_token

    generated = secrets.token_urlsafe(32)
    try:
        _TOKEN_FILE.write_text(generated + "\n")
        _TOKEN_FILE.chmod(0o600)
        logger.info("[auth] Token written to %s (mode 600).", _TOKEN_FILE)
    except OSError as exc:
        logger.warning("[auth] Could not write token file: %s", exc)

    print(  # noqa: T201 – intentional: operators need to see the token at startup
        f"\n[VisIVO] Backend token: {generated}\n"
        f"[VisIVO] Or set: export VISIVO_TOKEN={generated}\n"
    )
    return generated


# Module-level singleton – resolved once at import time.
TOKEN: str = _resolve_token()


async def verify_token(
    x_visivo_token: str | None = Security(_api_key_header),
) -> None:
    """FastAPI dependency: raise 401 if the request token is wrong or absent."""
    if x_visivo_token is None or not secrets.compare_digest(x_visivo_token, TOKEN):
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Missing or invalid X-Visivo-Token header.",
            headers={"WWW-Authenticate": f'ApiKey realm="{_HEADER_NAME}"'},
        )
