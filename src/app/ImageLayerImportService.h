#ifndef ImageLayerImportService_h
#define ImageLayerImportService_h

#include "ImageLayerImportRequest.h"
#include "ImageLayerImportResult.h"

class ImageLayerImportService
{
public:
    ImageLayerImportService() = default;

    // Validates a local image-layer import request for the desktop client.
    ImageLayerImportResult inspect(const ImageLayerImportRequest &request) const;
};

#endif
