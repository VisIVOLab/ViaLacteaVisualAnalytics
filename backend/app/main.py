"""
main.py – VisIVO Backend v1

Changes vs original (next-server branch):
  R1  Auth         – every route requires X-Visivo-Token (see auth.py)
  R2  Concurrency  – CPU-bound work runs in a ProcessPoolExecutor so the
                     ASGI event-loop is never blocked
  R3  Sessions     – datasets are stored per-session; the client must echo
                     the X-Visivo-Session header returned in /v1/datasets/open
  R4  Lazy I/O     – worker functions open FITS with memmap=True and read
                     only the required pixels (see compute.py)
  R8  API quality  – all routes prefixed /v1/, standardised error schema,
                     X-Request-ID header on every response

The legacy un-versioned paths (/health, /files/*, …) are intentionally
removed to enforce /v1/ from the start. The Qt client must be updated to
prepend /v1/ to every request URL (Settings dialog → backend URL field).
"""

from __future__ import annotations

import asyncio
import logging
import os
import uuid
from concurrent.futures import ProcessPoolExecutor
from datetime import datetime, timezone
from pathlib import Path
from typing import Any

import numpy as np
from astropy.io import fits
from fastapi import Depends, FastAPI, Header, HTTPException, Query, Request, Response, status
from fastapi.responses import JSONResponse
from pydantic import BaseModel

from .auth import verify_token
from .sessions import REGISTRY, Session, get_session

# ── Logging ───────────────────────────────────────────────────────────────────

logging.basicConfig(level=logging.INFO, format="%(asctime)s %(name)s %(levelname)s %(message)s")
logger = logging.getLogger("visivo.backend")

# ── Process pool (R2) ─────────────────────────────────────────────────────────

_WORKERS = int(os.environ.get("VISIVO_WORKERS", str(os.cpu_count() or 4)))
_POOL = ProcessPoolExecutor(max_workers=_WORKERS, initializer=_pool_initializer if False else None)


def _pool_initializer() -> None:  # pragma: no cover
    """Silence VTK startup noise in worker processes."""
    import logging as _l
    _l.getLogger("vtkmodules").setLevel(_l.ERROR)


_POOL = ProcessPoolExecutor(max_workers=_WORKERS)

# ── FastAPI app ───────────────────────────────────────────────────────────────

app = FastAPI(
    title="VisIVO Backend",
    version="1.0.0",
    description="Astrophysical data visualization backend for ViaLactea Visual Analytics.",
    docs_url="/v1/docs",
    redoc_url="/v1/redoc",
    openapi_url="/v1/openapi.json",
)

_FITS_SUFFIXES = {".fits", ".fit", ".fts"}

# ── VTK availability check (R startup) ───────────────────────────────────────

_VTK_AVAILABLE = False
try:
    import vtk as _vtk  # noqa: F401
    _VTK_AVAILABLE = True
except ImportError:
    logger.warning(
        "[startup] VTK Python bindings not available – "
        "/v1/products/isosurface will return 503."
    )

# ── Middleware: X-Request-ID (R8) ─────────────────────────────────────────────


@app.middleware("http")
async def attach_request_id(request: Request, call_next):
    request_id = str(uuid.uuid4())
    response: Response = await call_next(request)
    response.headers["X-Request-ID"] = request_id
    return response


# ── Standard error schema (R8) ────────────────────────────────────────────────


class ErrorDetail(BaseModel):
    error: str
    detail: str
    request_id: str = ""


def _error(detail: str, status_code: int = 400, error: str = "BadRequest") -> JSONResponse:
    return JSONResponse(
        status_code=status_code,
        content=ErrorDetail(error=error, detail=detail).model_dump(),
    )


# ── Pydantic models ───────────────────────────────────────────────────────────


class FileEntry(BaseModel):
    name: str
    path: str
    type: str
    size: int = 0
    modified_time: str = ""
    is_fits: bool = False


class FilesListResponse(BaseModel):
    valid: bool
    error: str
    current_path: str = ""
    entries: list[FileEntry]


class FileHeaderRequest(BaseModel):
    path: str


class FileHeaderResponse(BaseModel):
    valid: bool
    error: str
    cards: list[str]


class OpenDatasetRequest(BaseModel):
    path: str


