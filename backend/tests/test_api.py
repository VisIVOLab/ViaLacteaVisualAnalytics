"""
test_api.py – pytest-asyncio test suite for the VisIVO /v1/ API.

Covers:
  - Authentication (401 on missing/wrong token)
  - Health endpoint
  - File listing and header extraction
  - Dataset open (image and cube)
  - Session isolation (dataset from one session not visible in another)
  - Cube slice, subvolume, preview, moment, PV diagram
  - Image full and preview
  - Error handling for invalid inputs

Run with:
    cd backend
    pytest tests/ -v --asyncio-mode=auto
"""

from __future__ import annotations

import asyncio
import base64
import struct

import numpy as np
import pytest
import pytest_asyncio
from httpx import AsyncClient


# ── Helpers ───────────────────────────────────────────────────────────────────


def _decode_f32(b64: str) -> np.ndarray:
    """Decode a plain base64 float32 array."""
    return np.frombuffer(base64.b64decode(b64), dtype=np.float32)


def _decode_qt_zlib(b64: str) -> np.ndarray:
    """Decode a qt-zlib compressed float32 payload."""
    import zlib
    raw = base64.b64decode(b64)
    expected_len = struct.unpack(">I", raw[:4])[0]
    decompressed = zlib.decompress(raw[4:])
    assert len(decompressed) == expected_len
    return np.frombuffer(decompressed, dtype=np.float32)


# ── Auth tests ────────────────────────────────────────────────────────────────


@pytest.mark.asyncio
async def test_no_token_returns_401(client: AsyncClient) -> None:
    resp = await client.get("/v1/health")
    assert resp.status_code == 401


@pytest.mark.asyncio
async def test_wrong_token_returns_401(client: AsyncClient) -> None:
    resp = await client.get("/v1/health", headers={"X-Visivo-Token": "wrong-token"})
    assert resp.status_code == 401


# ── Health ────────────────────────────────────────────────────────────────────


@pytest.mark.asyncio
async def test_health_ok(client: AsyncClient, auth_headers: dict) -> None:
    resp = await client.get("/v1/health", headers=auth_headers)
    assert resp.status_code == 200
    data = resp.json()
    assert data["ok"] is True
    assert "workers" in data
    assert "active_sessions" in data
    assert "X-Request-ID" in resp.headers  # R8: request-id header present


# ── File operations ───────────────────────────────────────────────────────────


@pytest.mark.asyncio
async def test_list_files_home(client: AsyncClient, auth_headers: dict) -> None:
    resp = await client.get("/v1/files/list", headers=auth_headers)
    assert resp.status_code == 200
    data = resp.json()
    assert data["valid"] is True
    assert isinstance(data["entries"], list)


@pytest.mark.asyncio
async def test_list_files_invalid_path(client: AsyncClient, auth_headers: dict) -> None:
    resp = await client.get(
        "/v1/files/list",
        params={"path": "/nonexistent_path_xyz_12345"},
        headers=auth_headers,
    )
    assert resp.status_code == 200
    assert resp.json()["valid"] is False


@pytest.mark.asyncio
async def test_file_header_fits(
    client: AsyncClient, auth_headers: dict, synthetic_fits_image
) -> None:
    resp = await client.post(
        "/v1/files/header",
        json={"path": str(synthetic_fits_image)},
        headers=auth_headers,
    )
    assert resp.status_code == 200
    data = resp.json()
    assert data["valid"] is True
    cards_joined = " ".join(data["cards"])
    assert "NAXIS" in cards_joined
    assert "CTYPE1" in cards_joined


@pytest.mark.asyncio
async def test_file_header_nonexistent(client: AsyncClient, auth_headers: dict) -> None:
    resp = await client.post(
        "/v1/files/header",
        json={"path": "/does/not/exist.fits"},
        headers=auth_headers,
    )
    assert resp.json()["valid"] is False


# ── Dataset open ──────────────────────────────────────────────────────────────


@pytest.mark.asyncio
async def test_open_image(opened_image: dict) -> None:
    assert opened_image["dataset_id"].startswith("ds_")
    assert opened_image["session_id"] != ""


@pytest.mark.asyncio
async def test_open_cube(opened_cube: dict) -> None:
    assert opened_cube["dataset_id"].startswith("ds_")


