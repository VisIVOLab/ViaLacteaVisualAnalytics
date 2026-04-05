from __future__ import annotations

import logging
from dataclasses import dataclass
from pathlib import Path
from typing import Any

import numpy as np
from astropy.io import fits
from astropy import units as u
from astropy.wcs import WCS

logger = logging.getLogger("visivo.fits")


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
        self.naxis = int(header.get("NAXIS", 0))
        self.axis_sizes = [int(header.get(f"NAXIS{i}", 1)) for i in range(1, self.naxis + 1)]
        self.active_axes = [axis for axis, size in enumerate(self.axis_sizes, start=1) if size > 1]
        self.degenerate_axes = [axis for axis, size in enumerate(self.axis_sizes, start=1) if size == 1]
        self.client_axis_to_fits_axis = {idx + 1: axis for idx, axis in enumerate(self.active_axes)}
        self.sanitized_header, self.wcs_sanitized_axes = self._sanitize_header_for_wcs(header)
        self.wcs = self._build_wcs(self.sanitized_header)
        self.wcs_status = self._determine_wcs_status()
        self.wcs_warning_message = self._build_wcs_warning_message()
        self.bunit = str(header.get("BUNIT", "")).strip()
        self.beam = BeamMetadata(
            major=float(header["BMAJ"]) if "BMAJ" in header else None,
            minor=float(header["BMIN"]) if "BMIN" in header else None,
            pa=float(header["BPA"]) if "BPA" in header else None,
        )
        self._log_axis_debug()

    @staticmethod
    def _is_celestial_ctype(ctype: str) -> bool:
        token = (ctype or "").strip().upper()
        if not token:
            return False
        celestial_prefixes = ("RA--", "DEC-", "GLON", "GLAT", "ELON", "ELAT", "HLON", "HLAT", "SLON", "SLAT", "TLON", "TLAT")
        return token.startswith(celestial_prefixes)

    @staticmethod
    def _is_angular_unit(unit: str) -> bool:
        raw = (unit or "").strip()
        if not raw:
            return True
        try:
            parsed = u.Unit(raw)
        except Exception:
            return False
        physical_type = str(parsed.physical_type)
        return physical_type == "angle"

    def _sanitize_header_for_wcs(self, header: fits.Header) -> tuple[fits.Header, list[int]]:
        sanitized = header.copy()
        sanitized_axes: list[int] = []
        for axis in range(1, self.naxis + 1):
            ctype = str(header.get(f"CTYPE{axis}", "")).strip()
            cunit_key = f"CUNIT{axis}"
            cunit = str(header.get(cunit_key, "")).strip()
            if self._is_celestial_ctype(ctype) and not self._is_angular_unit(cunit):
                logger.warning(
                    "Malformed FITS WCS: CTYPE%s='%s' with CUNIT%s='%s'. Overriding to 'deg' for compatibility.",
                    axis,
                    ctype,
                    axis,
                    cunit,
                )
                sanitized[cunit_key] = "deg"
                sanitized_axes.append(axis)
        return sanitized, sanitized_axes

    def _build_wcs(self, header: fits.Header) -> WCS | None:
        try:
            return WCS(header, relax=True, fix=True)
        except Exception as exc:
            logger.warning("[fits] WCS build failed for path=%s: %s. Falling back to degraded metadata-only mode.", self.path, exc)
            return None

    def _determine_wcs_status(self) -> str:
        if self.wcs is None:
            return "degraded"
        if self.wcs_sanitized_axes:
            return "sanitized"
        return "ok"

    def _build_wcs_warning_message(self) -> str:
        if self.wcs_status == "ok":
            return ""
        if self.wcs_status == "sanitized":
            axes = ", ".join(str(axis) for axis in self.wcs_sanitized_axes)
            return (
                f"WCS metadata was repaired for compatibility. "
                f"Celestial axis units were corrected on FITS axes: {axes}."
            )
        if self.wcs_sanitized_axes:
            axes = ", ".join(str(axis) for axis in self.wcs_sanitized_axes)
            return (
                f"WCS metadata was repaired on FITS axes {axes}, but full WCS construction still failed. "
                f"Using degraded linear axis metadata."
            )
        return "WCS construction failed. Using degraded linear axis metadata."

    def _log_axis_debug(self) -> None:
        ctype = [str(self.header.get(f"CTYPE{i}", "")).strip() for i in range(1, self.naxis + 1)]
        cunit = [str(self.header.get(f"CUNIT{i}", "")).strip() for i in range(1, self.naxis + 1)]
        logger.info(
            "[fits] path=%s ctype=%s cunit=%s active_axes=%s client_axis_to_fits_axis=%s wcs_status=%s sanitized_axes=%s",
            self.path,
            ctype,
            cunit,
            self.active_axes,
            self.client_axis_to_fits_axis,
            self.wcs_status,
            self.wcs_sanitized_axes,
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
            "wcs_status": self.wcs_status,
            "wcs_warning_message": self.wcs_warning_message,
            "wcs_sanitized_axes": list(self.wcs_sanitized_axes),
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

    def _linear_axis_coordinates(self, fits_axis: int, start: int, end: int) -> np.ndarray:
        crpix = float(self.sanitized_header.get(f"CRPIX{fits_axis}", 1.0))
        crval = float(self.sanitized_header.get(f"CRVAL{fits_axis}", 0.0))
        cdelt = float(self.sanitized_header.get(f"CDELT{fits_axis}", 1.0))
        pixels = np.arange(start, end + 1, dtype=np.float64)
        return crval + (pixels + 1.0 - crpix) * cdelt

    def spectral_axis_metadata(self, channel_start: int, channel_end: int) -> dict[str, Any]:
        spectral_fits_axis = self.cube_numpy_spectral_fits_axis()
        axis_label = str(self.header.get(f"CTYPE{spectral_fits_axis}", f"AXIS{spectral_fits_axis}")).strip()
        axis_unit = str(self.sanitized_header.get(f"CUNIT{spectral_fits_axis}", self.header.get(f"CUNIT{spectral_fits_axis}", ""))).strip()

        if self.wcs is None:
            logger.warning(
                "[fits] degraded spectral metadata path=%s fits_axis=%s axis_label=%s axis_unit=%s",
                self.path,
                spectral_fits_axis,
                axis_label,
                axis_unit or "-",
            )
            return {
                "coordinates": self._linear_axis_coordinates(spectral_fits_axis, channel_start, channel_end),
                "axis_type": axis_label,
                "axis_unit": axis_unit,
                "axis_label": axis_label,
                "fits_axis": spectral_fits_axis,
                "world_axis": max(0, spectral_fits_axis - 1),
            }

        pixel_naxis = int(self.wcs.pixel_n_dim)
        world_naxis = int(self.wcs.world_n_dim)
        if pixel_naxis <= 0 or world_naxis <= 0:
            return {
                "coordinates": self._linear_axis_coordinates(spectral_fits_axis, channel_start, channel_end),
                "axis_type": axis_label,
                "axis_unit": axis_unit,
                "axis_label": axis_label,
                "fits_axis": spectral_fits_axis,
                "world_axis": max(0, spectral_fits_axis - 1),
            }

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
        try:
            world_values = self.wcs.pixel_to_world_values(*reference_pixels)
            coordinates = np.asarray(world_values[spectral_world_index], dtype=np.float64)
            if coordinates.ndim != 1 or coordinates.size != (channel_end - channel_start + 1):
                coordinates = np.ravel(coordinates).astype(np.float64, copy=False)
        except Exception as exc:
            logger.warning(
                "[fits] WCS spectral conversion failed path=%s fits_axis=%s error=%s. Falling back to linear header coordinates.",
                self.path,
                spectral_fits_axis,
                exc,
            )
            coordinates = self._linear_axis_coordinates(spectral_fits_axis, channel_start, channel_end)

        axis_type = axis_types[spectral_world_index] if spectral_world_index < len(axis_types) else ""
        if hasattr(self.wcs, "world_axis_units") and spectral_world_index < len(self.wcs.world_axis_units):
            axis_unit = self.wcs.world_axis_units[spectral_world_index] or axis_unit
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
            "wcs_status": self.wcs_status,
            "wcs_warning_message": self.wcs_warning_message,
            "wcs_sanitized_axes": list(self.wcs_sanitized_axes),
            "ctype": [str(self.header.get(f"CTYPE{i}", "")) for i in range(1, 4)],
            "cunit": [str(self.header.get(f"CUNIT{i}", "")) for i in range(1, 4)],
        }