class OpenDatasetResponse(BaseModel):
    valid: bool
    error: str
    dataset_id: str = ""
    session_id: str = ""           # R3: client must echo as X-Visivo-Session
    kind: str = ""
    active_axes: int = 0
    width: int = 0
    height: int = 0
    depth: int = 0
    degenerate_axes_summary: str = ""
    spacing: list[float] = [1.0, 1.0, 1.0]
    origin: list[float] = [0.0, 0.0, 0.0]
    ctype: list[str] = ["", "", ""]
    cunit: list[str] = ["", "", ""]
    crval: list[float] = [0.0, 0.0, 0.0]
    crpix: list[float] = [1.0, 1.0, 1.0]
    cdelt: list[float] = [1.0, 1.0, 1.0]


class MomentProductRequest(BaseModel):
    dataset_id: str
    moment_order: int
    channel_start: int = 0
    channel_end: int = 0
    mask_enabled: bool = False
    threshold_value: float = 0.0


class MomentProductResponse(BaseModel):
    valid: bool
    error: str
    width: int = 0
    height: int = 0
    scalar_type: str = ""
    range_min: float = 0.0
    range_max: float = 0.0
    data_base64: str = ""
    moment_unit: str = ""
    bunit: str = ""
    spectral_axis_type: str = ""
    spectral_axis_unit: str = ""


class CubePreviewRequest(BaseModel):
    dataset_id: str
    downsample: int = 1


class CubePreviewResponse(BaseModel):
    valid: bool
    error: str
    width: int = 0
    height: int = 0
    depth: int = 0
    scalar_type: str = ""
    range_min: float = 0.0
    range_max: float = 0.0
    data_base64: str = ""


class CubeSliceRequest(BaseModel):
    dataset_id: str
    axis: str
    index: int


class CubeSliceResponse(BaseModel):
    valid: bool
    error: str
    width: int = 0
    height: int = 0
    scalar_type: str = ""
    range_min: float = 0.0
    range_max: float = 0.0
    compression: str = ""
    data_base64: str = ""


class CubeSubvolumeRequest(BaseModel):
    dataset_id: str
    x0: int
    x1: int
    y0: int
    y1: int
    z0: int
    z1: int


class CubeSubvolumeResponse(BaseModel):
    valid: bool
    error: str
    width: int = 0
    height: int = 0
    depth: int = 0
    scalar_type: str = ""
    data_base64: str = ""


class CubePvRequest(BaseModel):
    dataset_id: str
    vertices: list[list[int]]
    width_pixels: int = 1


class CubePvResponse(BaseModel):
    valid: bool
    error: str
    num_samples: int = 0
    depth: int = 0
    scalar_type: str = ""
    compression: str = ""
    positions_base64: str = ""
    data_base64: str = ""
    computed_on: str = ""
    width_pixels: int = 1
    vertex_count: int = 0
    total_length: float = 0.0
    valid_samples: int = 0


class ImageFullRequest(BaseModel):
    dataset_id: str


class ImagePreviewRequest(BaseModel):
    dataset_id: str
    max_longest_side: int = 1536


class ImageFullResponse(BaseModel):
    valid: bool
    error: str
    width: int = 0
    height: int = 0
    full_width: int = 0
    full_height: int = 0
    scalar_type: str = ""
    range_min: float = 0.0
    range_max: float = 0.0
    is_preview: bool = False
    preview_scale_factor: float = 1.0
    compression: str = ""
    data_base64: str = ""


class IsosurfaceProductRequest(BaseModel):
    dataset_id: str
    threshold: float


class IsosurfaceProductResponse(BaseModel):
    valid: bool
    error: str
    compression: str = ""
    points_base64: str = ""
    polys_base64: str = ""
    num_points: int = 0
    num_polys: int = 0


# ── Internal helpers ──────────────────────────────────────────────────────────


def _normalize_path(raw: str) -> Path:
    if not raw:
        return Path.home()
    return Path(raw).expanduser().resolve()


def _is_fits(path: Path) -> bool:
    return path.is_file() and path.suffix.lower() in _FITS_SUFFIXES


async def _run(fn, *args) -> Any:
    """Run a CPU-bound function in the process pool without blocking the loop."""
    loop = asyncio.get_event_loop()
    return await loop.run_in_executor(_POOL, fn, *args)


def _require_dataset(session: Session, dataset_id: str) -> dict[str, Any]:
    entry = session.datasets.get(dataset_id)
    if not entry:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=f"Unknown dataset_id '{dataset_id}'. Open the dataset first via /v1/datasets/open.",
        )
    return entry


def _require_cube_path(session: Session, dataset_id: str) -> str:
    entry = _require_dataset(session, dataset_id)
    if entry["kind"] != "cube":
        raise HTTPException(
            status_code=status.HTTP_422_UNPROCESSABLE_ENTITY,
            detail="This endpoint requires a spectral cube dataset.",
        )
    return entry["path"]


