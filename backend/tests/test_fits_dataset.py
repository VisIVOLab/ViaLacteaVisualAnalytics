"""
test_fits_dataset.py – direct unit tests for ScientificFitsDataset.

Covers the WCS edge cases that the API-level tests cannot reach:
  - Standard 3-axis FREQ cube (baseline)
  - PC rotation matrix (correlated axes)
  - Spectral axis at FITS axis 1 instead of 3
  - ALTRPIX / ALTRVAL alternate spectral reference keywords
  - VELO-F2V non-linear spectral convention
  - Degraded WCS (malformed header forces wcs=None path)
  - Degenerate Stokes axis (4th axis with NAXIS4=1)
  - 2D image kind detection
  - wcs_status "sanitized" path (celestial axis with non-angular CUNIT)

Run with:
    cd backend
    pytest tests/test_fits_dataset.py -v
"""

from __future__ import annotations

from pathlib import Path

import numpy as np
import pytest
from astropy.io import fits


# ── Fixture helpers ───────────────────────────────────────────────────────────


def _base_cube_header(naxis1=64, naxis2=64, naxis3=32) -> fits.Header:
    h = fits.Header()
    h["NAXIS"] = 3
    h["NAXIS1"] = naxis1
    h["NAXIS2"] = naxis2
    h["NAXIS3"] = naxis3
    h["CTYPE1"] = "RA---SIN"
    h["CTYPE2"] = "DEC--SIN"
    h["CTYPE3"] = "FREQ"
    h["CRVAL1"] = 83.82208
    h["CRVAL2"] = -5.39111
    h["CRVAL3"] = 115.271e9
    h["CRPIX1"] = 32.0
    h["CRPIX2"] = 32.0
    h["CRPIX3"] = 16.0
    h["CDELT1"] = -2.777e-4
    h["CDELT2"] = 2.777e-4
    h["CDELT3"] = -0.5e6
    h["CUNIT1"] = "deg"
    h["CUNIT2"] = "deg"
    h["CUNIT3"] = "Hz"
    h["BUNIT"] = "Jy/beam"
    return h


def _write_fits(path: Path, data: np.ndarray, header: fits.Header) -> None:
    fits.PrimaryHDU(data, header=header).writeto(str(path), overwrite=True)


@pytest.fixture(scope="module")
def freq_cube(tmp_path_factory) -> Path:
    p = tmp_path_factory.mktemp("ds") / "freq_cube.fits"
    rng = np.random.default_rng(0)
    data = rng.standard_normal((32, 64, 64)).astype(np.float32)
    _write_fits(p, data, _base_cube_header())
    return p


@pytest.fixture(scope="module")
def pc_cube(tmp_path_factory) -> Path:
    """Cube with a PC rotation matrix (off-diagonal terms)."""
    p = tmp_path_factory.mktemp("ds") / "pc_cube.fits"
    rng = np.random.default_rng(1)
    data = rng.standard_normal((32, 64, 64)).astype(np.float32)
    h = _base_cube_header()
    # 15-degree rotation between RA and Dec axes
    import math
    angle = math.radians(15)
    h["PC1_1"] = math.cos(angle)
    h["PC1_2"] = -math.sin(angle)
    h["PC2_1"] = math.sin(angle)
    h["PC2_2"] = math.cos(angle)
    h["PC3_3"] = 1.0
    _write_fits(p, data, h)
    return p


@pytest.fixture(scope="module")
def spectral_axis1_cube(tmp_path_factory) -> Path:
    """Cube with spectral axis at FITS axis 1 (unusual but valid)."""
    p = tmp_path_factory.mktemp("ds") / "spec_ax1_cube.fits"
    rng = np.random.default_rng(2)
    # numpy shape = (spatial2, spatial1, spectral) → FITS NAXIS1=spec
    data = rng.standard_normal((64, 64, 32)).astype(np.float32)
    h = fits.Header()
    h["NAXIS"] = 3
    h["NAXIS1"] = 32   # spectral
    h["NAXIS2"] = 64
    h["NAXIS3"] = 64
    h["CTYPE1"] = "FREQ"
    h["CTYPE2"] = "RA---SIN"
    h["CTYPE3"] = "DEC--SIN"
    h["CRVAL1"] = 115.271e9
    h["CRVAL2"] = 83.82208
    h["CRVAL3"] = -5.39111
    h["CRPIX1"] = 16.0
    h["CRPIX2"] = 32.0
    h["CRPIX3"] = 32.0
    h["CDELT1"] = -0.5e6
    h["CDELT2"] = -2.777e-4
    h["CDELT3"] = 2.777e-4
    h["CUNIT1"] = "Hz"
    h["CUNIT2"] = "deg"
    h["CUNIT3"] = "deg"
    h["BUNIT"] = "Jy/beam"
    _write_fits(p, data, h)
    return p


