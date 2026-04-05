"""
compute.py – R2 + R4: CPU-bound worker functions for the process pool.

All functions in this module are designed to run inside a
concurrent.futures.ProcessPoolExecutor worker process.  They MUST:

  • be importable at the top level (picklable by multiprocessing)
  • open FITS files themselves with memmap=True so only the needed
    pixels are loaded from disk (R4 – lazy I/O, no full-cube RAM load)
  • accept / return only pickle-serialisable types (str, dict, list,
    numpy dtypes that are base64-encoded before returning)

Functions in this module intentionally do NOT import FastAPI or Pydantic
so they can be reused in batch / headless pipelines without the web stack.
"""

from __future__ import annotations

import base64
import logging
import zlib
from pathlib import Path
from typing import Any

import numpy as np
from astropy.io import fits
from astropy.wcs import WCS

logger = logging.getLogger("visivo.compute")

# ── Encoding helpers ──────────────────────────────────────────────────────────


def _qt_zlib_compress(raw: bytes) -> bytes:
    """Prepend the uncompressed length as a 4-byte big-endian uint (Qt convention)."""
    return len(raw).to_bytes(4, byteorder="big", signed=False) + zlib.compress(raw)


def _b64f32(array: np.ndarray) -> str:
    return base64.b64encode(np.ascontiguousarray(array, dtype=np.float32).tobytes()).decode("ascii")


def _b64i32(array: np.ndarray) -> str:
    return base64.b64encode(np.ascontiguousarray(array, dtype=np.int32).tobytes()).decode("ascii")


def _b64f32_compressed(array: np.ndarray) -> tuple[str, str]:
    raw = np.ascontiguousarray(array, dtype=np.float32).tobytes()
    return "qt-zlib", base64.b64encode(_qt_zlib_compress(raw)).decode("ascii")


def _b64i32_compressed(array: np.ndarray) -> tuple[str, str]:
    raw = np.ascontiguousarray(array, dtype=np.int32).tobytes()
    return "qt-zlib", base64.b64encode(_qt_zlib_compress(raw)).decode("ascii")


def _finite_range(array: np.ndarray) -> tuple[float, float]:
    finite = np.isfinite(array)
    if not finite.any():
        return 0.0, 0.0
    return float(np.nanmin(array)), float(np.nanmax(array))


# ── FITS I/O helpers ──────────────────────────────────────────────────────────


def _open_fits_memmap(path: str):
    """
    Open a FITS file in read-only memory-mapped mode.

    Returns the (HDUList, primary HDU data array) without materialising it.
    The caller is responsible for closing the HDUList.  Use as a context manager
    or call hdul.close() explicitly.

    The returned data object is a numpy memmap array – slicing it reads only
    the requested region from disk (R4 lazy I/O).
    """
    hdul = fits.open(path, memmap=True, mode="readonly")
    data = hdul[0].data
    if data is None:
        hdul.close()
        raise ValueError("FITS file contains no primary image data.")
    return hdul, data


def _squeeze_to_3d(data: np.ndarray) -> np.ndarray:
    """Remove length-1 axes until ndim <= 3, matching the original logic."""
    arr = np.squeeze(data)
    while arr.ndim > 3:
        arr = arr[0]
    return arr


def _active_fits_axes(header: fits.Header) -> list[int]:
    naxis = int(header.get("NAXIS", 0))
    return [axis for axis in range(1, naxis + 1) if int(header.get(f"NAXIS{axis}", 1)) > 1]


def _cube_numpy_spectral_fits_axis(header: fits.Header) -> int:
    """
    Map the cube spectral/channel index used by the current client/backend cube path
    to the corresponding FITS pixel axis.

    The current cube semantics treat numpy axis 0 as the spectral/depth axis.
    For FITS image cubes this matches the highest-numbered active FITS axis after
    dropping degenerate axes, which is also how the current 3D data path is opened.
    """
    active_axes = _active_fits_axes(header)
    if len(active_axes) < 3:
        raise ValueError("Scientific cube helpers require at least three active FITS axes.")
    return active_axes[-1]