# ── FITS metadata (runs in main process – no heavy I/O) ───────────────────────


def _fits_metadata(path: Path) -> tuple[str, dict[str, Any]]:
    """
    Open the FITS header only (no data materialisation) to detect kind and
    extract geometry metadata.  Runs synchronously in the main process because
    it is fast (header-only read, O(KB)).
    """
    with fits.open(str(path), memmap=True, mode="readonly") as hdul:
        header = hdul[0].header.copy()
        raw_shape = hdul[0].shape  # shape without reading data

    naxis = int(header.get("NAXIS", 0))
    axis_sizes = [int(header.get(f"NAXIS{i}", 1)) for i in range(1, naxis + 1)]
    active_axes_idx = [i for i, s in enumerate(axis_sizes, start=1) if s > 1]
    degenerate_idx  = [i for i, s in enumerate(axis_sizes, start=1) if s == 1]
    n_active = len(active_axes_idx)

    deg_entries: list[str] = []
    for ai in degenerate_idx:
        name = str(header.get(f"CTYPE{ai}", f"AXIS{ai}")).strip()
        val  = float(header.get(f"CRVAL{ai}", 0.0))
        unit = str(header.get(f"CUNIT{ai}", "")).strip()
        label = f"{name}={val:g}"
        if unit and name.upper() != "STOKES":
            label += f" {unit}"
        label += " (1)"
        deg_entries.append(label)

    def _h(key: str, default=0.0) -> float:
        return float(header.get(key, default))

    spacing = [_h("CDELT1", 1.0), _h("CDELT2", 1.0), _h("CDELT3", 1.0)]
    origin  = [
        _h("CRVAL1") - spacing[0] * (_h("CRPIX1", 1.0) - 1.0),
        _h("CRVAL2") - spacing[1] * (_h("CRPIX2", 1.0) - 1.0),
        _h("CRVAL3") - spacing[2] * (_h("CRPIX3", 1.0) - 1.0),
    ]

    # Derive spatial dimensions from the squeezed active shape.
    squeezed = [s for s in axis_sizes if s > 1]
    if n_active == 2:
        width, height, depth = squeezed[0], squeezed[1], 1
        kind = "image"
    elif n_active == 3:
        width, height, depth = squeezed[0], squeezed[1], squeezed[2]
        kind = "cube"
    else:
        width = squeezed[0] if squeezed else 1
        height = squeezed[1] if len(squeezed) > 1 else 1
        depth = 1
        kind = "image"

    geometry: dict[str, Any] = {
        "active_axes": n_active,
        "degenerate_axes_summary": f"Collapsed axes: {', '.join(deg_entries)}" if deg_entries else "",
        "width": width,
        "height": height,
        "depth": depth,
        "spacing": spacing,
        "origin": origin,
        "ctype": [str(header.get(f"CTYPE{i}", "")) for i in range(1, 4)],
        "cunit": [str(header.get(f"CUNIT{i}", "")) for i in range(1, 4)],
        "crval": [_h(f"CRVAL{i}") for i in range(1, 4)],
        "crpix": [_h(f"CRPIX{i}", 1.0) for i in range(1, 4)],
        "cdelt": spacing,
    }
    return kind, geometry


# ── Routes ────────────────────────────────────────────────────────────────────
# All routes require authentication (global dependency).
# Heavy routes additionally use `_run()` to avoid blocking the event-loop.

_auth = Depends(verify_token)


@app.get("/v1/health", tags=["meta"])
async def health(_: None = _auth) -> dict:
    return {
        "ok": True,
        "workers": _WORKERS,
        "active_sessions": REGISTRY.stats()["active_sessions"],
        "timestamp": datetime.now(tz=timezone.utc).isoformat(),
    }


@app.get("/v1/sessions", tags=["meta"])
async def session_stats(_: None = _auth) -> dict:
    """Expose session registry statistics (admin endpoint)."""
    return REGISTRY.stats()