@pytest.mark.asyncio
async def test_open_image_metadata(
    client: AsyncClient, auth_headers: dict, synthetic_fits_image
) -> None:
    resp = await client.post(
        "/v1/datasets/open",
        json={"path": str(synthetic_fits_image)},
        headers=auth_headers,
    )
    data = resp.json()
    assert data["kind"] == "image"
    assert data["width"] == 64
    assert data["height"] == 64
    assert data["depth"] == 1
    assert data["active_axes"] == 2
    assert data["ctype"][0] == "RA---SIN"


@pytest.mark.asyncio
async def test_open_cube_metadata(
    client: AsyncClient, auth_headers: dict, synthetic_fits_cube
) -> None:
    resp = await client.post(
        "/v1/datasets/open",
        json={"path": str(synthetic_fits_cube)},
        headers=auth_headers,
    )
    data = resp.json()
    assert data["kind"] == "cube"
    assert data["width"] == 64
    assert data["height"] == 64
    assert data["depth"] == 32
    assert data["active_axes"] == 3


@pytest.mark.asyncio
async def test_open_nonexistent_file(client: AsyncClient, auth_headers: dict) -> None:
    resp = await client.post(
        "/v1/datasets/open",
        json={"path": "/no/such/file.fits"},
        headers=auth_headers,
    )
    assert resp.json()["valid"] is False


# ── Session isolation ─────────────────────────────────────────────────────────


@pytest.mark.asyncio
async def test_session_isolation(
    client: AsyncClient,
    auth_headers: dict,
    synthetic_fits_cube,
) -> None:
    """A dataset opened in session A must NOT be accessible from session B."""
    import uuid

    # ── Session A: open a cube, get its dataset_id and session_id ────────────
    resp_a = await client.post(
        "/v1/datasets/open",
        json={"path": str(synthetic_fits_cube)},
        headers=auth_headers,  # no session → anonymous session A
    )
    data_a = resp_a.json()
    assert data_a["valid"]
    dataset_id_a = data_a["dataset_id"]
    session_id_a = data_a["session_id"]

    # ── Session B: supply a brand-new random session_id ───────────────────────
    # The registry will create a fresh session because this UUID is unknown.
    session_id_b = str(uuid.uuid4())
    headers_b = {**auth_headers, "X-Visivo-Session": session_id_b}

    # Try to access session A's dataset from session B – must be 404.
    resp = await client.post(
        "/v1/cube/slice",
        json={"dataset_id": dataset_id_a, "axis": "z", "index": 0},
        headers=headers_b,
    )
    assert resp.status_code == 404, (
        f"Expected 404 for cross-session access, got {resp.status_code}: {resp.text}"
    )


# ── Cube endpoints ────────────────────────────────────────────────────────────


@pytest.mark.asyncio
async def test_cube_slice_z(client: AsyncClient, opened_cube: dict) -> None:
    resp = await client.post(
        "/v1/cube/slice",
        json={"dataset_id": opened_cube["dataset_id"], "axis": "z", "index": 0},
        headers=opened_cube["headers"],
    )
    assert resp.status_code == 200
    data = resp.json()
    assert data["valid"] is True
    assert data["width"] == 64
    assert data["height"] == 64
    assert data["compression"] == "qt-zlib"
    # Decode and verify shape.
    arr = _decode_qt_zlib(data["data_base64"])
    assert arr.shape == (64 * 64,)


@pytest.mark.asyncio
async def test_cube_slice_out_of_range(client: AsyncClient, opened_cube: dict) -> None:
    resp = await client.post(
        "/v1/cube/slice",
        json={"dataset_id": opened_cube["dataset_id"], "axis": "z", "index": 9999},
        headers=opened_cube["headers"],
    )
    assert resp.json()["valid"] is False


@pytest.mark.asyncio
async def test_cube_subvolume(client: AsyncClient, opened_cube: dict) -> None:
    resp = await client.post(
        "/v1/cube/subvolume",
        json={
            "dataset_id": opened_cube["dataset_id"],
            "x0": 10, "x1": 20, "y0": 10, "y1": 20, "z0": 5, "z1": 10,
        },
        headers=opened_cube["headers"],
    )
    assert resp.status_code == 200
    data = resp.json()
    assert data["valid"] is True
    assert data["width"] == 11
    assert data["height"] == 11
    assert data["depth"] == 6


