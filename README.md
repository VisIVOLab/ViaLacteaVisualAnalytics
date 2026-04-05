# VisIVO Visual Analytics

VisIVO Visual Analytics is a scientific desktop application for interactive visualization of astronomical FITS datasets, including 2D images and 3D spectral cubes. It combines a native C++ client based on Qt and VTK with a Python FastAPI backend to support both local and remote workflows, with a remote-first architecture for scalable computation on HPC systems.

## Key Features

- Interactive 3D visualization of FITS cubes
- Interactive 2D visualization of FITS images
- Remote file browsing with FITS-aware metadata
- Remote full-resolution slice computation with client-side cache and neighbor prefetch
- Remote isosurface extraction with local VTK mesh rendering
- Preview-to-full-resolution workflow for responsive remote cube opening
- Moment map computation (orders 0, 1, 2, 6, 8, 10) locally and remotely
- Position-velocity diagram generation
- Token-based authentication for secure remote access
- Per-session dataset isolation for concurrent multi-user use
- HPC deployment via Apptainer/Singularity container and SLURM job scripts
- Asynchronous client updates for non-blocking interaction

## Architecture Overview

VisIVO Visual Analytics is split into two cooperating components.

The **C++ desktop client** is built with Qt 6 and VTK. It provides the interactive user interface, local rendering, camera controls, slice interaction, and mesh visualization. It communicates with the backend over HTTP, adding authentication and session headers to every request.

The **Python backend** is built with FastAPI, astropy, and numpy. It handles remote data access and computation: file inspection, slice extraction, moment computation, subvolume loading, PV diagrams, and isosurface generation. CPU-bound VTK and numpy work runs in a `ProcessPoolExecutor` to avoid blocking the ASGI event loop. FITS files are opened with `astropy.io.fits` in memory-mapped mode so only the requested pixels are read from disk.

This separation keeps the rendering experience responsive on the desktop while moving data-intensive work close to the data source on a server or HPC cluster.

## Authentication

Access to the backend is controlled by a **shared secret token** — there is no username/password system. The token is generated automatically the first time the backend starts, saved to `~/.visivo_token` (permissions `600`, readable only by the owning user), and also printed to the console:

```
[visivo] token: a3f9b2c1d4e57f8a...
```

The client reads the token automatically from the `VISIVO_TOKEN` environment variable or `~/.visivo_token`. For remote/HPC use, copy the token from the server and paste it in the Settings dialog under **Remote Backend → Access token**.

Anyone who knows the token can use the backend, so treat it like a password and avoid sharing it over unencrypted channels.

## Session Isolation

Each time a client opens a dataset the backend returns a **session ID** (a UUID). All subsequent requests for that dataset — slices, moments, isosurfaces, PV diagrams — carry the session ID in the `X-Visivo-Session` header. This allows multiple clients or multiple concurrent open datasets to be handled independently without interfering with each other. Sessions expire automatically after a configurable idle timeout.

## Main Workflows

### Open a Remote Cube

1. Browse the remote filesystem and select a FITS cube.
2. The client opens the dataset through the backend and receives metadata and a session ID.
3. A downsampled preview volume is loaded first so the cube appears immediately.
4. A full-resolution subvolume request starts automatically in the background.
5. When ready, the preview is replaced by the full-resolution cube in the same viewer.

### Navigate Slices

1. Slice selection uses the real dataset depth, not the preview depth.
2. The 2D slice view is fetched remotely at full resolution.
3. Recent slices are cached client-side.
4. Neighbor slices are prefetched to improve responsiveness during navigation.

### Compute a Moment Map

1. The user configures the moment order and optional channel range and mask.
2. In remote mode the request is sent to the backend with the session ID.
3. The resulting 2D image is returned and displayed in the moment viewer.

### Compute an Isosurface

1. The user selects a threshold value.
2. The backend computes the isosurface mesh from the remote FITS cube.
3. The mesh vertices and polygons are returned to the client.
4. The client reconstructs the VTK polydata and renders it locally in the cube viewer.

### Open a Remote 2D FITS Image

1. Browse the remote filesystem and select a FITS image.
2. The backend opens the dataset and returns image metadata.
3. A preview is fetched first; full resolution loads in the background.
4. The standard image viewer displays the result.

## Remote File Browser

The remote file browser behaves like a practical filesystem browser.

- Starts from the remote user's home directory
- Two-panel layout: directory tree on the left, metadata and FITS header on the right
- FITS-focused filtering and sorting
- Remembers the last visited directory during the current app session
- Displays FITS header cards in a dedicated preview panel

## Setup

### Backend — pip

Requirements: Python 3.10 or newer.

```bash
cd backend
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
uvicorn app.main:app --host 127.0.0.1 --port 8000
```