def _build_spectral_coordinate_metadata(
    header: fits.Header, channel_start: int, channel_end: int
) -> dict[str, Any]:
    """
    Compute physical spectral coordinates for the selected channel range using astropy.wcs.WCS.

    This avoids using raw CDELT3/CRVAL3/CRPIX3 directly, which is not scientifically safe
    for general FITS spectral axes.
    """
    spectral_fits_axis = _cube_numpy_spectral_fits_axis(header)
    wcs = WCS(header)
    pixel_naxis = int(wcs.pixel_n_dim)
    world_naxis = int(wcs.world_n_dim)
    if pixel_naxis <= 0 or world_naxis <= 0:
        raise ValueError("FITS WCS is not available for spectral-coordinate computation.")

    axis_types = [ptype or "" for ptype in (wcs.world_axis_physical_types or [])]
    correlation = np.asarray(wcs.axis_correlation_matrix, dtype=bool)
    spectral_pixel_index = spectral_fits_axis - 1

    spectral_world_index = None
    preferred_world_indices = [
        idx
        for idx, ptype in enumerate(axis_types)
        if ptype.startswith("em.") or "spect" in ptype or "freq" in ptype or "velo" in ptype
    ]
    for world_index in preferred_world_indices:
        if (
            world_index < correlation.shape[0]
            and spectral_pixel_index < correlation.shape[1]
            and correlation[world_index, spectral_pixel_index]
        ):
            spectral_world_index = world_index
            break
    if spectral_world_index is None:
        for world_index in range(min(correlation.shape[0], world_naxis)):
            if spectral_pixel_index < correlation.shape[1] and correlation[world_index, spectral_pixel_index]:
                spectral_world_index = world_index
                break
    if spectral_world_index is None:
        spectral_world_index = min(world_naxis - 1, spectral_pixel_index)

    reference_pixels = []
    for pixel_axis in range(pixel_naxis):
        crpix = float(header.get(f"CRPIX{pixel_axis + 1}", 1.0))
        reference_pixels.append(np.full(channel_end - channel_start + 1, crpix - 1.0, dtype=np.float64))
    reference_pixels[spectral_pixel_index] = np.arange(
        channel_start, channel_end + 1, dtype=np.float64
    )
    world_values = wcs.pixel_to_world_values(*reference_pixels)
    spectral_coordinates = np.asarray(world_values[spectral_world_index], dtype=np.float64)
    if spectral_coordinates.ndim != 1 or spectral_coordinates.size != (channel_end - channel_start + 1):
        spectral_coordinates = np.ravel(spectral_coordinates).astype(np.float64, copy=False)

    axis_type = axis_types[spectral_world_index] if spectral_world_index < len(axis_types) else ""
    axis_unit = ""
    if hasattr(wcs, "world_axis_units") and spectral_world_index < len(wcs.world_axis_units):
        axis_unit = wcs.world_axis_units[spectral_world_index] or ""
    axis_label = str(header.get(f"CTYPE{spectral_fits_axis}", f"AXIS{spectral_fits_axis}")).strip()
    logger.info(
        "[moment] spectral axis fits_axis=%s world_axis=%s type=%s unit=%s channels=%s..%s",
        spectral_fits_axis,
        spectral_world_index,
        axis_type or axis_label,
        axis_unit or "-",
        channel_start,
        channel_end,
    )
    return {
        "coordinates": spectral_coordinates,
        "axis_type": axis_type or axis_label,
        "axis_unit": axis_unit,
        "axis_label": axis_label,
        "fits_axis": spectral_fits_axis,
    }


def _integrated_spacing(coords: np.ndarray) -> np.ndarray:
    if coords.size == 0:
        return np.zeros(0, dtype=np.float64)
    if coords.size == 1:
        return np.ones(1, dtype=np.float64)
    edges = np.empty(coords.size + 1, dtype=np.float64)
    edges[1:-1] = 0.5 * (coords[:-1] + coords[1:])
    edges[0] = coords[0] - 0.5 * (coords[1] - coords[0])
    edges[-1] = coords[-1] + 0.5 * (coords[-1] - coords[-2])
    return np.abs(np.diff(edges))


def _moment_result_unit(order: int, bunit: str, spectral_unit: str) -> str:
    bunit = (bunit or "").strip()
    spectral_unit = (spectral_unit or "").strip()
    if order == 0:
        if bunit and spectral_unit:
            return f"{bunit} {spectral_unit}"
        return bunit or spectral_unit
    if order == 1:
        return spectral_unit
    if order == 2:
        return f"{spectral_unit}^2" if spectral_unit else ""
    if order in {6, 8, 10}:
        return bunit
    return ""


# ── Worker: cube slice (single spectral plane) ────────────────────────────────


