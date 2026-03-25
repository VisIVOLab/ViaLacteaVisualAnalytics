#include "ImageLayerImportService.h"

#include "AstroUtils.h"

ImageLayerImportResult ImageLayerImportService::inspect(const AstroUtils &baseImage,
                                                        const QString &filepath) const
{
    ImageLayerImportResult result{ filepath, false, { } };

    if (filepath.isEmpty()) {
        result.errorMessage = "Empty dataset path.";
        return result;
    }

    if (!baseImage.overlap(filepath.toStdString())) {
        result.errorMessage = "The regions do not overlap each other, the file cannot be imported.";
        return result;
    }

    AstroUtils other(filepath.toStdString());
    if (!other.isImage()) {
        result.errorMessage = "Only FITS images can be imported as layers.";
        return result;
    }

    result.accepted = true;
    return result;
}
