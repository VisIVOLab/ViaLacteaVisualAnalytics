#ifndef ImageLayerImportRequest_h
#define ImageLayerImportRequest_h

#include <QString>

struct ImageLayerImportRequest
{
    // Current implementation expects a local desktop path for the base dataset.
    QString baseDatasetPath;

    // Current implementation expects a local desktop path for the candidate image layer.
    QString layerFilepath;
};

#endif
