#ifndef ImageLayerImportService_h
#define ImageLayerImportService_h

#include "ImageLayerImportResult.h"

class AstroUtils;

class ImageLayerImportService
{
public:
    ImageLayerImportService() = default;

    ImageLayerImportResult inspect(const AstroUtils &baseImage, const QString &filepath) const;
};

#endif