@app.get("/v1/files/list", response_model=FilesListResponse, tags=["files"])
async def list_files(
    path: str = Query(""),
    _: None = _auth,
) -> FilesListResponse:
    try:
        directory = _normalize_path(path)
    except Exception:
        return FilesListResponse(valid=False, error="Invalid path.", current_path="", entries=[])

    if not directory.exists() or not directory.is_dir():
        return FilesListResponse(valid=False, error="Directory not found.", current_path="", entries=[])

    try:
        children = sorted(directory.iterdir(), key=lambda p: (not p.is_dir(), p.name.lower()))
    except OSError as exc:
        return FilesListResponse(valid=False, error=str(exc), current_path=str(directory), entries=[])

    entries: list[FileEntry] = []
    for child in children:
        try:
            st = child.stat()
            size = int(st.st_size)
            mtime = datetime.fromtimestamp(st.st_mtime, tz=timezone.utc).isoformat()
        except OSError:
            size, mtime = 0, ""
        ftype = "directory" if child.is_dir() else "file"
        entries.append(FileEntry(
            name=child.name,
            path=str(child),
            type=ftype,
            size=size,
            modified_time=mtime,
            is_fits=_is_fits(child) if child.is_file() else False,
        ))

    return FilesListResponse(valid=True, error="", current_path=str(directory), entries=entries)


@app.post("/v1/files/header", response_model=FileHeaderResponse, tags=["files"])
async def file_header(request: FileHeaderRequest, _: None = _auth) -> FileHeaderResponse:
    try:
        path = _normalize_path(request.path)
        if not _is_fits(path):
            raise ValueError("FITS file not found.")
        with fits.open(str(path), memmap=True) as hdul:
            cards = [str(card) for card in hdul[0].header.cards]
    except Exception as exc:
        return FileHeaderResponse(valid=False, error=str(exc), cards=[])
    return FileHeaderResponse(valid=True, error="", cards=cards)


@app.post("/v1/datasets/open", response_model=OpenDatasetResponse, tags=["datasets"])
async def open_dataset(
    request: OpenDatasetRequest,
    _: None = _auth,
    session: Session = Depends(get_session),
) -> OpenDatasetResponse:
    path = _normalize_path(request.path)
    if not _is_fits(path):
        return OpenDatasetResponse(valid=False, error="FITS file not found.")
    try:
        kind, geometry = _fits_metadata(path)
    except Exception as exc:
        return OpenDatasetResponse(valid=False, error=str(exc))

    dataset_id = f"ds_{uuid.uuid4().hex[:12]}"
    session.datasets[dataset_id] = {"path": str(path), "kind": kind, **geometry}
    logger.info(
        "[open] session=%s dataset_id=%s kind=%s path=%s",
        session.session_id, dataset_id, kind, path,
    )
    return OpenDatasetResponse(
        valid=True,
        error="",
        dataset_id=dataset_id,
        session_id=session.session_id,
        kind=kind,
        active_axes=int(geometry.get("active_axes", 0)),
        width=geometry["width"],
        height=geometry["height"],
        depth=geometry["depth"],
        degenerate_axes_summary=str(geometry.get("degenerate_axes_summary", "")),
        spacing=geometry["spacing"],
        origin=geometry["origin"],
        ctype=geometry["ctype"],
        cunit=geometry["cunit"],
        crval=geometry["crval"],
        crpix=geometry["crpix"],
        cdelt=geometry["cdelt"],
    )


@app.post("/v1/products/moment", response_model=MomentProductResponse, tags=["products"])
async def moment_product(
    request: MomentProductRequest,
    _: None = _auth,
    session: Session = Depends(get_session),
) -> MomentProductResponse:
    from .compute import worker_moment
    try:
        path = _require_cube_path(session, request.dataset_id)
        result = await _run(
            worker_moment,
            path,
            request.moment_order,
            request.channel_start,
            request.channel_end,
            request.mask_enabled,
            request.threshold_value,
        )
    except HTTPException:
        raise
    except Exception as exc:
        return MomentProductResponse(valid=False, error=str(exc))
    return MomentProductResponse(valid=True, error="", **result)


@app.post("/v1/products/isosurface", response_model=IsosurfaceProductResponse, tags=["products"])
async def isosurface_product(
    request: IsosurfaceProductRequest,
    _: None = _auth,
    session: Session = Depends(get_session),
) -> IsosurfaceProductResponse:
    if not _VTK_AVAILABLE:
        return IsosurfaceProductResponse(
            valid=False,
            error="VTK Python bindings are not installed on this server. Isosurface is unavailable.",
        )
    from .compute import worker_isosurface
    try:
        entry = _require_dataset(session, request.dataset_id)
        if entry["kind"] != "cube":
            raise HTTPException(422, "Isosurface requires a cube dataset.")
        result = await _run(
            worker_isosurface,
            entry["path"],
            int(entry["width"]),
            int(entry["height"]),
            int(entry["depth"]),
            float(request.threshold),
        )
    except HTTPException:
        raise
    except Exception as exc:
        logger.warning("[isosurface] failed dataset_id=%s error=%s", request.dataset_id, exc)
        return IsosurfaceProductResponse(valid=False, error=str(exc))
    return IsosurfaceProductResponse(valid=True, error="", **result)