The token is printed to the console on first start and saved to `~/.visivo_token`.

### Backend — conda (recommended for HPC)

```bash
cd backend
conda env create -f conda-environment.yml
conda activate visivo
uvicorn app.main:app --host 0.0.0.0 --port 8000
```

### Backend — Apptainer/Singularity container

Build the container image:

```bash
apptainer build visivo-backend.sif apptainer/backend.def
```

Run it (bind-mount your data directory):

```bash
VISIVO_TOKEN=your-token apptainer run \
  --bind /path/to/fits/data:/data \
  visivo-backend.sif
```

### Backend — SLURM (HPC)

The script `scripts/launch_backend_slurm.sh` submits a SLURM job, waits for the node to be assigned, polls the health endpoint, and prints the SSH tunnel command when the backend is ready.

```bash
bash scripts/launch_backend_slurm.sh \
  --sif visivo-backend.sif \
  --bind /data \
  --partition gpu \
  --time 04:00:00
```

After the script completes, it prints something like:

```
Backend ready on hpc-node-42:8000
SSH tunnel: ssh -L 18000:hpc-node-42:8000 your-hpc-login
```

Open the SSH tunnel on your laptop, then set **Server URL** to `http://localhost:18000` in the Settings dialog.

### Client

Requirements: CMake, a C++17 compiler, Qt 6, VTK.

```bash
cmake -S . -B build
cmake --build build --parallel
```

### Client configuration

Open **Edit → Settings → Remote Backend**:

| Field | Description |
|---|---|
| Server URL | Full URL of the backend, e.g. `http://127.0.0.1:8000` or the SSH tunnel address |
| Access token | Leave empty to auto-load from `~/.visivo_token`, or paste the token shown in the backend console |

The 👁 button reveals the token in plain text; ⎘ copies it to the clipboard.

## API Reference

All endpoints are prefixed with `/v1/` and require the header `X-Visivo-Token: <token>`. Dataset-scoped endpoints also accept `X-Visivo-Session: <session-id>`.

| Method | Path | Description |
|---|---|---|
| `GET` | `/v1/health` | Health check (no auth required) |
| `GET` | `/v1/files/list` | List a remote directory |
| `POST` | `/v1/files/header` | Read FITS header cards |
| `POST` | `/v1/dataset/open` | Open a dataset; returns `dataset_id` and `session_id` |
| `GET` | `/v1/sessions` | List active sessions (admin) |
| `POST` | `/v1/cube/slice` | Extract a 2D slice from a cube |
| `POST` | `/v1/cube/subvolume` | Fetch a 3D subvolume |
| `POST` | `/v1/cube/preview` | Fetch a downsampled preview volume |
| `POST` | `/v1/cube/moment` | Compute a moment map |
| `POST` | `/v1/cube/pv` | Compute a position-velocity diagram |
| `POST` | `/v1/image/full` | Fetch a full-resolution 2D image |
| `POST` | `/v1/image/preview` | Fetch a downsampled image preview |
| `POST` | `/v1/products/isosurface` | Compute an isosurface mesh |

## Running Tests

```bash
cd backend
pip install -r requirements-dev.txt
pytest tests/ -v
```

Tests use synthetic FITS fixtures (no real data required) and an in-process ASGI client. VTK-dependent tests are skipped automatically when VTK is not available.

## Design Decisions

**Python backend** — chosen for its strong scientific ecosystem (astropy, numpy, scipy), FITS tooling, and a clean path toward cluster-side computation. `ProcessPoolExecutor` avoids blocking the async event loop for CPU-bound work. Memory-mapped FITS I/O ensures only the requested pixels are loaded.

**C++ client** — chosen for interactive desktop performance, robust Qt UI integration, and high-quality VTK-based scientific rendering.

**Shared-secret token** — simpler than OAuth for the typical use case (a single researcher or small team connecting to their own HPC allocation). The token is generated once per installation and never transmitted in URLs.

**Session isolation** — enables concurrent multi-user use and multiple simultaneously open datasets without server-side state collisions.

## Roadmap

- Settings dialog: SSH tunnel launcher integrated in the UI
- Progressive streaming and multi-resolution data delivery
- ROI-based subvolume loading driven by the current camera view
- AI-assisted scientific workflows and a domain-aware analysis copilot
- Multi-user token management and per-user session quotas

## Status

The system supports working local and remote FITS workflows for remote file browsing, 2D image loading, cube preview and full-resolution upgrade, slice extraction, moment computation, PV diagrams, and isosurface extraction. The architecture is designed to evolve toward progressive data transport, large-scale HPC execution, and AI-assisted analysis while keeping the same core desktop interaction model.