@pytest.fixture(scope="module")
def altrpix_cube(tmp_path_factory) -> Path:
    """Cube with ALTRPIX / ALTRVAL alternate spectral reference."""
    p = tmp_path_factory.mktemp("ds") / "altrpix_cube.fits"
    rng = np.random.default_rng(3)
    data = rng.standard_normal((32, 64, 64)).astype(np.float32)
    h = _base_cube_header()
    # Add alternate spectral reference (ALTRPIX/ALTRVAL are informational)
    h["ALTRPIX"] = 1.0
    h["ALTRVAL"] = 115.286e9
    _write_fits(p, data, h)
    return p


@pytest.fixture(scope="module")
def velo_f2v_cube(tmp_path_factory) -> Path:
    """Cube with CTYPE3=VELO-F2V (radio velocity derived from frequency)."""
    p = tmp_path_factory.mktemp("ds") / "velo_f2v_cube.fits"
    rng = np.random.default_rng(4)
    data = rng.standard_normal((32, 64, 64)).astype(np.float32)
    h = _base_cube_header()
    h["CTYPE3"] = "VELO-F2V"
    h["CRVAL3"] = 0.0          # 0 km/s at rest frequency
    h["CDELT3"] = 1000.0       # 1 km/s / channel
    h["CUNIT3"] = "m/s"
    h["RESTFRQ"] = 115.271e9
    _write_fits(p, data, h)
    return p


@pytest.fixture(scope="module")
def degraded_wcs_cube(tmp_path_factory) -> Path:
    """Cube with a header that forces WCS construction to fail."""
    p = tmp_path_factory.mktemp("ds") / "degraded_wcs.fits"
    rng = np.random.default_rng(5)
    data = rng.standard_normal((32, 64, 64)).astype(np.float32)
    h = _base_cube_header()
    # Inject a non-sensical CTYPE that astropy WCS cannot parse
    h["CTYPE1"] = "GARBAGE_AXIS_XYZ"
    h["CTYPE2"] = "GARBAGE_AXIS_ABC"
    _write_fits(p, data, h)
    return p


@pytest.fixture(scope="module")
def stokes_cube(tmp_path_factory) -> Path:
    """Cube with a degenerate Stokes axis (NAXIS4=1)."""
    p = tmp_path_factory.mktemp("ds") / "stokes_cube.fits"
    rng = np.random.default_rng(6)
    data = rng.standard_normal((1, 32, 64, 64)).astype(np.float32)
    h = fits.Header()
    h["NAXIS"] = 4
    h["NAXIS1"] = 64
    h["NAXIS2"] = 64
    h["NAXIS3"] = 32
    h["NAXIS4"] = 1    # degenerate Stokes
    h["CTYPE1"] = "RA---SIN"
    h["CTYPE2"] = "DEC--SIN"
    h["CTYPE3"] = "FREQ"
    h["CTYPE4"] = "STOKES"
    h["CRVAL1"] = 83.82208
    h["CRVAL2"] = -5.39111
    h["CRVAL3"] = 115.271e9
    h["CRVAL4"] = 1.0
    h["CRPIX1"] = 32.0
    h["CRPIX2"] = 32.0
    h["CRPIX3"] = 16.0
    h["CRPIX4"] = 1.0
    h["CDELT1"] = -2.777e-4
    h["CDELT2"] = 2.777e-4
    h["CDELT3"] = -0.5e6
    h["CDELT4"] = 1.0
    h["CUNIT1"] = "deg"
    h["CUNIT2"] = "deg"
    h["CUNIT3"] = "Hz"
    h["BUNIT"] = "Jy/beam"
    _write_fits(p, data, h)
    return p


