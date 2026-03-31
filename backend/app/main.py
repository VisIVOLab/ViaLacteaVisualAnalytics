from __future__ import annotations

import base64
from datetime import datetime, timezone
import logging
import uuid
from pathlib import Path
from typing import Any

import numpy as np
from astropy.io import fits
from fastapi import FastAPI, Query
from pydantic import BaseModel

app = FastAPI(title="VisIVO Backend MVP")
logger = logging.getLogger("visivo.backend")

_DATASETS: dict[str, dict[str, Any]] = {}
_FITS_SUFFIXES = {".fits", ".fit", ".fts"}


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
    kind: str = ""
    width: int = 0
    height: int = 0
    depth: int = 0
    spacing: list[float] = [1.0, 1.0, 1.0]
    origin: list[float] = [0.0, 0.0, 0.0]


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


class ImageFullRequest(BaseModel):
    dataset_id: str


class ImageFullResponse(BaseModel):
    valid: bool
    error: str
    width: int = 0
    height: int = 0
    scalar_type: str = ""
    data_base64: str = ""


class IsosurfaceProductRequest(BaseModel):
    dataset_id: str
    threshold: float


class IsosurfaceProductResponse(BaseModel):
    valid: bool
    error: str
    points_base64: str = ""
    polys_base64: str = ""
    num_points: int = 0
    num_polys: int = 0


def _normalize_path(raw_path: str) -> Path:
    if not raw_path:
        return Path.home()
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


def _dataset_geometry(array: np.ndarray, header: fits.Header) -> dict[str, Any]:
    if array.ndim == 2:
        width = int(array.shape[1])
        height = int(array.shape[0])
        depth = 1
    elif array.ndim == 3:
        width = int(array.shape[2])
        height = int(array.shape[1])
        depth = int(array.shape[0])
    else:
        raise ValueError("Unsupported FITS dimensionality.")

    spacing = [
        float(header.get("CDELT1", 1.0)),
        float(header.get("CDELT2", 1.0)),
        float(header.get("CDELT3", 1.0)),
    ]
    origin = [
        float(header.get("CRVAL1", 0.0)) - spacing[0] * (float(header.get("CRPIX1", 1.0)) - 1.0),
        float(header.get("CRVAL2", 0.0)) - spacing[1] * (float(header.get("CRPIX2", 1.0)) - 1.0),
        float(header.get("CRVAL3", 0.0)) - spacing[2] * (float(header.get("CRPIX3", 1.0)) - 1.0),
    ]

    return {
        "width": width,
        "height": height,
        "depth": depth,
        "spacing": spacing,
        "origin": origin,
    }


def _detect_kind(path: Path) -> tuple[str, dict[str, Any]]:
    array, header = _load_dataset_array(path)
    geometry = _dataset_geometry(array, header)
    if array.ndim == 2:
        return "image", geometry
    if array.ndim == 3:
        return "cube", geometry
    raise ValueError("Unsupported FITS dimensionality.")


def _dataset_entry(dataset_id: str) -> dict[str, Any]:
    entry = _DATASETS.get(dataset_id)
    if not entry:
        raise ValueError("Unknown dataset_id.")
    return entry


def _require_cube(dataset_id: str) -> Path:
    entry = _dataset_entry(dataset_id)
    if entry["kind"] != "cube":
        raise ValueError("Cube endpoint requires a cube dataset.")
    return Path(entry["path"])


def _finite_range(array: np.ndarray) -> tuple[float, float]:
    finite = np.isfinite(array)
    if not finite.any():
        return 0.0, 0.0
    return float(np.nanmin(array)), float(np.nanmax(array))


def _encode_array(array: np.ndarray) -> str:
    return base64.b64encode(np.ascontiguousarray(array, dtype=np.float32).tobytes()).decode("ascii")


def _encode_int_array(array: np.ndarray) -> str:
    return base64.b64encode(np.ascontiguousarray(array, dtype=np.int32).tobytes()).decode("ascii")