@pytest.mark.asyncio
async def test_cube_preview(client: AsyncClient, opened_cube: dict) -> None:
    resp = await client.post(
        "/v1/cube/preview",
        json={"dataset_id": opened_cube["dataset_id"], "downsample": 2},
        headers=opened_cube["headers"],
    )
    assert resp.status_code == 200
    data = resp.json()
    assert data["valid"] is True
    assert data["depth"] == 16   # 32 // 2
    assert data["width"] == 32   # 64 // 2


@pytest.mark.asyncio
@pytest.mark.parametrize("order", [0, 1, 2, 6, 8, 10])
async def test_moment_map_orders(
    client: AsyncClient, opened_cube: dict, order: int
) -> None:
    resp = await client.post(
        "/v1/products/moment",
        json={
            "dataset_id": opened_cube["dataset_id"],
            "moment_order": order,
            "channel_start": 10,
            "channel_end": 20,
            "mask_enabled": False,
            "threshold_value": 0.0,
        },
        headers=opened_cube["headers"],
    )
    assert resp.status_code == 200
    data = resp.json()
    assert data["valid"] is True, f"Moment {order} failed: {data['error']}"
    assert data["width"] == 64
    assert data["height"] == 64
    arr = _decode_f32(data["data_base64"])
    assert arr.shape == (64 * 64,)
    # At least some finite values.
    assert np.isfinite(arr).any()
    assert "spectral_axis_type" in data
    assert "moment_unit" in data


@pytest.mark.asyncio
async def test_moment_with_mask(client: AsyncClient, opened_cube: dict) -> None:
    resp = await client.post(
        "/v1/products/moment",
        json={
            "dataset_id": opened_cube["dataset_id"],
            "moment_order": 0,
            "channel_start": 0,
            "channel_end": 31,
            "mask_enabled": True,
            "threshold_value": 3.0,
        },
        headers=opened_cube["headers"],
    )
    data = resp.json()
    assert data["valid"] is True
    arr = _decode_f32(data["data_base64"])
    # With mask only the bright emission-line region should contribute.
    assert np.nansum(arr) > 0


@pytest.mark.asyncio
async def test_cube_pv(client: AsyncClient, opened_cube: dict) -> None:
    resp = await client.post(
        "/v1/cube/pv",
        json={
            "dataset_id": opened_cube["dataset_id"],
            "vertices": [[10, 32], [54, 32]],
            "width_pixels": 3,
        },
        headers=opened_cube["headers"],
    )
    assert resp.status_code == 200
    data = resp.json()
    assert data["valid"] is True
    assert data["depth"] == 32
    assert data["valid_samples"] > 0
    assert data["compression"] == "qt-zlib"
    assert "spectral_axis_type" in data


@pytest.mark.asyncio
async def test_moment_task_endpoint(client: AsyncClient, opened_cube: dict) -> None:
    create_resp = await client.post(
        "/v1/tasks/moment",
        json={
            "dataset_id": opened_cube["dataset_id"],
            "moment_order": 0,
            "channel_start": 4,
            "channel_end": 12,
            "mask_enabled": False,
            "threshold_value": 0.0,
        },
        headers=opened_cube["headers"],
    )
    assert create_resp.status_code == 200
    task = create_resp.json()
    assert task["valid"] is True
    assert task["task_id"].startswith("task_")

    for _ in range(20):
        status_resp = await client.get(f"/v1/tasks/{task['task_id']}", headers=opened_cube["headers"])
        assert status_resp.status_code == 200
        status_data = status_resp.json()
        if status_data["status"] == "completed":
            assert status_data["result"]["valid"] is True
            assert "data_base64" in status_data["result"]
            break
        await asyncio.sleep(0.05)
    else:
        pytest.fail("moment task did not complete in time")


@pytest.mark.asyncio
async def test_pv_task_endpoint(client: AsyncClient, opened_cube: dict) -> None:
    create_resp = await client.post(
        "/v1/tasks/pv",
        json={
            "dataset_id": opened_cube["dataset_id"],
            "vertices": [[10, 32], [54, 32]],
            "width_pixels": 3,
        },
        headers=opened_cube["headers"],
    )
    assert create_resp.status_code == 200
    task = create_resp.json()
    assert task["valid"] is True
    assert task["task_id"].startswith("task_")

    for _ in range(20):
        status_resp = await client.get(f"/v1/tasks/{task['task_id']}", headers=opened_cube["headers"])
        assert status_resp.status_code == 200
        status_data = status_resp.json()
        if status_data["status"] == "completed":
            assert status_data["result"]["valid"] is True
            assert "data_base64" in status_data["result"]
            assert status_data["result"]["valid_samples"] > 0
            break
        await asyncio.sleep(0.05)
    else:
        pytest.fail("pv task did not complete in time")