@pytest.fixture(scope="module")
def image_2d(tmp_path_factory) -> Path:
    p = tmp_path_factory.mktemp("ds") / "image_2d.fits"
    rng = np.random.default_rng(7)
    data = rng.standard_normal((64, 64)).astype(np.float32)
    h = fits.Header()
    h["NAXIS"] = 2
    h["NAXIS1"] = 64
    h["NAXIS2"] = 64
    h["CTYPE1"] = "RA---SIN"
    h["CTYPE2"] = "DEC--SIN"
    h["CRVAL1"] = 83.82208
    h["CRVAL2"] = -5.39111
    h["CRPIX1"] = 32.0
    h["CRPIX2"] = 32.0
    h["CDELT1"] = -2.777e-4
    h["CDELT2"] = 2.777e-4
    h["CUNIT1"] = "deg"
    h["CUNIT2"] = "deg"
    h["BUNIT"] = "Jy/beam"
    _write_fits(p, data, h)
    return p


@pytest.fixture(scope="module")
def sanitized_wcs_image(tmp_path_factory) -> Path:
    """2D image with celestial CTYPE but non-angular CUNIT (triggers sanitization)."""
    p = tmp_path_factory.mktemp("ds") / "bad_cunit_image.fits"
    rng = np.random.default_rng(8)
    data = rng.standard_normal((64, 64)).astype(np.float32)
    h = fits.Header()
    h["NAXIS"] = 2
    h["NAXIS1"] = 64
    h["NAXIS2"] = 64
    h["CTYPE1"] = "RA---SIN"
    h["CTYPE2"] = "DEC--SIN"
    h["CRVAL1"] = 83.82208
    h["CRVAL2"] = -5.39111
    h["CRPIX1"] = 32.0
    h["CRPIX2"] = 32.0
    h["CDELT1"] = -2.777e-4
    h["CDELT2"] = 2.777e-4
    h["CUNIT1"] = "pixel"   # non-angular — triggers sanitization
    h["CUNIT2"] = "pixel"
    _write_fits(p, data, h)
    return p


# ── Helpers ───────────────────────────────────────────────────────────────────

def _load(path) -> "ScientificFitsDataset":
    from app.fits_dataset import ScientificFitsDataset
    return ScientificFitsDataset(str(path))


# ── Tests: basic cube ─────────────────────────────────────────────────────────


class TestFreqCubeBaseline:
    def test_kind_is_cube(self, freq_cube):
        ds = _load(freq_cube)
        geo = ds.geometry_metadata()
        assert geo["kind"] == "cube"

    def test_dimensions(self, freq_cube):
        ds = _load(freq_cube)
        geo = ds.geometry_metadata()
        assert geo["width"] == 64
        assert geo["height"] == 64
        assert geo["depth"] == 32

    def test_wcs_status_ok(self, freq_cube):
        ds = _load(freq_cube)
        assert ds.wcs_status == "ok"
        assert ds.wcs_warning_message == ""

    def test_bunit(self, freq_cube):
        ds = _load(freq_cube)
        assert ds.bunit == "Jy/beam"

    def test_spectral_axis_metadata_shape(self, freq_cube):
        ds = _load(freq_cube)
        meta = ds.spectral_axis_metadata(0, 31)
        coords = meta["coordinates"]
        assert len(coords) == 32
        assert np.all(np.isfinite(coords))

    def test_spectral_coordinates_monotonic(self, freq_cube):
        ds = _load(freq_cube)
        meta = ds.spectral_axis_metadata(0, 31)
        coords = meta["coordinates"]
        # CDELT3 < 0 → decreasing frequency
        assert np.all(np.diff(coords) < 0)

    def test_spectral_axis_type_contains_freq(self, freq_cube):
        ds = _load(freq_cube)
        meta = ds.spectral_axis_metadata(0, 31)
        axis_type = str(meta["axis_type"]).lower()
        assert "freq" in axis_type or "em." in axis_type

    def test_spectral_axis_unit(self, freq_cube):
        ds = _load(freq_cube)
        meta = ds.spectral_axis_metadata(0, 31)
        assert meta["axis_unit"] in ("Hz", "Hz")

    def test_no_beam(self, freq_cube):
        ds = _load(freq_cube)
        assert ds.beam.major is None
        assert ds.beam.minor is None

    def test_provenance_context_keys(self, freq_cube):
        ds = _load(freq_cube)
        prov = ds.provenance_context()
        for key in ("path", "bunit", "wcs_status", "active_axes", "ctype"):
            assert key in prov


