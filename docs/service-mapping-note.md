# Service Mapping Note

## Purpose
This note captures the current local service boundaries and the intended future mapping toward possible local/remote dual implementations.

It does not describe an implemented backend. It only documents the current state and the minimum gap that still exists before a clean split.

## `DatasetOpenService`

### Current contract
- `DatasetOpenRequest`
- `DatasetOpenInfo`
- `DatasetOpenService::inspect(const DatasetOpenRequest &request)`

### Current local implementation
- local inspection service in `visivo_shared_core`
- input is currently a local desktop FITS file path
- implementation uses `AstroUtils` to classify the dataset as image or cube

### Future remote equivalent
- inspect a remote dataset reference or catalog resource id
- return the same kind of inspection result needed by the client to decide how to open the dataset

### Missing step before clean split
- clarify the future request identity model beyond local file paths
- keep the client dependent on the contract, not on local inspection assumptions

## `ImageLayerImportService`

### Current contract
- `ImageLayerImportRequest`
- `ImageLayerImportResult`
- `ImageLayerImportService::inspect(const ImageLayerImportRequest &request)`

### Current local implementation
- local validation service in `visivo_shared_core`
- request currently carries:
  - local desktop path of the base dataset
  - local desktop path of the candidate image layer
- implementation uses `AstroUtils` to validate overlap and image compatibility

### Future remote equivalent
- validate a base dataset reference against a candidate layer reference
- return acceptance/error information needed by the client import flow

### Missing step before clean split
- replace path-based request semantics with stable dataset/layer references
- keep validation result small and client-oriented

## `MomentProcessingService`

### Current contract
- `MomentMapRequest`
- `MomentMapResult`
- `MomentProcessingService::process(const MomentMapRequest &request)`

### Current local implementation
- local VTK-bound processing service in `visivo_shared_vtk`
- request currently carries only the requested moment order
- implementation is bound to live `vtkMomentMapFilter` and `vtkLookupTable`
- processing updates local VTK runtime state and returns the output image range

### Future remote equivalent
- submit or execute moment-map computation on a remote processing side
- return a portable processing result that the client can then apply locally

### Missing step before clean split
- separate "compute moment result" from "apply result to local VTK/LUT state"
- reduce direct dependence on live VTK runtime objects inside the service contract boundary

## Non-Goals
This note does not imply that any of the following already exist:
- backend services
- remote rendering
- remote jobs
- async execution
- generic service interfaces
- widget rewrites

## Summary
Today the project already has three meaningful local service boundaries.

The best current pilot for a future local/remote dual implementation is `DatasetOpenService`, because it already has the cleanest contract and the lowest runtime coupling.

`ImageLayerImportService` is the next closest candidate.

`MomentProcessingService` is architecturally important, but still requires one more decoupling step before a clean local/remote split becomes realistic.