@pytest.mark.asyncio
async def test_pv_task_cache_hit(client: AsyncClient, opened_cube: dict) -> None:
    payload = {
        "dataset_id": opened_cube["dataset_id"],
        "vertices": [[10, 32], [54, 32]],
        "width_pixels": 3,
    }
    first_resp = await client.post("/v1/tasks/pv", json=payload, headers=opened_cube["headers"])
    assert first_resp.status_code == 200
    first_task = first_resp.json()

    for _ in range(20):
        status_resp = await client.get(f"/v1/tasks/{first_task['task_id']}", headers=opened_cube["headers"])
        status_data = status_resp.json()
        if status_data["status"] == "completed":
            break
        await asyncio.sleep(0.05)
    else:
        pytest.fail("first pv task did not complete in time")

    second_resp = await client.post("/v1/tasks/pv", json=payload, headers=opened_cube["headers"])
    assert second_resp.status_code == 200
    second_task = second_resp.json()

    for _ in range(20):
        status_resp = await client.get(f"/v1/tasks/{second_task['task_id']}", headers=opened_cube["headers"])
        status_data = status_resp.json()
        if status_data["status"] == "completed":
            assert status_data["cache_hit"] is True
            assert status_data["result"]["valid"] is True
            break
        await asyncio.sleep(0.05)
    else:
        pytest.fail("second pv task did not complete in time")


@pytest.mark.asyncio
async def test_cube_pv_too_few_vertices(client: AsyncClient, opened_cube: dict) -> None:
    resp = await client.post(
        "/v1/cube/pv",
        json={
            "dataset_id": opened_cube["dataset_id"],
            "vertices": [[10, 10]],
            "width_pixels": 1,
        },
        headers=opened_cube["headers"],
    )
    assert resp.json()["valid"] is False


# ── Image endpoints ───────────────────────────────────────────────────────────


@pytest.mark.asyncio
async def test_image_full(client: AsyncClient, opened_image: dict) -> None:
    resp = await client.post(
        "/v1/image/full",
        json={"dataset_id": opened_image["dataset_id"]},
        headers=opened_image["headers"],
    )
    assert resp.status_code == 200
    data = resp.json()
    assert data["valid"] is True
    assert data["width"] == 64
    assert data["height"] == 64
    assert data["is_preview"] is False
    assert data["compression"] == "qt-zlib"
    arr = _decode_qt_zlib(data["data_base64"])
    assert arr.shape == (64 * 64,)


@pytest.mark.asyncio
async def test_image_preview_downsampled(client: AsyncClient, opened_image: dict) -> None:
    resp = await client.post(
        "/v1/image/preview",
        json={"dataset_id": opened_image["dataset_id"], "max_longest_side": 32},
        headers=opened_image["headers"],
    )
    data = resp.json()
    assert data["valid"] is True
    assert data["is_preview"] is True
    assert max(data["width"], data["height"]) <= 32


@pytest.mark.asyncio
async def test_image_full_wrong_kind(client: AsyncClient, opened_cube: dict) -> None:
    """Passing a cube dataset_id to /image/full must fail gracefully."""
    resp = await client.post(
        "/v1/image/full",
        json={"dataset_id": opened_cube["dataset_id"]},
        headers=opened_cube["headers"],
    )
    data = resp.json()
    assert data["valid"] is False


# ── Unknown dataset_id ────────────────────────────────────────────────────────


@pytest.mark.asyncio
async def test_unknown_dataset_id(client: AsyncClient, auth_headers: dict) -> None:
    resp = await client.post(
        "/v1/cube/slice",
        json={"dataset_id": "ds_doesnotexist", "axis": "z", "index": 0},
        headers=auth_headers,
    )
    assert resp.status_code == 404


def test_task_registry_ttl_eviction() -> None:
    from backend.app.tasks import TaskRegistry

    registry = TaskRegistry(ttl_seconds=1)
    task = registry.create("pv")
    record = registry.get(task.task_id)
    assert record is not None
    record.last_touched_monotonic -= 5.0
    assert registry.get(task.task_id) is None
