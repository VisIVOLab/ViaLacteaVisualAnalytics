"""
conftest.py – pytest fixtures for the VisIVO backend test suite.

Provides:
  - synthetic_fits_image  : temporary 2D FITS file (64×64 pixels)
  - synthetic_fits_cube   : temporary 3D FITS cube (32×64×64 pixels)
  - auth_headers          : dict with the X-Visivo-Token header
  - client                : httpx.AsyncClient wired to the FastAPI test app
  - opened_image          : pre-opened image dataset_id + session_id
  - opened_cube           : pre-opened cube  dataset_id + session_id
"""

from __future__ import annotations

import os
import tempfile
from pathlib import Path
from typing import AsyncGenerator

import numpy as np
import pytest
import pytest_asyncio
from astropy.io import fits
from httpx import ASGITransport, AsyncClient

# ── Patch VISIVO_TOKEN before the app module is imported ─────────────────────
_TEST_TOKEN = "test-token-visivo-12345"
os.environ["VISIVO_TOKEN"] = _TEST_TOKEN


@pytest.fixture(scope="session")
def auth_headers() -> dict[str, str]:
    """Headers that satisfy the authentication dependency."""
    return {"X-Visivo-Token": _TEST_TOKEN}


# ── Synthetic FITS files ──────────────────────────────────────────────────────

def _write_fits_image(path: str) -> None:
    """Write a 64×64 float32 2D FITS image with realistic WCS headers."""
    rng = np.random.default_rng(42)
    data = rng.standard_normal((64, 64)).astype(np.float32)
    # Inject a few NaNs to test robust statistics.
    data[10:12, 10:12] = np.nan

    hdu = fits.PrimaryHDU(data)
    h = hdu.header
    h["NAXIS"]  = 2
    h["NAXIS1"] = 64
    h["NAXIS2"] = 64
    h["CTYPE1"] = "RA---SIN"
    h["CTYPE2"] = "DEC--SIN"
    h["CRVAL1"] = 83.82208
    h["CRVAL2"] = -5.39111
    h["CRPIX1"] = 32.0
    h["CRPIX2"] = 32.0
    h["CDELT1"] = -2.777e-4
    h["CDELT2"] =  2.777e-4
    h["CUNIT1"] = "deg"
    h["CUNIT2"] = "deg"
    h["BUNIT"]  = "Jy/beam"
    hdu.writeto(path, overwrite=True)


def _write_fits_cube(path: str) -> None:
    """Write a 32×64×64 float32 spectral cube with WCS headers."""
    rng = np.random.default_rng(7)
    # Simulate an emission line at channel 16.
    data = rng.standard_normal((32, 64, 64)).astype(np.float32) * 0.1
    data[14:18, 25:40, 25:40] += 5.0   # bright line region
    data[5, 30, 30] = np.nan             # isolated NaN

    hdu = fits.PrimaryHDU(data)
    h = hdu.header
    h["NAXIS"]  = 3
    h["NAXIS1"] = 64
    h["NAXIS2"] = 64
    h["NAXIS3"] = 32
    h["CTYPE1"] = "RA---SIN"
    h["CTYPE2"] = "DEC--SIN"
    h["CTYPE3"] = "FREQ"
    h["CRVAL1"] = 83.82208
    h["CRVAL2"] = -5.39111
    h["CRVAL3"] = 115.271e9          # CO(1-0) rest frequency (Hz)
    h["CRPIX1"] = 32.0
    h["CRPIX2"] = 32.0
    h["CRPIX3"] = 16.0
    h["CDELT1"] = -2.777e-4
    h["CDELT2"] =  2.777e-4
    h["CDELT3"] = -0.5e6             # -0.5 MHz / channel
    h["CUNIT1"] = "deg"
    h["CUNIT2"] = "deg"
    h["CUNIT3"] = "Hz"
    h["BUNIT"]  = "Jy/beam"
    hdu.writeto(path, overwrite=True)


@pytest.fixture(scope="session")
def synthetic_fits_image(tmp_path_factory) -> Path:
    p = tmp_path_factory.mktemp("fits") / "test_image.fits"
    _write_fits_image(str(p))
    return p


@pytest.fixture(scope="session")
def synthetic_fits_cube(tmp_path_factory) -> Path:
    p = tmp_path_factory.mktemp("fits") / "test_cube.fits"
    _write_fits_cube(str(p))
    return p


# ── httpx AsyncClient ─────────────────────────────────────────────────────────

@pytest_asyncio.fixture(scope="session")
async def client() -> AsyncGenerator[AsyncClient, None]:
    # Import here so VISIVO_TOKEN env var is already set.
    from app.main import app  # noqa: PLC0415

    async with AsyncClient(
        transport=ASGITransport(app=app),
        base_url="http://testserver",
    ) as ac:
        yield ac


# ── Pre-opened datasets ───────────────────────────────────────────────────────

@pytest_asyncio.fixture(scope="session")
async def opened_image(
    client: AsyncClient,
    auth_headers: dict,
    synthetic_fits_image: Path,
) -> dict[str, str]:
    """Open the synthetic 2D image and return {dataset_id, session_id, headers}."""
    resp = await client.post(
        "/v1/datasets/open",
        json={"path": str(synthetic_fits_image)},
        headers=auth_headers,
    )
    assert resp.status_code == 200
    data = resp.json()
    assert data["valid"], data["error"]
    assert data["kind"] == "image"
    session_id = data["session_id"]
    return {
        "dataset_id": data["dataset_id"],
        "session_id": session_id,
        "headers": {**auth_headers, "X-Visivo-Session": session_id},
    }


@pytest_asyncio.fixture(scope="session")
async def opened_cube(
    client: AsyncClient,
    auth_headers: dict,
    synthetic_fits_cube: Path,
) -> dict[str, str]:
    """Open the synthetic cube and return {dataset_id, session_id, headers}."""
    resp = await client.post(
        "/v1/datasets/open",
        json={"path": str(synthetic_fits_cube)},
        headers=auth_headers,
    )
    assert resp.status_code == 200
    data = resp.json()
    assert data["valid"], data["error"]
    assert data["kind"] == "cube"
    session_id = data["session_id"]
    return {
        "dataset_id": data["dataset_id"],
        "session_id": session_id,
        "headers": {**auth_headers, "X-Visivo-Session": session_id},
    }
