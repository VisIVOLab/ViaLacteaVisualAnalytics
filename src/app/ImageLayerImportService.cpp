#include "ImageLayerImportService.h"

#include "AstroUtils.h"

ImageLayerImportResult ImageLayerImportService::inspect(const ImageLayerImportRequest &request) const
{
    ImageLayerImportResult result{ request.layerFilepath, false, { } };

    if (request.layerFilepath.isEmpty()) {
        result.errorMessage = "Empty dataset path.";
        return result;
    }

    AstroUtils baseImage(request.baseDatasetPath.toStdString());
    if (!baseImage.overlap(request.layerFilepath.toStdString())) {
        result.errorMessage = "The regions do not overlap each other, the file cannot be imported.";
        return result;
    }

    AstroUtils other(request.layerFilepath.toStdString());
    if (!other.isImage()) {
        result.errorMessage = "Only FITS images can be imported as layers.";
        return result;
    }

    result.accepted = true;
    return result;
}
