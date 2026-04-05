"""
test_compute.py – unit tests for the compute worker functions.

These tests exercise the Python-level worker functions directly (without going
through FastAPI) so that WCS edge-cases, metadata correctness, and new
endpoints can be validated in isolation.

Run with:
    cd backend
    pytest tests/test_compute.py -v
"""

from __future__ import annotations

import base64
import struct
import tempfile
from pathlib import Path

import numpy as np
import pytest
from astropy.io import fits


# ── FITS fixture helpers ───────────────────────────────────────────────────────


def _write_freq_cube(path: str, nz: int = 32, ny: int = 16, nx: int = 16) -> None:
    """Standard FREQ cube – matches the conftest fixture used in test_api.py."""
    rng = np.random.default_rng(42)
    data = rng.standard_normal((nz, ny, nx)).astype(np.float32) * 0.1
    data[nz // 2 - 2 : nz // 2 + 2, 4:12, 4:12] += 5.0  # emission region
    hdu = fits.PrimaryHDU(data)
    h = hdu.header
    h["NAXIS"] = 3
    h["NAXIS1"] = nx
    h["NAXIS2"] = ny
    h["NAXIS3"] = nz
    h["CTYPE1"] = "RA---SIN"
    h["CTYPE2"] = "DEC--SIN"
    h["CTYPE3"] = "FREQ"
    h["CRVAL1"] = 83.82
    h["CRVAL2"] = -5.39
    h["CRVAL3"] = 115.271e9
    h["CRPIX1"] = nx / 2.0
    h["CRPIX2"] = ny / 2.0
    h["CRPIX3"] = nz / 2.0
    h["CDELT1"] = -2.777e-4
    h["CDELT2"] = 2.777e-4
    h["CDELT3"] = -0.5e6
    h["CUNIT1"] = "deg"
    h["CUNIT2"] = "deg"
    h["CUNIT3"] = "Hz"
    h["BUNIT"] = "Jy/beam"
    h["BMAJ"] = 0.001  # degrees
    h["BMIN"] = 0.0008
    h["BPA"] = 45.0
    hdu.writeto(path, overwrite=True)


def _write_vrad_cube(path: str, nz: int = 32, ny: int = 16, nx: int = 16) -> None:
    """Cube with VRAD (radio velocity) spectral axis in km/s."""
    rng = np.random.default_rng(7)
    data = rng.standard_normal((nz, ny, nx)).astype(np.float32) * 0.1
    data[14:18, 4:12, 4:12] += 3.0
    hdu = fits.PrimaryHDU(data)
    h = hdu.header
    h["NAXIS"] = 3
    h["NAXIS1"] = nx
    h["NAXIS2"] = ny
    h["NAXIS3"] = nz
    h["CTYPE1"] = "RA---SIN"
    h["CTYPE2"] = "DEC--SIN"
    h["CTYPE3"] = "VRAD"
    h["CRVAL1"] = 10.0
    h["CRVAL2"] = 20.0
    h["CRVAL3"] = 0.0      # km/s
    h["CRPIX1"] = nx / 2.0
    h["CRPIX2"] = ny / 2.0
    h["CRPIX3"] = nz / 2.0
    h["CDELT1"] = -2.777e-4
    h["CDELT2"] = 2.777e-4
    h["CDELT3"] = 1.0      # 1 km/s per channel
    h["CUNIT1"] = "deg"
    h["CUNIT2"] = "deg"
    h["CUNIT3"] = "km/s"
    h["BUNIT"] = "Jy/beam"
    hdu.writeto(path, overwrite=True)


def _write_pc_matrix_cube(path: str, nz: int = 32, ny: int = 16, nx: int = 16) -> None:
    """Cube with a PC rotation matrix (non-diagonal WCS)."""
    rng = np.random.default_rng(99)
    data = rng.standard_normal((nz, ny, nx)).astype(np.float32) * 0.1
    data[14:18, 4:12, 4:12] += 4.0
    hdu = fits.PrimaryHDU(data)
    h = hdu.header
    h["NAXIS"] = 3
    h["NAXIS1"] = nx
    h["NAXIS2"] = ny
    h["NAXIS3"] = nz
    h["CTYPE1"] = "RA---SIN"
    h["CTYPE2"] = "DEC--SIN"
    h["CTYPE3"] = "FREQ"
    h["CRVAL1"] = 83.82
    h["CRVAL2"] = -5.39
    h["CRVAL3"] = 115.271e9
    h["CRPIX1"] = nx / 2.0
    h["CRPIX2"] = ny / 2.0
    h["CRPIX3"] = nz / 2.0
    # PC matrix: 15-degree rotation in the RA/Dec plane
    import math
    angle = math.radians(15)
    h["PC1_1"] = math.cos(angle)
    h["PC1_2"] = -math.sin(angle)
    h["PC2_1"] = math.sin(angle)
    h["PC2_2"] = math.cos(angle)
    h["PC3_3"] = 1.0
    h["CDELT1"] = -2.777e-4
    h["CDELT2"] = 2.777e-4
    h["CDELT3"] = -0.5e6
    h["CUNIT1"] = "deg"
    h["CUNIT2"] = "deg"
    h["CUNIT3"] = "Hz"
    h["BUNIT"] = "Jy/beam"
    hdu.writeto(path, overwrite=True)


def _write_degenerate_cube(path: str) -> None:
    """Cube with degenerate Stokes axis (4D FITS squeezed to 3D effective)."""
    nz, ny, nx = 16, 8, 8
    rng = np.random.default_rng(3)
    data = rng.standard_normal((1, nz, ny, nx)).astype(np.float32) * 0.1
    data[0, 6:10, 2:6, 2:6] += 2.0
    hdu = fits.PrimaryHDU(data)
    h = hdu.header
    h["NAXIS"] = 4
    h["NAXIS1"] = nx
    h["NAXIS2"] = ny
    h["NAXIS3"] = nz
    h["NAXIS4"] = 1  # degenerate Stokes
    h["CTYPE1"] = "RA---SIN"
    h["CTYPE2"] = "DEC--SIN"
    h["CTYPE3"] = "FREQ"
    h["CTYPE4"] = "STOKES"
    h["CRVAL1"] = 0.0
    h["CRVAL2"] = 0.0
    h["CRVAL3"] = 1.4e9
    h["CRVAL4"] = 1.0
    h["CRPIX1"] = nx / 2.0
    h["CRPIX2"] = ny / 2.0
    h["CRPIX3"] = nz / 2.0
    h["CRPIX4"] = 1.0
    h["CDELT1"] = -1e-4
    h["CDELT2"] = 1e-4
    h["CDELT3"] = 1e6
    h["CDELT4"] = 1.0
    h["CUNIT1"] = "deg"
    h["CUNIT2"] = "deg"
    h["CUNIT3"] = "Hz"
    h["BUNIT"] = "K"
    hdu.writeto(path, overwrite=True)


def _decode_f32(b64: str) -> np.ndarray:
    return np.frombuffer(base64.b64decode(b64), dtype=np.float32)


def _decode_qt_zlib(b64: str) -> np.ndarray:
    import zlib
    raw = base64.b64decode(b64)
    expected = struct.unpack(">I", raw[:4])[0]
    return np.frombuffer(zlib.decompress(raw[4:])[:expected], dtype=np.float32)


# ── worker_moment: metadata fields ────────────────────────────────────────────


@pytest.fixture(scope="module")
def freq_cube_path(tmp_path_factory) -> Path:
    p = tmp_path_factory.mktemp("wcs") / "freq_cube.fits"
    _write_freq_cube(str(p))
    return p


@pytest.fixture(scope="module")
def vrad_cube_path(tmp_path_factory) -> Path:
    p = tmp_path_factory.mktemp("wcs") / "vrad_cube.fits"
    _write_vrad_cube(str(p))
    return p


@pytest.fixture(scope="module")
def pc_cube_path(tmp_path_factory) -> Path:
    p = tmp_path_factory.mktemp("wcs") / "pc_cube.fits"
    _write_pc_matrix_cube(str(p))
    return p


@pytest.fixture(scope="module")
def degen_cube_path(tmp_path_factory) -> Path:
    p = tmp_path_factory.mktemp("wcs") / "degen_cube.fits"
    _write_degenerate_cube(str(p))
    return p


class TestWorkerMomentMetadata:
    """Verify that worker_moment returns correct scientific metadata fields."""

    def test_freq_cube_m0_metadata(self, freq_cube_path):
        from app.compute import worker_moment
        result = worker_moment(str(freq_cube_path), 0, 0, 31, False, 0.0)
        assert result["bunit"] == "Jy/beam"
        assert result["spectral_axis_type"] == "FREQ"
        assert result["spectral_axis_unit"] == "Hz"
        # M0 unit = BUNIT × spectral_unit
        assert "Jy/beam" in result["moment_unit"]
        assert "Hz" in result["moment_unit"]

    def test_freq_cube_m1_metadata(self, freq_cube_path):
        from app.compute import worker_moment
        result = worker_moment(str(freq_cube_path), 1, 0, 31, False, 0.0)
        # M1 unit = spectral_unit only
        assert result["moment_unit"] == "Hz"

    def test_freq_cube_m2_metadata(self, freq_cube_path):
        from app.compute import worker_moment
        result = worker_moment(str(freq_cube_path), 2, 0, 31, False, 0.0)
        # M2 unit = spectral_unit^2
        assert result["moment_unit"] == "Hz^2"

    @pytest.mark.parametrize("order", [6, 8, 10])
    def test_intensity_moment_unit_equals_bunit(self, freq_cube_path, order):
        from app.compute import worker_moment
        result = worker_moment(str(freq_cube_path), order, 0, 31, False, 0.0)
        # M6/M8/M10 preserve the intensity unit
        assert result["moment_unit"] == "Jy/beam"

    def test_vrad_cube_m0_unit(self, vrad_cube_path):
        """km/s spectral axis should propagate into moment_unit."""
        from app.compute import worker_moment
        result = worker_moment(str(vrad_cube_path), 0, 0, 31, False, 0.0)
        assert result["spectral_axis_unit"] == "km/s"
        assert "km/s" in result["moment_unit"]

    def test_vrad_cube_m2_unit(self, vrad_cube_path):
        from app.compute import worker_moment
        result = worker_moment(str(vrad_cube_path), 2, 0, 31, False, 0.0)
        assert result["moment_unit"] == "km/s^2"


class TestWorkerMomentWCSEdgeCases:
    """Ensure worker_moment succeeds on WCS corner cases."""

    def test_pc_matrix_cube_m0(self, pc_cube_path):
        """PC rotation matrix must not prevent moment computation."""
        from app.compute import worker_moment
        result = worker_moment(str(pc_cube_path), 0, 0, 31, False, 0.0)
        arr = _decode_f32(result["data_base64"])
        assert np.isfinite(arr).any(), "M0 should have finite values for PC-matrix cube"
        assert result["bunit"] == "Jy/beam"
        assert result["spectral_axis_type"] == "FREQ"

    def test_degenerate_stokes_axis_m0(self, degen_cube_path):
        """4D cube with degenerate Stokes axis should be squeezed to 3D correctly."""
        from app.compute import worker_moment
        result = worker_moment(str(degen_cube_path), 0, 0, 15, False, 0.0)
        arr = _decode_f32(result["data_base64"])
        assert np.isfinite(arr).any()
        assert result["bunit"] == "K"

    def test_all_nan_channel_range_raises(self, freq_cube_path):
        """A channel range with no finite data must raise ValueError."""
        from app.compute import worker_moment
        # Set an absurdly high threshold so nothing passes the mask.
        with pytest.raises((ValueError, Exception)):
            worker_moment(str(freq_cube_path), 0, 0, 31, True, 1e10)

    def test_channel_range_clamping(self, freq_cube_path):
        """Out-of-range channel indices should be clamped, not raise."""
        from app.compute import worker_moment
        result = worker_moment(str(freq_cube_path), 0, -5, 999, False, 0.0)
        arr = _decode_f32(result["data_base64"])
        assert np.isfinite(arr).any()


class TestWorkerNoiseEstimate:
    """Verify worker_noise_estimate returns plausible MAD/sigma values."""

    def test_noise_shape(self, freq_cube_path):
        from app.compute import worker_noise_estimate
        result = worker_noise_estimate(str(freq_cube_path), 0, 5, 0, 5, 0, 15)
        assert result["num_channels"] == 16
        assert len(result["mad"]) == 16
        assert len(result["sigma"]) == 16

    def test_sigma_approx_1_4826_times_mad(self, freq_cube_path):
        from app.compute import worker_noise_estimate
        result = worker_noise_estimate(str(freq_cube_path), 0, 5, 0, 5, 0, 15)
        for mad, sigma in zip(result["mad"], result["sigma"]):
            if np.isfinite(mad) and np.isfinite(sigma) and mad > 0:
                assert abs(sigma / mad - 1.4826) < 1e-4

    def test_emission_region_has_higher_mad(self, freq_cube_path):
        """MAD in the emission region should exceed MAD in the noise region."""
        from app.compute import worker_noise_estimate
        # Noise region: corner pixels far from emission
        noise = worker_noise_estimate(str(freq_cube_path), 0, 3, 0, 3, 0, 31)
        # Emission region (4:12, 4:12, channels 14:18 have +5 injected)
        emission = worker_noise_estimate(str(freq_cube_path), 4, 12, 4, 12, 14, 17)
        noise_med = float(np.nanmedian(noise["sigma"]))
        emission_med = float(np.nanmedian(emission["sigma"]))
        assert emission_med > noise_med, (
            f"Emission σ={emission_med:.4f} should exceed noise σ={noise_med:.4f}"
        )

    def test_invalid_region_raises(self, freq_cube_path):
        from app.compute import worker_noise_estimate
        with pytest.raises((ValueError, Exception)):
            worker_noise_estimate(str(freq_cube_path), 10, 5, 0, 3, 0, 15)  # x0 > x1


class TestWorkerPvArcsec:
    """Verify that worker_pv returns physical arcsecond positions."""

    def test_pv_arcsec_present(self, freq_cube_path):
        from app.compute import worker_pv
        result = worker_pv(str(freq_cube_path), [[2, 8], [13, 8]], 1)
        assert "positions_arcsec_base64" in result
        assert result["spatial_unit"] == "arcsec"
        assert result["pixel_scale_arcsec_per_pixel"] > 0

    def test_pv_arcsec_monotone(self, freq_cube_path):
        """Arcsecond positions must be non-decreasing (cumulative path distance)."""
        from app.compute import worker_pv
        result = worker_pv(str(freq_cube_path), [[2, 8], [13, 8]], 1)
        arcsec = _decode_qt_zlib(result["positions_arcsec_base64"])
        assert np.all(np.diff(arcsec) >= 0), "Arcsec positions must be non-decreasing"

    def test_pv_arcsec_scale_relation(self, freq_cube_path):
        """positions_arcsec should equal positions_pixels * pixel_scale."""
        from app.compute import worker_pv
        result = worker_pv(str(freq_cube_path), [[2, 8], [13, 8]], 1)
        pixels = _decode_qt_zlib(result["positions_base64"])
        arcsec = _decode_qt_zlib(result["positions_arcsec_base64"])
        scale = result["pixel_scale_arcsec_per_pixel"]
        np.testing.assert_allclose(arcsec, pixels * scale, rtol=1e-5)


class TestMomentUnitHelper:
    """Test the _moment_unit helper in isolation."""

    def test_m0(self):
        from app.compute import _moment_unit
        assert _moment_unit(0, "Jy/beam", "Hz") == "Jy/beam Hz"

    def test_m0_no_bunit(self):
        from app.compute import _moment_unit
        assert _moment_unit(0, "", "Hz") == "Hz"

    def test_m1(self):
        from app.compute import _moment_unit
        assert _moment_unit(1, "Jy/beam", "km/s") == "km/s"

    def test_m2(self):
        from app.compute import _moment_unit
        assert _moment_unit(2, "Jy/beam", "km/s") == "km/s^2"

    def test_m2_no_spectral(self):
        from app.compute import _moment_unit
        assert _moment_unit(2, "Jy/beam", "") == ""

    @pytest.mark.parametrize("order", [6, 8, 10])
    def test_intensity_orders(self, order):
        from app.compute import _moment_unit
        assert _moment_unit(order, "Jy/beam", "Hz") == "Jy/beam"