def worker_cube_slice(path: str, z_index: int) -> dict[str, Any]:
    """
    Extract a single Z-axis slice from a FITS cube.

    With memmap=True astropy reads only the bytes for the requested plane
    from disk – O(width * height * 4 bytes) I/O regardless of cube depth.
    """
    hdul, raw = _open_fits_memmap(path)
    try:
        data = _squeeze_to_3d(raw)
        if data.ndim != 3:
            raise ValueError("Slice endpoint requires a 3D cube.")
        if z_index < 0 or z_index >= data.shape[0]:
            raise ValueError(f"Slice index {z_index} out of range [0, {data.shape[0] - 1}].")
        # Materialise ONLY this plane – memmap ensures disk read is minimal.
        plane = np.asarray(data[z_index, :, :], dtype=np.float32)
    finally:
        hdul.close()

    range_min, range_max = _finite_range(plane)
    compression, data_b64 = _b64f32_compressed(plane)
    return {
        "width": int(plane.shape[1]),
        "height": int(plane.shape[0]),
        "scalar_type": "float32",
        "range_min": range_min,
        "range_max": range_max,
        "compression": compression,
        "data_base64": data_b64,
    }


# ── Worker: cube subvolume ────────────────────────────────────────────────────


def worker_cube_subvolume(
    path: str, x0: int, x1: int, y0: int, y1: int, z0: int, z1: int
) -> dict[str, Any]:
    hdul, raw = _open_fits_memmap(path)
    try:
        data = _squeeze_to_3d(raw)
        if data.ndim != 3:
            raise ValueError("Subvolume endpoint requires a 3D cube.")
        depth, height, width = data.shape
        x0, x1 = max(0, x0), min(x1, width - 1)
        y0, y1 = max(0, y0), min(y1, height - 1)
        z0, z1 = max(0, z0), min(z1, depth - 1)
        if x0 > x1 or y0 > y1 or z0 > z1:
            raise ValueError("Invalid subvolume ROI.")
        # Materialise only the requested sub-region.
        sub = np.asarray(data[z0 : z1 + 1, y0 : y1 + 1, x0 : x1 + 1], dtype=np.float32)
    finally:
        hdul.close()

    return {
        "width": int(sub.shape[2]),
        "height": int(sub.shape[1]),
        "depth": int(sub.shape[0]),
        "scalar_type": "float32",
        "data_base64": _b64f32(sub),
    }


# ── Worker: moment map ────────────────────────────────────────────────────────


def _moment_map_from_array(
    cube: np.ndarray,
    order: int,
    spectral_coordinates: np.ndarray,
    mask_enabled: bool,
    threshold_value: float,
) -> np.ndarray:
    """Pure numpy moment-map computation (no VTK dependency)."""
    spectral_values = np.asarray(spectral_coordinates, dtype=np.float64)
    spectral_delta = _integrated_spacing(spectral_values)

    subset = np.asarray(cube, dtype=np.float32)
    safe = np.where(np.isfinite(subset), subset, np.nan)
    if mask_enabled:
        safe = np.where(safe >= float(threshold_value), safe, np.nan)

    if order == 0:
        return np.nansum(safe * spectral_delta[:, None, None], axis=0, dtype=np.float32)

    if order == 1:
        weights = spectral_delta[:, None, None]
        m0 = np.nansum(safe * weights, axis=0, dtype=np.float32)
        num = np.nansum(safe * spectral_values[:, None, None] * weights, axis=0, dtype=np.float32)
        out = np.full(m0.shape, np.nan, dtype=np.float32)
        valid = np.isfinite(m0) & (m0 != 0.0)
        out[valid] = num[valid] / m0[valid]
        return out

    if order == 2:
        weights = spectral_delta[:, None, None]
        m0 = np.nansum(safe * weights, axis=0, dtype=np.float32)
        m1_num = np.nansum(safe * spectral_values[:, None, None] * weights, axis=0, dtype=np.float32)
        m1 = np.full(m0.shape, np.nan, dtype=np.float32)
        valid = np.isfinite(m0) & (m0 != 0.0)
        m1[valid] = m1_num[valid] / m0[valid]
        diff = spectral_values[:, None, None] - m1[None, :, :]
        num = np.nansum(safe * diff * diff * weights, axis=0, dtype=np.float32)
        out = np.full(m0.shape, np.nan, dtype=np.float32)
        out[valid] = num[valid] / m0[valid]
        return out

    if order == 6:
        return np.sqrt(np.nanmean(safe * safe, axis=0, dtype=np.float32))

    if order == 8:
        return np.nanmax(safe, axis=0)

    if order == 10:
        return np.nanmin(safe, axis=0)

    raise ValueError(f"Unsupported moment order: {order}.")