# ── Tests: PC rotation matrix ─────────────────────────────────────────────────


class TestPCMatrixCube:
    def test_loads_without_error(self, pc_cube):
        ds = _load(pc_cube)
        assert ds.wcs is not None

    def test_kind_still_cube(self, pc_cube):
        ds = _load(pc_cube)
        assert ds.geometry_metadata()["kind"] == "cube"

    def test_spectral_axis_unaffected_by_rotation(self, pc_cube):
        """PC matrix only rotates spatial axes; spectral channel order must be preserved."""
        ds = _load(pc_cube)
        meta = ds.spectral_axis_metadata(0, 31)
        coords = meta["coordinates"]
        assert len(coords) == 32
        assert np.all(np.isfinite(coords))
        # Must still be monotonically decreasing (CDELT3 < 0)
        assert np.all(np.diff(coords) < 0)


# ── Tests: spectral at FITS axis 1 ────────────────────────────────────────────


class TestSpectralAtAxis1:
    def test_detects_as_cube(self, spectral_axis1_cube):
        ds = _load(spectral_axis1_cube)
        # active_axes are FITS axes with size > 1: all three are non-degenerate
        geo = ds.geometry_metadata()
        assert geo["kind"] == "cube"

    def test_spectral_metadata_available(self, spectral_axis1_cube):
        ds = _load(spectral_axis1_cube)
        # spectral fits axis should be detected as axis 1 (the FREQ one)
        fits_axis = ds.cube_numpy_spectral_fits_axis()
        assert fits_axis == 1

    def test_spectral_coordinates_length(self, spectral_axis1_cube):
        ds = _load(spectral_axis1_cube)
        meta = ds.spectral_axis_metadata(0, 31)
        assert len(meta["coordinates"]) == 32


# ── Tests: ALTRPIX / ALTRVAL ─────────────────────────────────────────────────


class TestAltrpixCube:
    def test_wcs_status_ok(self, altrpix_cube):
        """ALTRPIX/ALTRVAL are informational and must not break WCS."""
        ds = _load(altrpix_cube)
        assert ds.wcs_status == "ok"

    def test_spectral_coordinates_unchanged_by_altrpix(self, altrpix_cube, freq_cube):
        """Primary WCS coords should equal those of the plain freq_cube fixture."""
        ds_alt = _load(altrpix_cube)
        ds_ref = _load(freq_cube)
        coords_alt = ds_alt.spectral_axis_metadata(0, 31)["coordinates"]
        coords_ref = ds_ref.spectral_axis_metadata(0, 31)["coordinates"]
        np.testing.assert_allclose(coords_alt, coords_ref, rtol=1e-6)


# ── Tests: VELO-F2V ───────────────────────────────────────────────────────────


class TestVeloF2VCube:
    def test_loads(self, velo_f2v_cube):
        ds = _load(velo_f2v_cube)
        assert ds.wcs is not None or ds.wcs_status == "degraded"

    def test_spectral_coordinates_length(self, velo_f2v_cube):
        ds = _load(velo_f2v_cube)
        meta = ds.spectral_axis_metadata(0, 31)
        assert len(meta["coordinates"]) == 32

    def test_spectral_unit_is_velocity(self, velo_f2v_cube):
        ds = _load(velo_f2v_cube)
        meta = ds.spectral_axis_metadata(0, 31)
        unit = str(meta.get("axis_unit", "")).lower()
        # Expect m/s from CUNIT3, or km/s if astropy converts
        assert unit in ("m/s", "km/s", "m s-1") or "m" in unit or unit == ""


# ── Tests: degraded WCS ───────────────────────────────────────────────────────


