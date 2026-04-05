from __future__ import annotations

import threading
import uuid
from dataclasses import dataclass, field
from datetime import datetime, timezone
from typing import Any


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


class TaskRegistry:
    def __init__(self) -> None:
        self._tasks: dict[str, TaskRecord] = {}
        self._lock = threading.Lock()

    def create(self, operation: str) -> TaskRecord:
        record = TaskRecord(task_id=f"task_{uuid.uuid4().hex[:16]}", operation=operation)
        with self._lock:
            self._tasks[record.task_id] = record
        return record

    def get(self, task_id: str) -> TaskRecord | None:
        with self._lock:
            return self._tasks.get(task_id)

    def update(self, task_id: str, **changes: Any) -> TaskRecord | None:
        with self._lock:
            record = self._tasks.get(task_id)
            if record is None:
                return None
            for key, value in changes.items():
                setattr(record, key, value)
            record.updated_at = _now_iso()
            return record


TASKS = TaskRegistry()
