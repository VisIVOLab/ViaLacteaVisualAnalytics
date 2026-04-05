from __future__ import annotations

import hashlib
import json
import logging
import os
import threading
from collections import OrderedDict
from dataclasses import dataclass
from datetime import datetime, timezone
from typing import Any

logger = logging.getLogger("visivo.cache")


@dataclass
class CachedProductRecord:
    key: str
    operation: str
    dataset_path: str
    parameter_hash: str
    payload: dict[str, Any]
    scientific_metadata: dict[str, Any]
    provenance_metadata: dict[str, Any]
    created_at: str


class ProductCache:
    def __init__(self, max_entries: int = 32) -> None:
        self._max_entries = max(1, max_entries)
        self._entries: OrderedDict[str, CachedProductRecord] = OrderedDict()
        self._lock = threading.Lock()

    @staticmethod
    def make_key(dataset_path: str, operation: str, params: dict[str, Any]) -> tuple[str, str]:
        encoded = json.dumps(params, sort_keys=True, separators=(",", ":"), default=str)
        param_hash = hashlib.sha256(encoded.encode("utf-8")).hexdigest()[:16]
        return f"{operation}:{dataset_path}:{param_hash}", param_hash

    def get(self, key: str) -> CachedProductRecord | None:
        with self._lock:
            record = self._entries.get(key)
            if record is None:
                logger.info("[cache] miss key=%s", key)
                return None
            self._entries.move_to_end(key)
            logger.info("[cache] hit key=%s op=%s", key, record.operation)
            return record

    def put(
        self,
        *,
        key: str,
        operation: str,
        dataset_path: str,
        parameter_hash: str,
        payload: dict[str, Any],
        scientific_metadata: dict[str, Any],
        provenance_metadata: dict[str, Any],
    ) -> CachedProductRecord:
        record = CachedProductRecord(
            key=key,
            operation=operation,
            dataset_path=dataset_path,
            parameter_hash=parameter_hash,
            payload=payload,
            scientific_metadata=scientific_metadata,
            provenance_metadata=provenance_metadata,
            created_at=datetime.now(tz=timezone.utc).isoformat(),
        )
        with self._lock:
            self._entries[key] = record
            self._entries.move_to_end(key)
            while len(self._entries) > self._max_entries:
                self._entries.popitem(last=False)
        logger.info("[cache] insert key=%s op=%s", key, operation)
        return record

    def stats(self) -> dict[str, Any]:
        with self._lock:
            return {
                "entries": len(self._entries),
                "max_entries": self._max_entries,
            }


PRODUCT_CACHE = ProductCache(max_entries=int(os.environ.get("VISIVO_PRODUCT_CACHE_ENTRIES", "32")))
