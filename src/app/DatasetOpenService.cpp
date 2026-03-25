#include "DatasetOpenService.h"

#include "AstroUtils.h"

DatasetOpenInfo DatasetOpenService::inspect(const DatasetOpenRequest &request) const
{
    DatasetOpenInfo result;
    result.filepath = request.filepath;

    if (request.filepath.isEmpty()) {
        result.errorMessage = "Empty dataset path.";
        return result;
    }

    const AstroUtils astro(request.filepath.toStdString());
    if (astro.isImage()) {
        result.kind = DatasetKind::Image;
        return result;
    }

    if (astro.isCube()) {
        result.kind = DatasetKind::Cube;
        return result;
    }

    result.errorMessage = "Unknown file format.";
    return result;
}