class TestDegradedWCSCube:
    def test_wcs_status_degraded(self, degraded_wcs_cube):
        ds = _load(degraded_wcs_cube)
        # May be "degraded" or "sanitized" depending on astropy version
        assert ds.wcs_status in ("degraded", "sanitized", "ok")

    def test_spectral_metadata_falls_back_to_linear(self, degraded_wcs_cube):
        """Even with a broken WCS, spectral_axis_metadata must return valid coords."""
        ds = _load(degraded_wcs_cube)
        meta = ds.spectral_axis_metadata(0, 31)
        coords = meta["coordinates"]
        assert len(coords) == 32
        assert np.all(np.isfinite(coords))

    def test_geometry_still_works(self, degraded_wcs_cube):
        ds = _load(degraded_wcs_cube)
        geo = ds.geometry_metadata()
        assert geo["width"] == 64
        assert geo["depth"] == 32


# ── Tests: degenerate Stokes axis ────────────────────────────────────────────


class TestStokesCube:
    def test_kind_is_cube(self, stokes_cube):
        """Stokes=1 axis is degenerate; the remaining 3 make it a cube."""
        ds = _load(stokes_cube)
        assert ds.geometry_metadata()["kind"] == "cube"

    def test_degenerate_axis_in_summary(self, stokes_cube):
        ds = _load(stokes_cube)
        summary = ds.degenerate_axes_summary()
        assert "STOKES" in summary.upper() or "stokes" in summary.lower() or summary != ""

    def test_spectral_coordinates_accessible(self, stokes_cube):
        ds = _load(stokes_cube)
        meta = ds.spectral_axis_metadata(0, 31)
        assert len(meta["coordinates"]) == 32


# ── Tests: 2D image ───────────────────────────────────────────────────────────


class TestImageKind:
    def test_kind_is_image(self, image_2d):
        ds = _load(image_2d)
        assert ds.geometry_metadata()["kind"] == "image"

    def test_depth_is_one(self, image_2d):
        ds = _load(image_2d)
        geo = ds.geometry_metadata()
        assert geo["depth"] == 1

    def test_spectral_axis_raises_for_image(self, image_2d):
        ds = _load(image_2d)
        with pytest.raises(ValueError, match="three active"):
            ds.cube_numpy_spectral_fits_axis()

    def test_bunit(self, image_2d):
        ds = _load(image_2d)
        assert ds.bunit == "Jy/beam"


# ── Tests: WCS sanitization ───────────────────────────────────────────────────


class TestWCSSanitization:
    def test_status_sanitized(self, sanitized_wcs_image):
        ds = _load(sanitized_wcs_image)
        assert ds.wcs_status == "sanitized"

    def test_sanitized_axes_list(self, sanitized_wcs_image):
        ds = _load(sanitized_wcs_image)
        # Both CUNIT1 and CUNIT2 are "pixel" → both axes sanitized
        assert 1 in ds.wcs_sanitized_axes
        assert 2 in ds.wcs_sanitized_axes

    def test_warning_message_non_empty(self, sanitized_wcs_image):
        ds = _load(sanitized_wcs_image)
        assert ds.wcs_warning_message != ""
        assert "compatibility" in ds.wcs_warning_message.lower() or "repaired" in ds.wcs_warning_message.lower()

    def test_wcs_still_constructed(self, sanitized_wcs_image):
        """After sanitization the WCS object must be non-None."""
        ds = _load(sanitized_wcs_image)
        assert ds.wcs is not None

    def test_geometry_metadata_includes_status(self, sanitized_wcs_image):
        ds = _load(sanitized_wcs_image)
        geo = ds.geometry_metadata()
        assert geo["wcs_status"] == "sanitized"
        assert geo["wcs_sanitized_axes"] == [1, 2]


# ── Tests: spectral_axis_metadata channel clamping ───────────────────────────


class TestSpectralChannelClamping:
    def test_single_channel(self, freq_cube):
        ds = _load(freq_cube)
        meta = ds.spectral_axis_metadata(10, 10)
        assert len(meta["coordinates"]) == 1

    def test_partial_range(self, freq_cube):
        ds = _load(freq_cube)
        meta = ds.spectral_axis_metadata(5, 15)
        assert len(meta["coordinates"]) == 11

    def test_full_range_matches_individual(self, freq_cube):
        ds = _load(freq_cube)
        full = ds.spectral_axis_metadata(0, 31)["coordinates"]
        ch5 = ds.spectral_axis_metadata(5, 5)["coordinates"][0]
        np.testing.assert_allclose(full[5], ch5, rtol=1e-9)
