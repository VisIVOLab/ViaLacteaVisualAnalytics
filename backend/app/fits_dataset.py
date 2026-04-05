from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Any

import numpy as np
from astropy.io import fits
from astropy.wcs import WCS


@dataclass(frozen=True)
class BeamMetadata:
    major: float | None
    minor: float | None
    pa: float | None


class ScientificFitsDataset:
    """
    Minimal backend-side scientific FITS dataset helper.

    Centralizes:
    - FITS header access
    - WCS
    - active/degenerate axis handling
    - BUNIT
    - beam metadata
    - spectral axis identification for the current cube semantics
    - provenance-friendly metadata
    """

    def __init__(self, path: str | Path) -> None:
        self.path = str(Path(path))
        with fits.open(self.path, memmap=True, mode="readonly") as hdul:
            header = hdul[0].header.copy()
            shape = hdul[0].shape
        self.header = header
        self.shape = tuple(shape) if shape is not None else ()
        self.wcs = WCS(header)
        self.naxis = int(header.get("NAXIS", 0))
        self.axis_sizes = [int(header.get(f"NAXIS{i}", 1)) for i in range(1, self.naxis + 1)]
        self.active_axes = [axis for axis, size in enumerate(self.axis_sizes, start=1) if size > 1]
        self.degenerate_axes = [axis for axis, size in enumerate(self.axis_sizes, start=1) if size == 1]
        self.bunit = str(header.get("BUNIT", "")).strip()
        self.beam = BeamMetadata(
            major=float(header["BMAJ"]) if "BMAJ" in header else None,
            minor=float(header["BMIN"]) if "BMIN" in header else None,
            pa=float(header["BPA"]) if "BPA" in header else None,
        )

    def open_memmap(self):
        hdul = fits.open(self.path, memmap=True, mode="readonly")
        data = hdul[0].data
        if data is None:
            hdul.close()
            raise ValueError("FITS file contains no primary image data.")
        return hdul, data

    @staticmethod
    def squeeze_to_3d(data: np.ndarray) -> np.ndarray:
        arr = np.squeeze(data)
        while arr.ndim > 3:
            arr = arr[0]
        return arr

    def degenerate_axes_summary(self) -> str:
        entries: list[str] = []
        for axis in self.degenerate_axes:
            name = str(self.header.get(f"CTYPE{axis}", f"AXIS{axis}")).strip()
            val = float(self.header.get(f"CRVAL{axis}", 0.0))
            unit = str(self.header.get(f"CUNIT{axis}", "")).strip()
            label = f"{name}={val:g}"
            if unit and name.upper() != "STOKES":
                label += f" {unit}"
            label += " (1)"
            entries.append(label)
        return f"Collapsed axes: {', '.join(entries)}" if entries else ""

    def _axis_float(self, key: str, default: float = 0.0) -> float:
        return float(self.header.get(key, default))

    def geometry_metadata(self) -> dict[str, Any]:
        spacing = [self._axis_float("CDELT1", 1.0), self._axis_float("CDELT2", 1.0), self._axis_float("CDELT3", 1.0)]
        origin = [
            self._axis_float("CRVAL1") - spacing[0] * (self._axis_float("CRPIX1", 1.0) - 1.0),
            self._axis_float("CRVAL2") - spacing[1] * (self._axis_float("CRPIX2", 1.0) - 1.0),
            self._axis_float("CRVAL3") - spacing[2] * (self._axis_float("CRPIX3", 1.0) - 1.0),
        ]
        squeezed = [size for size in self.axis_sizes if size > 1]
        if len(self.active_axes) == 2:
            width, height, depth = squeezed[0], squeezed[1], 1
            kind = "image"
        elif len(self.active_axes) == 3:
            width, height, depth = squeezed[0], squeezed[1], squeezed[2]
            kind = "cube"
        else:
            width = squeezed[0] if squeezed else 1
            height = squeezed[1] if len(squeezed) > 1 else 1
            depth = 1
            kind = "image"
        return {
            "kind": kind,
            "active_axes": len(self.active_axes),
            "degenerate_axes_summary": self.degenerate_axes_summary(),
            "width": width,
            "height": height,
            "depth": depth,
            "spacing": spacing,
            "origin": origin,
            "ctype": [str(self.header.get(f"CTYPE{i}", "")) for i in range(1, 4)],
            "cunit": [str(self.header.get(f"CUNIT{i}", "")) for i in range(1, 4)],
            "crval": [self._axis_float(f"CRVAL{i}") for i in range(1, 4)],
            "crpix": [self._axis_float(f"CRPIX{i}", 1.0) for i in range(1, 4)],
            "cdelt": spacing,
        }

    def cube_numpy_spectral_fits_axis(self) -> int:
        if len(self.active_axes) < 3:
            raise ValueError("Scientific cube helpers require at least three active FITS axes.")
        return self.active_axes[-1]

    def spectral_axis_metadata(self, channel_start: int, channel_end: int) -> dict[str, Any]:
        spectral_fits_axis = self.cube_numpy_spectral_fits_axis()
        pixel_naxis = int(self.wcs.pixel_n_dim)
        world_naxis = int(self.wcs.world_n_dim)
        if pixel_naxis <= 0 or world_naxis <= 0:
            raise ValueError("FITS WCS is not available for spectral-coordinate computation.")

        axis_types = [ptype or "" for ptype in (self.wcs.world_axis_physical_types or [])]
        correlation = np.asarray(self.wcs.axis_correlation_matrix, dtype=bool)
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
            crpix = float(self.header.get(f"CRPIX{pixel_axis + 1}", 1.0))
            reference_pixels.append(
                np.full(channel_end - channel_start + 1, crpix - 1.0, dtype=np.float64)
            )
        reference_pixels[spectral_pixel_index] = np.arange(
            channel_start, channel_end + 1, dtype=np.float64
        )
        world_values = self.wcs.pixel_to_world_values(*reference_pixels)
        coordinates = np.asarray(world_values[spectral_world_index], dtype=np.float64)
        if coordinates.ndim != 1 or coordinates.size != (channel_end - channel_start + 1):
            coordinates = np.ravel(coordinates).astype(np.float64, copy=False)

        axis_type = axis_types[spectral_world_index] if spectral_world_index < len(axis_types) else ""
        axis_unit = ""
        if hasattr(self.wcs, "world_axis_units") and spectral_world_index < len(self.wcs.world_axis_units):
            axis_unit = self.wcs.world_axis_units[spectral_world_index] or ""
        axis_label = str(self.header.get(f"CTYPE{spectral_fits_axis}", f"AXIS{spectral_fits_axis}")).strip()
        return {
            "coordinates": coordinates,
            "axis_type": axis_type or axis_label,
            "axis_unit": axis_unit,
            "axis_label": axis_label,
            "fits_axis": spectral_fits_axis,
            "world_axis": spectral_world_index,
        }

    def provenance_context(self) -> dict[str, Any]:
        return {
            "path": self.path,
            "bunit": self.bunit,
            "beam_major": self.beam.major,
            "beam_minor": self.beam.minor,
            "beam_pa": self.beam.pa,
            "active_axes": list(self.active_axes),
            "degenerate_axes": list(self.degenerate_axes),
            "ctype": [str(self.header.get(f"CTYPE{i}", "")) for i in range(1, 4)],
            "cunit": [str(self.header.get(f"CUNIT{i}", "")) for i in range(1, 4)],
        }