@app.post("/v1/cube/preview", response_model=CubePreviewResponse, tags=["cube"])
async def cube_preview(
    request: CubePreviewRequest,
    _: None = _auth,
    session: Session = Depends(get_session),
) -> CubePreviewResponse:
    from .compute import worker_cube_preview
    try:
        path = _require_cube_path(session, request.dataset_id)
        result = await _run(worker_cube_preview, path, request.downsample)
    except HTTPException:
        raise
    except Exception as exc:
        return CubePreviewResponse(valid=False, error=str(exc))
    return CubePreviewResponse(valid=True, error="", **result)


@app.post("/v1/cube/slice", response_model=CubeSliceResponse, tags=["cube"])
async def cube_slice(
    request: CubeSliceRequest,
    _: None = _auth,
    session: Session = Depends(get_session),
) -> CubeSliceResponse:
    from .compute import worker_cube_slice
    try:
        path = _require_cube_path(session, request.dataset_id)
        if request.axis.lower() != "z":
            raise HTTPException(422, "Only axis='z' is currently supported.")
        result = await _run(worker_cube_slice, path, request.index)
    except HTTPException:
        raise
    except Exception as exc:
        return CubeSliceResponse(valid=False, error=str(exc))
    return CubeSliceResponse(valid=True, error="", **result)


@app.post("/v1/cube/subvolume", response_model=CubeSubvolumeResponse, tags=["cube"])
async def cube_subvolume(
    request: CubeSubvolumeRequest,
    _: None = _auth,
    session: Session = Depends(get_session),
) -> CubeSubvolumeResponse:
    from .compute import worker_cube_subvolume
    try:
        path = _require_cube_path(session, request.dataset_id)
        result = await _run(
            worker_cube_subvolume,
            path,
            request.x0, request.x1,
            request.y0, request.y1,
            request.z0, request.z1,
        )
    except HTTPException:
        raise
    except Exception as exc:
        return CubeSubvolumeResponse(valid=False, error=str(exc))
    return CubeSubvolumeResponse(valid=True, error="", **result)


@app.post("/v1/cube/pv", response_model=CubePvResponse, tags=["cube"])
async def cube_pv(
    request: CubePvRequest,
    _: None = _auth,
    session: Session = Depends(get_session),
) -> CubePvResponse:
    from .compute import worker_pv
    try:
        path = _require_cube_path(session, request.dataset_id)
        result = await _run(
            worker_pv,
            path,
            request.vertices,
            max(1, int(request.width_pixels)),
        )
    except HTTPException:
        raise
    except Exception as exc:
        logger.warning("[pv] failed dataset_id=%s error=%s", request.dataset_id, exc)
        return CubePvResponse(valid=False, error=str(exc))
    return CubePvResponse(valid=True, error="", **result)


@app.post("/v1/image/full", response_model=ImageFullResponse, tags=["image"])
async def image_full(
    request: ImageFullRequest,
    _: None = _auth,
    session: Session = Depends(get_session),
) -> ImageFullResponse:
    from .compute import worker_image_full
    try:
        entry = _require_dataset(session, request.dataset_id)
        if entry["kind"] != "image":
            return ImageFullResponse(valid=False, error="Image endpoint requires an image dataset.")
        result = await _run(worker_image_full, entry["path"])
    except HTTPException:
        raise
    except Exception as exc:
        return ImageFullResponse(valid=False, error=str(exc))
    return ImageFullResponse(valid=True, error="", **result)


@app.post("/v1/image/preview", response_model=ImageFullResponse, tags=["image"])
async def image_preview(
    request: ImagePreviewRequest,
    _: None = _auth,
    session: Session = Depends(get_session),
) -> ImageFullResponse:
    from .compute import worker_image_preview
    try:
        entry = _require_dataset(session, request.dataset_id)
        if entry["kind"] != "image":
            raise HTTPException(422, "Image preview endpoint requires an image dataset.")
        result = await _run(worker_image_preview, entry["path"], request.max_longest_side)
    except HTTPException:
        raise
    except Exception as exc:
        return ImageFullResponse(valid=False, error=str(exc))
    return ImageFullResponse(valid=True, error="", **result)