def worker_moment(
    path: str,
    order: int,
    channel_start: int,
    channel_end: int,
    mask_enabled: bool,
    threshold_value: float,
) -> dict[str, Any]:
    """
    Compute a moment map reading only [channel_start:channel_end] planes.
    With memmap=True the slice read is O(nchannels * ny * nx) – not the
    full cube size.
    """
    hdul, raw = _open_fits_memmap(path)
    try:
        header = hdul[0].header.copy()
        data = _squeeze_to_3d(raw)
        if data.ndim != 3:
            raise ValueError("Moment products require a cube dataset.")
        z0 = max(0, min(int(channel_start), data.shape[0] - 1))
        z1 = max(0, min(int(channel_end), data.shape[0] - 1))
        if z0 > z1:
            raise ValueError("Invalid channel range.")
        spectral = _build_spectral_coordinate_metadata(header, z0, z1)
        subset = np.asarray(data[z0 : z1 + 1, :, :], dtype=np.float32)
        image = _moment_map_from_array(
            subset,
            order,
            spectral["coordinates"],
            mask_enabled,
            threshold_value,
        )
        bunit = str(header.get("BUNIT", "")).strip()
        moment_unit = _moment_result_unit(order, bunit, str(spectral["axis_unit"]))
    finally:
        hdul.close()

    if not np.isfinite(image).any():
        raise ValueError("No valid voxels found for the selected moment parameters.")

    range_min, range_max = _finite_range(image)
    return {
        "width": int(image.shape[1]),
        "height": int(image.shape[0]),
        "scalar_type": "float32",
        "range_min": range_min,
        "range_max": range_max,
        "spectral_axis_type": str(spectral["axis_type"]),
        "spectral_axis_unit": str(spectral["axis_unit"]),
        "moment_unit": moment_unit,
        "bunit": bunit,
        "data_base64": _b64f32(image),
    }


# ── Worker: isosurface ────────────────────────────────────────────────────────


def worker_isosurface(path: str, width: int, height: int, depth: int, threshold: float) -> dict[str, Any]:
    """
    Compute a VTK isosurface (vtkFlyingEdges3D) from a FITS cube.

    The cube is loaded with memmap=True and materialised fully only in the
    worker process; the main FastAPI process never touches the large array.
    """
    try:
        import vtk  # noqa: PLC0415
        from vtk.util.numpy_support import vtk_to_numpy  # noqa: PLC0415
    except ImportError as exc:
        raise RuntimeError(f"VTK Python bindings are required: {exc}") from exc

    hdul, raw = _open_fits_memmap(path)
    try:
        data = _squeeze_to_3d(raw)
        if data.ndim != 3:
            raise ValueError("Isosurface endpoint requires a 3D cube.")
        cube = np.ascontiguousarray(data, dtype=np.float32)
    finally:
        hdul.close()

    cube_bytes = cube.tobytes()
    image_import = vtk.vtkImageImport()
    image_import.CopyImportVoidPointer(cube_bytes, len(cube_bytes))
    image_import.SetDataScalarTypeToFloat()
    image_import.SetNumberOfScalarComponents(1)
    image_import.SetDataExtent(0, width - 1, 0, height - 1, 0, depth - 1)
    image_import.SetWholeExtent(0, width - 1, 0, height - 1, 0, depth - 1)
    image_import.SetDataSpacing(1.0, 1.0, 1.0)
    image_import.SetDataOrigin(0.0, 0.0, 0.0)
    image_import.Update()

    contour = vtk.vtkFlyingEdges3D()
    contour.SetInputConnection(image_import.GetOutputPort())
    contour.SetValue(0, float(threshold))
    contour.ComputeNormalsOff()
    contour.ComputeGradientsOff()
    contour.Update()

    mesh = contour.GetOutput()
    num_points = int(mesh.GetNumberOfPoints())
    num_polys = int(mesh.GetNumberOfPolys())
    if num_points == 0 or num_polys == 0:
        raise ValueError(f"Empty isosurface mesh for threshold {threshold}.")

    pts = vtk_to_numpy(mesh.GetPoints().GetData()).astype(np.float32).reshape(-1)
    polys = vtk_to_numpy(mesh.GetPolys().GetData()).astype(np.int32)
    pts_compression, pts_b64 = _b64f32_compressed(pts)
    polys_compression, polys_b64 = _b64i32_compressed(polys)

    return {
        "compression": pts_compression,
        "points_base64": pts_b64,
        "polys_base64": polys_b64,
        "num_points": num_points,
        "num_polys": num_polys,
    }


