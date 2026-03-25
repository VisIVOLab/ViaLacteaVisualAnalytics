#ifndef ImageLayerImportService_h
#define ImageLayerImportService_h

#include "ImageLayerImportRequest.h"
#include "ImageLayerImportResult.h"

class ImageLayerImportService
{
public:
    ImageLayerImportService() = default;

    ImageLayerImportResult inspect(const ImageLayerImportRequest &request) const;
};

#endif
