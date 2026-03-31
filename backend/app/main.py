from __future__ import annotations

import base64
import uuid
from pathlib import Path

import numpy as np
from astropy.io import fits
from fastapi import FastAPI, Query
from pydantic import BaseModel

app = FastAPI(title="VisIVO Backend MVP")

_DATASETS: dict[str, dict[str, str]] = {}
_FITS_SUFFIXES = {".fits", ".fit", ".fts"}


class FileEntry(BaseModel):
    name: str
    path: str
    type: str


class FilesListResponse(BaseModel):
    valid: bool
    error: str
    entries: list[FileEntry]


class OpenDatasetRequest(BaseModel):
    path: str


class OpenDatasetResponse(BaseModel):
    valid: bool
    error: str
    dataset_id: str = ""
    kind: str = ""


class MomentProductRequest(BaseModel):
    dataset_id: str
    moment_order: int


class MomentProductResponse(BaseModel):
    valid: bool
    error: str
    width: int = 0
    height: int = 0
    scalar_type: str = ""
    range_min: float = 0.0
    range_max: float = 0.0
    data_base64: str = ""


def _normalize_path(raw_path: str) -> Path:
    if not raw_path:
        return Path("/")
    return Path(raw_path).expanduser().resolve()


def _is_fits_file(path: Path) -> bool:
    return path.is_file() and path.suffix.lower() in _FITS_SUFFIXES


def _load_dataset_array(path: Path) -> tuple[np.ndarray, fits.Header]:
    with fits.open(path, memmap=True) as hdul:
        data = hdul[0].data
        header = hdul[0].header.copy()

    if data is None:
        raise ValueError("FITS file contains no primary image data.")

    array = np.asarray(data, dtype=np.float32)
    array = np.squeeze(array)
    while array.ndim > 3:
        array = array[0]

    return array, header


def _detect_kind(path: Path) -> str:
    array, _ = _load_dataset_array(path)
    if array.ndim == 2:
        return "image"
    if array.ndim == 3:
        return "cube"
    raise ValueError("Unsupported FITS dimensionality.")


def _moment_map(cube: np.ndarray, header: fits.Header, order: int) -> np.ndarray:
    if cube.ndim != 3:
        raise ValueError("Moment products require a cube dataset.")

    spectral_delta = abs(float(header.get("CDELT3", 1.0)))
    init_spectral = float(header.get("CRVAL3", 0.0)) - spectral_delta * (
        float(header.get("CRPIX3", 1.0)) - 1.0
    )
    spectral_values = init_spectral + spectral_delta * np.arange(cube.shape[0], dtype=np.float32)

    finite = np.isfinite(cube)
    safe_cube = np.where(finite, cube, np.nan)

    if order == 0:
        return np.nansum(safe_cube * spectral_delta, axis=0, dtype=np.float32)

    if order == 1:
        moment0 = _moment_map(cube, header, 0)
        weighted = safe_cube * spectral_values[:, None, None] * spectral_delta
        numerator = np.nansum(weighted, axis=0, dtype=np.float32)
        out = np.full(moment0.shape, np.nan, dtype=np.float32)
        valid = np.isfinite(moment0) & (moment0 != 0.0)
        out[valid] = numerator[valid] / moment0[valid]
        return out

    if order == 2:
        moment0 = _moment_map(cube, header, 0)
        moment1 = _moment_map(cube, header, 1)
        diff = spectral_values[:, None, None] - moment1[None, :, :]
        numerator = np.nansum(safe_cube * diff * diff * spectral_delta, axis=0, dtype=np.float32)
        out = np.full(moment0.shape, np.nan, dtype=np.float32)
        valid = np.isfinite(moment0) & (moment0 != 0.0)
        out[valid] = numerator[valid] / moment0[valid]
        return out

    if order == 8:
        return np.nanmax(safe_cube, axis=0)

    if order == 10:
        return np.nanmin(safe_cube, axis=0)

    raise ValueError("Unsupported moment order.")


@app.get("/health")
def health() -> dict[str, bool]:
    return {"ok": True}


@app.get("/files/list", response_model=FilesListResponse)
def list_files(path: str = Query("/")) -> FilesListResponse:
    try:
        directory = _normalize_path(path)
    except Exception:
        return FilesListResponse(valid=False, error="Invalid path.", entries=[])

    if not directory.exists() or not directory.is_dir():
        return FilesListResponse(valid=False, error="Directory not found.", entries=[])

    entries: list[FileEntry] = []
    try:
        children = sorted(directory.iterdir(), key=lambda item: (not item.is_dir(), item.name.lower()))
    except OSError as exc:
        return FilesListResponse(valid=False, error=str(exc), entries=[])

    for child in children:
        if child.is_dir():
            entries.append(FileEntry(name=child.name, path=str(child), type="directory"))
        elif _is_fits_file(child):
            entries.append(FileEntry(name=child.name, path=str(child), type="file"))

    return FilesListResponse(valid=True, error="", entries=entries)


@app.post("/datasets/open", response_model=OpenDatasetResponse)
def open_dataset(request: OpenDatasetRequest) -> OpenDatasetResponse:
    path = _normalize_path(request.path)
    if not _is_fits_file(path):
        return OpenDatasetResponse(valid=False, error="FITS file not found.")

    try:
        kind = _detect_kind(path)
    except Exception as exc:
        return OpenDatasetResponse(valid=False, error=str(exc))

    dataset_id = f"ds_{uuid.uuid4().hex[:12]}"
    _DATASETS[dataset_id] = {"path": str(path), "kind": kind}
    return OpenDatasetResponse(valid=True, error="", dataset_id=dataset_id, kind=kind)


@app.post("/products/moment", response_model=MomentProductResponse)
def moment_product(request: MomentProductRequest) -> MomentProductResponse:
    entry = _DATASETS.get(request.dataset_id)
    if not entry:
        return MomentProductResponse(valid=False, error="Unknown dataset_id.")

    if entry["kind"] != "cube":
        return MomentProductResponse(valid=False, error="Moment products require a cube dataset.")

    try:
        cube, header = _load_dataset_array(Path(entry["path"]))
        image = np.asarray(_moment_map(cube, header, request.moment_order), dtype=np.float32)
    except Exception as exc:
        return MomentProductResponse(valid=False, error=str(exc))

    finite = np.isfinite(image)
    if finite.any():
        range_min = float(np.nanmin(image))
        range_max = float(np.nanmax(image))
    else:
        range_min = 0.0
        range_max = 0.0

    payload = base64.b64encode(np.ascontiguousarray(image).tobytes()).decode("ascii")
    return MomentProductResponse(
        valid=True,
        error="",
        width=int(image.shape[1]),
        height=int(image.shape[0]),
        scalar_type="float32",
        range_min=range_min,
        range_max=range_max,
        data_base64=payload,
    )
