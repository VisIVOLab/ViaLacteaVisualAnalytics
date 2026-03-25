# Architecture Note

## Purpose
This note summarizes the current architecture after the recent local refactorings and build modularization.

It describes what is already true in the codebase today, which boundaries are now stable, and which parts are the best candidates for a future local/remote split.

This is not a backend design document. No backend, remote rendering, or async execution is implemented yet.

## Current Module Map

### `visivo_shared_core`
Local non-UI application/core logic.

Current contents include:
- `DatasetOpenService`
- `DatasetOpenRequest`
- `DatasetOpenTypes`
- `ImageLayerImportService`
- `ImageLayerImportRequest`
- `ImageLayerImportResult`
- `AstroUtils`

Main role:
- dataset inspection/classification
- image-layer import validation
- scientific/file-level checks that do not require Qt widgets or VTK rendering widgets

### `visivo_shared_vtk`
Local VTK/runtime and processing logic.

Current contents include:
- `ImageLayerSet`
- `ImageLayer`
- `MomentProcessingService`
- `vtkFITSReader`
- `vtkFITSWriter`
- `vtkMomentMapFilter`
- `vtkLegendScaleActorWCS`
- `vtkInteractorStyleProfile`
- `ColorMaps`

Main role:
- local VTK runtime support
- local scientific/visual processing tied to VTK objects
- image layer runtime state

### Client GUI
Qt widget layer and UI orchestration.

Current contents include:
- `MainWindow`
- `vtkWindowImage`
- `vtkWindowCube`
- `DatasetWindowFactory`
- `ImageLayerController`
- `CubeViewController`
- dialogs and widget-only helpers

Main role:
- file dialogs, menus, actions, UI updates
- render-window orchestration
- widget lifecycle and interaction flow

## Stable Local Boundaries

The following local boundaries are now explicitly present in the code:

### Dataset opening
- `DatasetOpenRequest`
- `DatasetOpenService`
- `DatasetOpenInfo`

The GUI no longer owns dataset classification logic directly.

`DatasetOpenService` should currently be read as a local dataset inspection service, not as a generic remote-ready implementation.

### Image layer import validation
- `ImageLayerImportRequest`
- `ImageLayerImportService`
- `ImageLayerImportResult`

`vtkWindowImage` still owns the UI flow, but import validation is outside the widget.

`ImageLayerImportService` should currently be read as a local image-layer validation service, not as a transport-neutral validation API.

### Moment map processing
- `MomentMapRequest`
- `MomentMapResult`
- `MomentProcessingService`

`vtkWindowCube` still owns UI orchestration, but moment recomputation and LUT/range update are outside the widget.

### Image layer runtime state
- `ImageLayerSet`

`LayerListModel` no longer owns the full non-Qt layer management logic.

## Current Responsibilities

### Shared Core
Owns:
- explicit request/result-based local services
- dataset/file validation rules
- non-UI logic that may later map to service contracts

Does not own:
- widget flow
- render windows
- VTK rendering orchestration

### Shared VTK
Owns:
- VTK-side processing and helpers
- image layer runtime objects
- processing services tied to live VTK objects

Does not own:
- Qt widget behavior
- menus, dialogs, action wiring

### Client GUI
Owns:
- widget wiring
- action handling
- render triggers
- UI state updates
- choosing when to call local services

Does not need to own:
- dataset classification rules
- image-layer import validation rules
- moment-map recomputation rules

## Future Local/Remote Candidates

The best candidates for a future local/remote split are:

### `DatasetOpenService`
Reason:
- already has explicit request/result shape
- behavior is local and bounded
- good future candidate for remote dataset inspection or catalog-backed open flows

Current semantics:
- `DatasetOpenRequest.filepath` is currently a local desktop file path
- the current implementation is a local inspection service backed by `AstroUtils`

Future mapping:
- local implementation: inspect a local FITS path
- remote implementation: inspect a dataset reference or remote resource identifier
- the widget/client should not depend on how inspection is performed

### `ImageLayerImportService`
Reason:
- already has explicit request/result shape
- validates compatibility between a base dataset and a candidate layer
- good future candidate for remote validation

Current semantics:
- `ImageLayerImportRequest.baseDatasetPath` is currently a local desktop path for the base dataset
- `ImageLayerImportRequest.layerFilepath` is currently a local desktop path for the candidate image layer
- the current implementation is a local validation service backed by `AstroUtils`

Future mapping:
- local implementation: validate overlap and image compatibility using local FITS/WCS inspection
- remote implementation: validate a base dataset reference against a candidate layer reference
- the widget/client should not depend on how validation is performed

### `MomentProcessingService`
Reason:
- already has explicit request/result shape
- represents a real processing boundary
- strongest candidate for eventual async local execution or remote processing

Constraint today:
- it is still bound to live VTK runtime objects (`vtkMomentMapFilter`, `vtkLookupTable`)
- this means the contract exists, but it is not yet directly remotizable without one more decoupling step

## Non-Goals / Deferred Items

The following are intentionally not implemented yet:

- no backend
- no remote rendering
- no remote job execution
- no general async/job framework
- no generic service interfaces
- no large widget rewrite
- no full decomposition of `vtkWindowCube`
- no attempt to make all VTK runtime code backend-ready today

These are deferred on purpose. The current focus has been to create stable local boundaries first.

## Architecture Decisions Already Taken

### 1. Service logic moves out of widgets before any backend work
We first extracted local services and controllers from GUI classes instead of introducing premature remote abstractions.

### 2. Build modularization follows code boundaries
The build now reflects the code structure with:
- `visivo_shared_core`
- `visivo_shared_vtk`
- final GUI executable

### 3. Request/result contracts are preferred for stable service boundaries
Where useful, local services now expose explicit request/result types instead of ad-hoc primitive inputs.

### 4. GUI remains the orchestration shell
Widgets still own:
- file dialogs
- action triggers
- render calls
- UI updates

Services own bounded non-UI logic.

## Recommended Next Milestone

Define and document backend-ready service contracts for the existing local services, without implementing backend or async yet.

Practical next step:
- keep `DatasetOpenService`, `ImageLayerImportService`, and `MomentProcessingService` as the reference service boundaries
- document their intended local/remote mapping
- avoid further widget refactors unless they directly improve those boundaries

## Summary

The project is still a local Qt/VTK desktop application.

What changed is that several previously embedded pieces of logic are now explicit local boundaries with clearer ownership. This is the main architectural foundation for any future evolution toward async execution or client/server separation.