def _compute_isosurface_payload(cube: np.ndarray, entry: dict[str, Any], threshold: float) -> dict[str, Any]:
    try:
        import vtk
        from vtk.util.numpy_support import vtk_to_numpy
    except Exception as exc:
        raise RuntimeError(f"VTK Python bindings are required for isosurface compute: {exc}") from exc

    image_import = vtk.vtkImageImport()
    cube_bytes = np.ascontiguousarray(cube, dtype=np.float32).tobytes()
    image_import.CopyImportVoidPointer(cube_bytes, len(cube_bytes))
    image_import.SetDataScalarTypeToFloat()
    image_import.SetNumberOfScalarComponents(1)
    image_import.SetDataExtent(0, int(entry["width"]) - 1, 0, int(entry["height"]) - 1, 0,
                               int(entry["depth"]) - 1)
    image_import.SetWholeExtent(0, int(entry["width"]) - 1, 0, int(entry["height"]) - 1, 0,
                                int(entry["depth"]) - 1)
    spacing = [1.0, 1.0, 1.0]
    origin = [0.0, 0.0, 0.0]
    image_import.SetDataSpacing(1.0, 1.0, 1.0)
    image_import.SetDataOrigin(0.0, 0.0, 0.0)
    image_import.Update()
    logger.info(
        "[remote-iso] image dims=%s spacing=%s origin=%s threshold=%s",
        (int(entry["width"]), int(entry["height"]), int(entry["depth"])),
        tuple(float(v) for v in spacing),
        tuple(float(v) for v in origin),
        float(threshold),
    )

    contour = vtk.vtkFlyingEdges3D()
    contour.SetInputConnection(image_import.GetOutputPort())
    contour.SetValue(0, float(threshold))
    contour.ComputeNormalsOff()
    contour.ComputeGradientsOff()
    contour.Update()

    mesh = contour.GetOutput()
    num_points = int(mesh.GetNumberOfPoints())
    num_polys = int(mesh.GetNumberOfPolys())
    bounds = mesh.GetBounds()
    logger.info("[remote-iso] mesh points=%s polys=%s", num_points, num_polys)
    logger.info(
        "[remote-iso] mesh bounds=%s",
        tuple(float(v) for v in bounds),
    )
    if num_points == 0 or num_polys == 0:
        raise ValueError(f"Empty isosurface mesh for threshold {threshold}")

    points_data = vtk_to_numpy(mesh.GetPoints().GetData()).astype(np.float32, copy=False).reshape(-1)
    polys_data = vtk_to_numpy(mesh.GetPolys().GetData()).astype(np.int32, copy=False)
    return {
        "points_base64": _encode_array(points_data),
        "polys_base64": _encode_int_array(polys_data),
        "num_points": num_points,
        "num_polys": num_polys,
    }


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
def list_files(path: str = Query("")) -> FilesListResponse:
    try:
        directory = _normalize_path(path)
    except Exception:
        return FilesListResponse(valid=False, error="Invalid path.", current_path="", entries=[])

    if not directory.exists() or not directory.is_dir():
        return FilesListResponse(valid=False, error="Directory not found.", current_path="", entries=[])

    entries: list[FileEntry] = []
    try:
        children = sorted(directory.iterdir(), key=lambda item: (not item.is_dir(), item.name.lower()))
    except OSError as exc:
        return FilesListResponse(valid=False, error=str(exc), current_path=str(directory), entries=[])

    for child in children:
        try:
            stat = child.stat()
            size = int(stat.st_size)
            modified_time = datetime.fromtimestamp(stat.st_mtime, tz=timezone.utc).isoformat()
        except OSError:
            size = 0
            modified_time = ""

        if child.is_dir():
            entries.append(
                FileEntry(
                    name=child.name,
                    path=str(child),
                    type="directory",
                    size=size,
                    modified_time=modified_time,
                    is_fits=False,
                )
            )
        elif child.is_file():
            entries.append(
                FileEntry(
                    name=child.name,
                    path=str(child),
                    type="file",
                    size=size,
                    modified_time=modified_time,
                    is_fits=_is_fits_file(child),
                )
            )

    return FilesListResponse(valid=True, error="", current_path=str(directory), entries=entries)


@app.post("/files/header", response_model=FileHeaderResponse)
def file_header(request: FileHeaderRequest) -> FileHeaderResponse:
    try:
        path = _normalize_path(request.path)
        if not _is_fits_file(path):
            raise ValueError("FITS file not found.")

        with fits.open(path, memmap=True) as hdul:
            header = hdul[0].header
            cards = [str(card) for card in header.cards]
    except Exception as exc:
        return FileHeaderResponse(valid=False, error=str(exc), cards=[])

    return FileHeaderResponse(valid=True, error="", cards=cards)


@app.post("/datasets/open", response_model=OpenDatasetResponse)
def open_dataset(request: OpenDatasetRequest) -> OpenDatasetResponse:
    path = _normalize_path(request.path)
    if not _is_fits_file(path):
        return OpenDatasetResponse(valid=False, error="FITS file not found.")

    try:
        kind, geometry = _detect_kind(path)
    except Exception as exc:
        return OpenDatasetResponse(valid=False, error=str(exc))

    dataset_id = f"ds_{uuid.uuid4().hex[:12]}"
    _DATASETS[dataset_id] = {"path": str(path), "kind": kind, **geometry}
    return OpenDatasetResponse(
        valid=True,
        error="",
        dataset_id=dataset_id,
        kind=kind,
        width=geometry["width"],
        height=geometry["height"],
        depth=geometry["depth"],
        spacing=geometry["spacing"],
        origin=geometry["origin"],
    )


@app.post("/products/moment", response_model=MomentProductResponse)
def moment_product(request: MomentProductRequest) -> MomentProductResponse:
    try:
        cube_path = _require_cube(request.dataset_id)
        cube, header = _load_dataset_array(cube_path)
        image = np.asarray(_moment_map(cube, header, request.moment_order), dtype=np.float32)
    except Exception as exc:
        return MomentProductResponse(valid=False, error=str(exc))

    range_min, range_max = _finite_range(image)
    return MomentProductResponse(
        valid=True,
        error="",
        width=int(image.shape[1]),
        height=int(image.shape[0]),
        scalar_type="float32",
        range_min=range_min,
        range_max=range_max,
        data_base64=_encode_array(image),
    )