# ── Worker: Position-Velocity diagram ────────────────────────────────────────


def _sample_polyline(vertices: list[tuple[int, int]]) -> tuple[list[tuple[int, int]], float]:
    sampled: list[tuple[int, int]] = []
    total = 0.0
    for seg in range(1, len(vertices)):
        s, e = vertices[seg - 1], vertices[seg]
        dx, dy = e[0] - s[0], e[1] - s[1]
        steps = max(abs(dx), abs(dy))
        if steps <= 0:
            if not sampled or sampled[-1] != s:
                sampled.append(s)
            continue
        for step in range(steps + 1):
            t = float(step) / float(steps)
            pt = (int(round(s[0] + t * dx)), int(round(s[1] + t * dy)))
            if sampled and sampled[-1] == pt:
                continue
            if sampled:
                total += float(np.hypot(pt[0] - sampled[-1][0], pt[1] - sampled[-1][1]))
            sampled.append(pt)
    return sampled, total


def _local_normal(pts: list[tuple[int, int]], i: int) -> tuple[float, float]:
    prev = pts[i if i == 0 else i - 1]
    nxt = pts[i + 1 if i + 1 < len(pts) else i]
    tx, ty = float(nxt[0] - prev[0]), float(nxt[1] - prev[1])
    length = float(np.hypot(tx, ty))
    if length <= 0:
        return 0.0, 1.0
    return -ty / length, tx / length


def worker_pv(path: str, vertices: list[list[int]], width_pixels: int) -> dict[str, Any]:
    hdul, raw = _open_fits_memmap(path)
    try:
        data = _squeeze_to_3d(raw)
        if data.ndim != 3:
            raise ValueError("PV extraction requires a 3D cube.")

        cleaned = []
        for v in vertices:
            if len(v) >= 2:
                pt = (int(v[0]), int(v[1]))
                if not cleaned or cleaned[-1] != pt:
                    cleaned.append(pt)
        if len(cleaned) < 2:
            raise ValueError("At least two distinct vertices required for a PV cut.")

        sampled, total_length = _sample_polyline(cleaned)
        depth, height, width = data.shape
        half = 0.5 * float(max(1, width_pixels) - 1)
        x_candidates = [pt[0] for pt in sampled]
        y_candidates = [pt[1] for pt in sampled]
        spatial_margin = max(1, int(np.ceil(abs(half))) + 2)
        slab_x0 = max(0, min(x_candidates) - spatial_margin)
        slab_x1 = min(width - 1, max(x_candidates) + spatial_margin)
        slab_y0 = max(0, min(y_candidates) - spatial_margin)
        slab_y1 = min(height - 1, max(y_candidates) + spatial_margin)

        # Materialise only the spatial slab needed by the PV path. This still reads
        # the full spectral depth, but avoids eager loading the entire cube volume.
        cube = np.asarray(data[:, slab_y0 : slab_y1 + 1, slab_x0 : slab_x1 + 1], dtype=np.float32)
        logger.info(
            "[pv] slab read full_depth=%s slab_x=%s..%s slab_y=%s..%s width_pixels=%s samples=%s",
            depth,
            slab_x0,
            slab_x1,
            slab_y0,
            slab_y1,
            width_pixels,
            len(sampled),
        )
    finally:
        hdul.close()

    sampled_local = [(pt[0] - slab_x0, pt[1] - slab_y0) for pt in sampled]
    x_samples = len(sampled_local)
    positions = np.zeros(x_samples, dtype=np.float32)
    pv = np.full((depth, x_samples), np.nan, dtype=np.float32)
    valid_samples = 0

    for i in range(1, x_samples):
        positions[i] = positions[i - 1] + float(
            np.hypot(
                sampled_local[i][0] - sampled_local[i - 1][0],
                sampled_local[i][1] - sampled_local[i - 1][1],
            )
        )

    slab_height = cube.shape[1]
    slab_width = cube.shape[2]
    for si, center in enumerate(sampled_local):
        nx, ny = _local_normal(sampled_local, si)
        for z in range(depth):
            vals: list[float] = []
            for off in range(max(1, width_pixels)):
                cx = int(round(center[0] + (float(off) - half) * nx))
                cy = int(round(center[1] + (float(off) - half) * ny))
                if 0 <= cx < slab_width and 0 <= cy < slab_height:
                    v_val = float(cube[z, cy, cx])
                    if np.isfinite(v_val):
                        vals.append(v_val)
            if vals:
                pv[z, si] = float(np.mean(vals))
                valid_samples += 1

    if valid_samples <= 0:
        raise ValueError("No valid data found along the PV cut.")

    pos_compression, pos_b64 = _b64f32_compressed(positions)
    data_compression, data_b64 = _b64f32_compressed(pv.reshape(-1))
    return {
        "num_samples": x_samples,
        "depth": int(pv.shape[0]),
        "scalar_type": "float32",
        "compression": pos_compression,
        "positions_base64": pos_b64,
        "data_base64": data_b64,
        "computed_on": "spatial_slab",
        "width_pixels": width_pixels,
        "vertex_count": len(cleaned),
        "total_length": float(total_length),
        "valid_samples": valid_samples,
    }


