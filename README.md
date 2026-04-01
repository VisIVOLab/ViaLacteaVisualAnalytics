# VisIVO Visual Analytics

VisIVO Visual Analytics is a scientific desktop application for interactive visualization of astronomical FITS datasets, including 2D images and 3D spectral cubes. It combines a native C++ client based on Qt and VTK with a Python FastAPI backend to support both local and remote workflows, with a remote-first architecture for scalable computation.

## Key Features

- Interactive 3D visualization of FITS cubes
- Interactive 2D visualization of FITS images
- Remote file browsing with FITS-aware metadata
- Remote full-resolution slice computation
- Remote isosurface extraction with local mesh rendering
- Preview to full-resolution workflow for responsive remote cube opening
- Asynchronous client updates for non-blocking interaction
- Client-side slice cache with neighbor prefetch for smoother navigation

## Architecture Overview

VisIVO Visual Analytics is split into two cooperating components:

- C++ desktop client:
  Built with Qt and VTK, it provides the interactive user interface, local rendering, camera controls, slice interaction, and mesh visualization.
- Python backend:
  Built with FastAPI and scientific Python libraries, it handles remote data access and computational tasks such as file inspection, slice extraction, moment computation, subvolume loading, and isosurface generation.

This separation keeps the rendering experience responsive on the desktop while moving data-intensive or computation-heavy work close to the data source. The same model is designed to evolve toward HPC execution, progressive data delivery, and remote rendering without changing the user-facing workflow.

## Main Workflows

### Open a Remote Cube

1. Browse the remote filesystem and select a FITS cube.
2. The client opens the dataset through the backend and receives dataset metadata.
3. A downsampled preview volume is loaded first so the cube appears quickly.
4. A full-resolution remote subvolume request starts automatically in the background.
5. When ready, the preview is replaced by the full-resolution cube in the same viewer.

### Navigate Slices

1. Slice selection uses the real dataset depth, not preview depth.
2. The 3D cube remains the currently displayed preview or full-resolution volume.
3. The 2D slice view is fetched remotely at full resolution.
4. Recent slices are cached client-side.
5. Neighbor slices are prefetched to improve responsiveness during navigation.

### Compute an Isosurface

1. The user selects isosurface mode and a threshold.
2. The client sends the threshold and dataset identifier to the backend.
3. The backend computes the mesh remotely.
4. The mesh is returned to the client.
5. The client reconstructs the VTK mesh and renders it locally in the existing cube viewer.

### Open a Remote 2D FITS Image

1. Browse the remote filesystem and select a 2D FITS file.
2. The client opens the dataset and detects that it is an image.
3. The image is fetched through the backend.
4. The same image viewer used for local FITS images displays the remote image.

## Remote File Browser

The remote file browser is designed to behave like a practical filesystem browser rather than a raw API client.

- Starts from the remote user's home directory
- Uses a two-panel layout
- Shows directories and files with metadata
- Supports FITS-focused filtering
- Remembers the last visited directory during the current app session
- Displays FITS header cards in a dedicated preview panel

The actual open flow remains unchanged: select a FITS file, open the dataset, then route it to the correct viewer by dataset kind.

## Setup

### Backend

Requirements:

- Python 3.10 or newer
- `pip`

Install dependencies:

```bash
cd backend
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

Run the backend:

```bash
uvicorn app.main:app --host 127.0.0.1 --port 8000
```

### Client

Requirements:

- CMake
- A C++17 compiler
- Qt
- VTK

Build:

```bash
cmake -S . -B build
cmake --build build --parallel
```

The desktop application binary is produced in the CMake build tree.

## API Overview

### `GET /files/list`

List the contents of a remote directory, including file type, size, modification time, and FITS detection.

### `POST /files/header`

Read the FITS header of a selected file and return the header cards without loading the full dataset.

### `POST /cube/slice`

Extract a full-resolution 2D slice from a remote cube for interactive slice navigation.

### `POST /cube/subvolume`

Fetch a subvolume from a remote cube. Currently used for full-resolution cube replacement after preview loading.

### `POST /products/isosurface`

Compute an isosurface remotely and return the resulting mesh payload for local rendering.

### `POST /image/full`

Fetch a full-resolution 2D FITS image for display in the standard image viewer.

## Design Decisions

- Python backend:
  Chosen for its strong scientific ecosystem, FITS tooling, numerical libraries, and a clean path toward cluster or HPC-side computation.
- C++ client:
  Chosen for interactive desktop performance, robust Qt UI integration, and high-quality VTK-based scientific rendering.

This balance keeps the application responsive for users while preserving flexibility for future distributed computation.

## Roadmap

- Additional slice UX refinements for high-latency remote environments
- ROI-based subvolume loading driven by the current view
- Progressive streaming and multi-resolution data delivery
- More advanced remote execution and HPC integration
- AI-assisted scientific workflows and a domain-aware scientific copilot

## Status

The system already supports working local and remote FITS workflows for:

- remote file browsing
- remote 2D FITS image loading
- remote cube preview and full-resolution upgrade
- remote slice extraction
- remote moment computation
- remote isosurface extraction

The architecture is designed to evolve further toward progressive data transport, large-scale remote execution, and AI-assisted analysis while keeping the same core desktop interaction model.