@app.post("/products/isosurface", response_model=IsosurfaceProductResponse)
def isosurface_product(request: IsosurfaceProductRequest) -> IsosurfaceProductResponse:
    try:
        entry = _dataset_entry(request.dataset_id)
        cube_path = _require_cube(request.dataset_id)
        logger.info(
            "[remote-iso] request dataset_id=%s threshold=%s path=%s",
            request.dataset_id,
            float(request.threshold),
            cube_path,
        )
        cube, _ = _load_dataset_array(cube_path)
        payload = _compute_isosurface_payload(cube, entry, request.threshold)
    except Exception as exc:
        logger.warning("[remote-iso] failed dataset_id=%s error=%s", request.dataset_id, exc)
        return IsosurfaceProductResponse(valid=False, error=str(exc))

    return IsosurfaceProductResponse(valid=True, error="", **payload)


@app.post("/cube/preview", response_model=CubePreviewResponse)
def cube_preview(request: CubePreviewRequest) -> CubePreviewResponse:
    try:
        cube_path = _require_cube(request.dataset_id)
        cube, _ = _load_dataset_array(cube_path)
        stride = max(1, int(request.downsample))
        preview = np.asarray(cube[::stride, ::stride, ::stride], dtype=np.float32)
    except Exception as exc:
        return CubePreviewResponse(valid=False, error=str(exc))

    range_min, range_max = _finite_range(preview)
    return CubePreviewResponse(
        valid=True,
        error="",
        width=int(preview.shape[2]),
        height=int(preview.shape[1]),
        depth=int(preview.shape[0]),
        scalar_type="float32",
        range_min=range_min,
        range_max=range_max,
        data_base64=_encode_array(preview),
    )


@app.post("/cube/slice", response_model=CubeSliceResponse)
def cube_slice(request: CubeSliceRequest) -> CubeSliceResponse:
    try:
        cube_path = _require_cube(request.dataset_id)
        cube, _ = _load_dataset_array(cube_path)
        axis = request.axis.lower()
        if axis != "z":
            raise ValueError("Only axis='z' is supported in this step.")
        if request.index < 0 or request.index >= cube.shape[0]:
            raise ValueError("Slice index out of range.")
        image = np.asarray(cube[request.index, :, :], dtype=np.float32)
    except Exception as exc:
        return CubeSliceResponse(valid=False, error=str(exc))

    range_min, range_max = _finite_range(image)
    return CubeSliceResponse(
        valid=True,
        error="",
        width=int(image.shape[1]),
        height=int(image.shape[0]),
        scalar_type="float32",
        range_min=range_min,
        range_max=range_max,
        data_base64=_encode_array(image),
    )


@app.post("/cube/subvolume", response_model=CubeSubvolumeResponse)
def cube_subvolume(request: CubeSubvolumeRequest) -> CubeSubvolumeResponse:
    try:
        cube_path = _require_cube(request.dataset_id)
        cube, _ = _load_dataset_array(cube_path)
        if cube.ndim != 3:
            raise ValueError("Subvolume endpoint requires a cube dataset.")

        depth, height, width = cube.shape
        x0 = max(0, min(request.x0, width - 1))
        x1 = max(0, min(request.x1, width - 1))
        y0 = max(0, min(request.y0, height - 1))
        y1 = max(0, min(request.y1, height - 1))
        z0 = max(0, min(request.z0, depth - 1))
        z1 = max(0, min(request.z1, depth - 1))
        if x0 > x1 or y0 > y1 or z0 > z1:
            raise ValueError("Invalid subvolume ROI.")

        subvolume = np.ascontiguousarray(cube[z0 : z1 + 1, y0 : y1 + 1, x0 : x1 + 1], dtype=np.float32)
    except Exception as exc:
        return CubeSubvolumeResponse(valid=False, error=str(exc))

    return CubeSubvolumeResponse(
        valid=True,
        error="",
        width=int(subvolume.shape[2]),
        height=int(subvolume.shape[1]),
        depth=int(subvolume.shape[0]),
        scalar_type="float32",
        data_base64=_encode_array(subvolume),
    )


@app.post("/image/full", response_model=ImageFullResponse)
def image_full(request: ImageFullRequest) -> ImageFullResponse:
    try:
        entry = _dataset_entry(request.dataset_id)
        if entry["kind"] != "image":
            raise ValueError("Image endpoint requires an image dataset.")
        image_path = Path(entry["path"])
        image, _ = _load_dataset_array(image_path)
        if image.ndim != 2:
            raise ValueError("Remote image endpoint requires 2D FITS data.")
        image = np.ascontiguousarray(image, dtype=np.float32)
    except Exception as exc:
        return ImageFullResponse(valid=False, error=str(exc))

    return ImageFullResponse(
        valid=True,
        error="",
        width=int(image.shape[1]),
        height=int(image.shape[0]),
        scalar_type="float32",
        data_base64=_encode_array(image),
    )