# ── Worker: image full / preview ──────────────────────────────────────────────


def _build_preview(image: np.ndarray, max_side: int) -> tuple[np.ndarray, float]:
    h, w = image.shape
    longest = max(w, h)
    limit = max(1, int(max_side))
    if longest <= limit:
        return np.ascontiguousarray(image, dtype=np.float32), 1.0
    scale = float(limit) / float(longest)
    pw = max(1, int(round(w * scale)))
    ph = max(1, int(round(h * scale)))
    xi = np.clip(
        np.floor((np.arange(pw, dtype=np.float64) + 0.5) * (w / pw)).astype(np.int64), 0, w - 1
    )
    yi = np.clip(
        np.floor((np.arange(ph, dtype=np.float64) + 0.5) * (h / ph)).astype(np.int64), 0, h - 1
    )
    preview = np.ascontiguousarray(image[np.ix_(yi, xi)], dtype=np.float32)
    return preview, float(longest) / float(max(preview.shape[0], preview.shape[1]))


def worker_image_full(path: str) -> dict[str, Any]:
    hdul, raw = _open_fits_memmap(path)
    try:
        data = _squeeze_to_3d(raw)
        if data.ndim != 2:
            raise ValueError("Image endpoint requires a 2D FITS dataset.")
        image = np.asarray(data, dtype=np.float32)
    finally:
        hdul.close()

    range_min, range_max = _finite_range(image)
    compression, data_b64 = _b64f32_compressed(image)
    return {
        "width": int(image.shape[1]),
        "height": int(image.shape[0]),
        "full_width": int(image.shape[1]),
        "full_height": int(image.shape[0]),
        "scalar_type": "float32",
        "range_min": range_min,
        "range_max": range_max,
        "is_preview": False,
        "preview_scale_factor": 1.0,
        "compression": compression,
        "data_base64": data_b64,
    }


def worker_image_preview(path: str, max_longest_side: int) -> dict[str, Any]:
    hdul, raw = _open_fits_memmap(path)
    try:
        data = _squeeze_to_3d(raw)
        if data.ndim != 2:
            raise ValueError("Image preview endpoint requires a 2D FITS dataset.")
        full_h, full_w = int(data.shape[0]), int(data.shape[1])
        image = np.asarray(data, dtype=np.float32)
    finally:
        hdul.close()

    preview, scale = _build_preview(image, max_longest_side)
    is_preview = preview.shape != image.shape
    range_min, range_max = _finite_range(preview)
    compression, data_b64 = _b64f32_compressed(preview)
    return {
        "width": int(preview.shape[1]),
        "height": int(preview.shape[0]),
        "full_width": full_w,
        "full_height": full_h,
        "scalar_type": "float32",
        "range_min": range_min,
        "range_max": range_max,
        "is_preview": is_preview,
        "preview_scale_factor": scale,
        "compression": compression,
        "data_base64": data_b64,
    }


def worker_cube_preview(path: str, downsample: int) -> dict[str, Any]:
    hdul, raw = _open_fits_memmap(path)
    try:
        data = _squeeze_to_3d(raw)
        if data.ndim != 3:
            raise ValueError("Cube preview requires a 3D cube.")
        stride = max(1, int(downsample))
        preview = np.asarray(data[::stride, ::stride, ::stride], dtype=np.float32)
    finally:
        hdul.close()

    range_min, range_max = _finite_range(preview)
    return {
        "width": int(preview.shape[2]),
        "height": int(preview.shape[1]),
        "depth": int(preview.shape[0]),
        "scalar_type": "float32",
        "range_min": range_min,
        "range_max": range_max,
        "data_base64": _b64f32(preview),
    }
