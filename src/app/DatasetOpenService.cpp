#include "DatasetOpenService.h"

#include "AstroUtils.h"

#include <QDebug>

#include <algorithm>

DatasetOpenInfo DatasetOpenService::inspect(const DatasetOpenRequest &request) const
{
    DatasetOpenInfo result;
    result.filepath = request.filepath;

    if (request.filepath.isEmpty()) {
        result.errorMessage = "Empty dataset path.";
        return result;
    }

    const AstroUtils astro(request.filepath.toStdString());
    result.activeAxes = astro.getActiveAxisCount();
    result.degenerateAxesSummary = astro.degenerateAxesSummary();
    if (astro.isImage()) {
        qDebug().noquote()
                << QStringLiteral("[fits] active_axes=%1 degenerate_axes=%2 -> opening as 2D image")
                           .arg(result.activeAxes)
                           .arg(std::max(0, astro.getAxisCount() - result.activeAxes));
        result.kind = DatasetKind::Image;
        return result;
    }

    if (astro.isCube()) {
        qDebug().noquote()
                << QStringLiteral("[fits] active_axes=%1 degenerate_axes=%2 -> opening as cube")
                           .arg(result.activeAxes)
                           .arg(std::max(0, astro.getAxisCount() - result.activeAxes));
        result.kind = DatasetKind::Cube;
        return result;
    }

    qWarning().noquote()
            << QStringLiteral("[fits] active_axes=%1 degenerate_axes=%2 -> unsupported, falling back")
                       .arg(result.activeAxes)
                       .arg(std::max(0, astro.getAxisCount() - result.activeAxes));
    result.kind = DatasetKind::Image;
    result.errorMessage.clear();
    return result;
}
