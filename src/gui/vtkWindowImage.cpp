#include "vtkWindowImage.h"
#include "ui_vtkWindowImage.h"

#include "AstroUtils.h"
#include "app/BackendClient.h"
#include "ColorMaps.h"
#include "CatalogueOverlayUtils.h"
#include "ImageLayerController.h"
#include "ImageLayerImportService.h"
#include "ImageLayerLoadTask.h"
#include "LUTCustomizerDialog.h"
#include "LayerListModel.h"
#include "ProfileWidget.h"
#include "vtkInteractorStyleProfile.h"
#include "vtkLegendScaleActorWCS.h"
#include "wcs.h"

#include <vtkCamera.h>
#include <vtkAxisActor2D.h>
#include <vtkContourTriangulator.h>
#include <vtkCoordinate.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkImageData.h>
#include <vtkImageStack.h>
#include <vtkInteractorStyleImage.h>
#include <vtkInteractorStyleUser.h>
#include <vtkLineSource.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkProperty2D.h>
#include <vtkRegularPolygonSource.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkScalarBarActor.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

#include <QActionGroup>
#include <QAction>
#include <QButtonGroup>
#include <QCheckBox>
#include <QDebug>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QInputDialog>
#include <QLabel>
#include <QMetaObject>
#include <QMessageBox>
#include <QSignalBlocker>
#include <QStringList>
#include <QtConcurrentRun>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <sstream>

using namespace Qt::StringLiterals;

namespace {
constexpr int overlayTickCount = 5;
constexpr double polygonClosureTolerance = 3.0;
constexpr double catalogueMarkerHalfSize = 3.0;
constexpr int maxCatalogueLabelCount = 200;

struct VisibleImageBounds2D
{
    bool valid{ false };
    double xmin{ 0. };
    double xmax{ 0. };
    double ymin{ 0. };
    double ymax{ 0. };
};

struct RegionStatistics
{
    int totalCount{ 0 };
    int validCount{ 0 };
    int blankedCount{ 0 };
    double minValue{ 0. };
    double maxValue{ 0. };
    double mean{ 0. };
    double median{ 0. };
    double stddev{ 0. };
    bool valid{ false };
};

enum class SanityLevel
{
    Ok,
    Warning,
    Unknown,
};

struct SanityReport
{
    SanityLevel level{ SanityLevel::Ok };
    QString summary;
    QString details;
};

double distance2d(const std::array<int, 2> &a, const std::array<int, 2> &b)
{
    const double dx = static_cast<double>(a[0] - b[0]);
    const double dy = static_cast<double>(a[1] - b[1]);
    return std::sqrt(dx * dx + dy * dy);
}

void buildAnnulusFill(vtkPoints *points, vtkCellArray *cells, vtkPolyData *polyData,
                      const std::array<int, 2> &center, double innerRadius, double outerRadius,
                      int sides = 96)
{
    points->Reset();
    cells->Reset();
    if (!polyData || !points || !cells || outerRadius <= 0.0 || innerRadius < 0.0
        || innerRadius >= outerRadius) {
        if (polyData) {
            polyData->Modified();
        }
        return;
    }

    const double cx = static_cast<double>(center[0]);
    const double cy = static_cast<double>(center[1]);
    const double twoPi = 2.0 * std::acos(-1.0);
    for (int i = 0; i < sides; ++i) {
        const double theta = twoPi * static_cast<double>(i) / static_cast<double>(sides);
        const double cosTheta = std::cos(theta);
        const double sinTheta = std::sin(theta);
        points->InsertNextPoint(cx + outerRadius * cosTheta, cy + outerRadius * sinTheta, 0.0);
        points->InsertNextPoint(cx + innerRadius * cosTheta, cy + innerRadius * sinTheta, 0.0);
    }

    cells->InsertNextCell(static_cast<vtkIdType>(2 * (sides + 1)));
    for (int i = 0; i <= sides; ++i) {
        const int wrapped = i % sides;
        const vtkIdType outer = static_cast<vtkIdType>(2 * wrapped);
        const vtkIdType inner = outer + 1;
        cells->InsertCellPoint(outer);
        cells->InsertCellPoint(inner);
    }

    polyData->SetPoints(points);
    polyData->SetPolys(nullptr);
    polyData->SetStrips(cells);
    polyData->Modified();
}

double computeBlankFraction(vtkImageData *imageData)
{
    if (!imageData) {
        return 0.0;
    }
    int extent[6];
    imageData->GetExtent(extent);
    const qsizetype total = static_cast<qsizetype>(extent[1] - extent[0] + 1)
            * static_cast<qsizetype>(extent[3] - extent[2] + 1)
            * static_cast<qsizetype>(std::max(1, extent[5] - extent[4] + 1));
    if (total <= 0) {
        return 0.0;
    }
    qsizetype blanked = 0;
    for (int z = extent[4]; z <= extent[5]; ++z) {
        for (int y = extent[2]; y <= extent[3]; ++y) {
            for (int x = extent[0]; x <= extent[1]; ++x) {
                if (!std::isfinite(imageData->GetScalarComponentAsDouble(x, y, z, 0))) {
                    ++blanked;
                }
            }
        }
    }
    return static_cast<double>(blanked) / static_cast<double>(total);
}

bool axisHasAnyMetadata(const QString &ctype, const QString &cunit, double crval, double crpix, double cdelt)
{
    return !ctype.trimmed().isEmpty() || !cunit.trimmed().isEmpty() || std::isfinite(crval)
            || std::isfinite(crpix) || std::isfinite(cdelt);
}

bool axisHasLinearWcs(const QString &ctype, double crval, double crpix, double cdelt)
{
    return !ctype.trimmed().isEmpty() && std::isfinite(crval) && std::isfinite(crpix)
            && std::isfinite(cdelt) && std::abs(cdelt) > 1e-12;
}

bool isCelestialLikeAxis(const QString &ctypeRaw)
{
    const QString ctype = ctypeRaw.trimmed().toUpper();
    return ctype.startsWith(u"RA"_s) || ctype.startsWith(u"DEC"_s) || ctype.startsWith(u"GLON"_s)
            || ctype.startsWith(u"GLAT"_s) || ctype.startsWith(u"ELON"_s)
            || ctype.startsWith(u"ELAT"_s);
}

bool pointInBox(const std::array<int, 2> &anchor, const std::array<int, 2> &current, int x, int y)
{
    const int xmin = std::min(anchor[0], current[0]);
    const int xmax = std::max(anchor[0], current[0]);
    const int ymin = std::min(anchor[1], current[1]);
    const int ymax = std::max(anchor[1], current[1]);
    return x >= xmin && x <= xmax && y >= ymin && y <= ymax;
}

bool pointInCircle(const std::array<int, 2> &anchor, const std::array<int, 2> &current, int x, int y)
{
    const double dx = static_cast<double>(current[0] - anchor[0]);
    const double dy = static_cast<double>(current[1] - anchor[1]);
    const double radius = std::sqrt(dx * dx + dy * dy);
    if (radius <= 0.0) {
        return x == anchor[0] && y == anchor[1];
    }
    const double px = static_cast<double>(x - anchor[0]);
    const double py = static_cast<double>(y - anchor[1]);
    return (px * px + py * py) <= radius * radius;
}

bool pointInPolygon(const std::vector<std::array<int, 2>> &vertices, int x, int y)
{
    if (vertices.size() < 3) {
        return false;
    }

    bool inside = false;
    const double px = static_cast<double>(x) + 0.5;
    const double py = static_cast<double>(y) + 0.5;
    for (std::size_t i = 0, j = vertices.size() - 1; i < vertices.size(); j = i++) {
        const double xi = static_cast<double>(vertices[i][0]) + 0.5;
        const double yi = static_cast<double>(vertices[i][1]) + 0.5;
        const double xj = static_cast<double>(vertices[j][0]) + 0.5;
        const double yj = static_cast<double>(vertices[j][1]) + 0.5;
        const bool intersects = ((yi > py) != (yj > py))
                && (px < (xj - xi) * (py - yi) / ((yj - yi) == 0.0 ? 1e-12 : (yj - yi)) + xi);
        if (intersects) {
            inside = !inside;
        }
    }
    return inside;
}

bool pointInAnnulus(const std::array<int, 2> &center, const std::array<int, 2> &outerPoint,
                    double innerRadius, int x, int y)
{
    const double outerDx = static_cast<double>(outerPoint[0] - center[0]);
    const double outerDy = static_cast<double>(outerPoint[1] - center[1]);
    const double outerRadius = std::sqrt(outerDx * outerDx + outerDy * outerDy);
    if (outerRadius <= 0.0) {
        return false;
    }

    const double px = static_cast<double>(x - center[0]);
    const double py = static_cast<double>(y - center[1]);
    const double radius = std::sqrt(px * px + py * py);
    return radius >= std::max(0.0, innerRadius) && radius <= outerRadius;
}

std::array<int, 4> regionBounds2D(vtkWindowImage::RegionMode mode, const std::array<int, 2> &anchor,
                                  const std::array<int, 2> &current,
                                  const std::vector<std::array<int, 2>> &polygonVertices)
{
    if (mode == vtkWindowImage::RegionMode::Polygon && !polygonVertices.empty()) {
        int xmin = polygonVertices.front()[0];
        int xmax = polygonVertices.front()[0];
        int ymin = polygonVertices.front()[1];
        int ymax = polygonVertices.front()[1];
        for (const auto &vertex : polygonVertices) {
            xmin = std::min(xmin, vertex[0]);
            xmax = std::max(xmax, vertex[0]);
            ymin = std::min(ymin, vertex[1]);
            ymax = std::max(ymax, vertex[1]);
        }
        xmin = std::min(xmin, current[0]);
        xmax = std::max(xmax, current[0]);
        ymin = std::min(ymin, current[1]);
        ymax = std::max(ymax, current[1]);
        return { xmin, xmax, ymin, ymax };
    }

    const int xmin = std::min(anchor[0], current[0]);
    const int xmax = std::max(anchor[0], current[0]);
    const int ymin = std::min(anchor[1], current[1]);
    const int ymax = std::max(anchor[1], current[1]);
    return { xmin, xmax, ymin, ymax };
}

bool pointInRegion(vtkWindowImage::RegionMode mode, const std::array<int, 2> &anchor,
                   const std::array<int, 2> &current,
                   const std::vector<std::array<int, 2>> &polygonVertices, double annulusInnerRadius,
                   int x, int y)
{
    switch (mode) {
    case vtkWindowImage::RegionMode::Box:
        return pointInBox(anchor, current, x, y);
    case vtkWindowImage::RegionMode::Circle:
        return pointInCircle(anchor, current, x, y);
    case vtkWindowImage::RegionMode::Polygon:
        return pointInPolygon(polygonVertices, x, y);
    case vtkWindowImage::RegionMode::Annulus:
        return pointInAnnulus(anchor, current, annulusInnerRadius, x, y);
    case vtkWindowImage::RegionMode::None:
    default:
        return false;
    }
}

QString regionModeLabel(vtkWindowImage::RegionMode mode)
{
    switch (mode) {
    case vtkWindowImage::RegionMode::Box:
        return u"Box"_s;
    case vtkWindowImage::RegionMode::Circle:
        return u"Circle"_s;
    case vtkWindowImage::RegionMode::Polygon:
        return u"Polygon"_s;
    case vtkWindowImage::RegionMode::Annulus:
        return u"Annulus"_s;
    case vtkWindowImage::RegionMode::None:
    default:
        return u"Region"_s;
    }
}

RegionStatistics computeRegionStatistics2D(vtkImageData *imageData, vtkWindowImage::RegionMode mode,
                                           const std::array<int, 2> &anchor,
                                           const std::array<int, 2> &current,
                                           const std::vector<std::array<int, 2>> &polygonVertices,
                                           double annulusInnerRadius)
{
    RegionStatistics stats;
    if (!imageData) {
        return stats;
    }

    int extent[6];
    imageData->GetExtent(extent);
    const auto bounds = regionBounds2D(mode, anchor, current, polygonVertices);
    std::vector<double> values;
    double sum = 0.0;
    double sumSq = 0.0;
    stats.minValue = std::numeric_limits<double>::infinity();
    stats.maxValue = -std::numeric_limits<double>::infinity();

    for (int y = std::max(extent[2], bounds[2]); y <= std::min(extent[3], bounds[3]); ++y) {
        for (int x = std::max(extent[0], bounds[0]); x <= std::min(extent[1], bounds[1]); ++x) {
            const bool inside =
                    pointInRegion(mode, anchor, current, polygonVertices, annulusInnerRadius, x, y);
            if (!inside) {
                continue;
            }

            ++stats.totalCount;
            const double value = imageData->GetScalarComponentAsDouble(x, y, 0, 0);
            if (!std::isfinite(value)) {
                ++stats.blankedCount;
                continue;
            }

            ++stats.validCount;
            stats.minValue = std::min(stats.minValue, value);
            stats.maxValue = std::max(stats.maxValue, value);
            sum += value;
            sumSq += value * value;
            values.push_back(value);
        }
    }

    if (stats.validCount <= 0) {
        return stats;
    }

    stats.mean = sum / static_cast<double>(stats.validCount);
    stats.stddev = std::sqrt(std::max(0.0, sumSq / static_cast<double>(stats.validCount)
                                               - stats.mean * stats.mean));
    std::sort(values.begin(), values.end());
    const int mid = stats.validCount / 2;
    stats.median = (stats.validCount % 2 == 0) ? 0.5 * (values[mid - 1] + values[mid]) : values[mid];
    stats.valid = true;
    return stats;
}

VisibleImageBounds2D computeVisibleImageBounds2D(vtkRenderer *renderer, vtkImageData *imageData)
{
    VisibleImageBounds2D result;
    if (!renderer || !imageData || !renderer->GetActiveCamera()) {
        return result;
    }

    const int *size = renderer->GetSize();
    if (!size || size[0] <= 0 || size[1] <= 0) {
        return result;
    }

    double bounds[6];
    imageData->GetBounds(bounds);
    auto *camera = renderer->GetActiveCamera();
    const double *focalPoint = camera->GetFocalPoint();
    const double halfHeight = camera->GetParallelScale();
    const double halfWidth = halfHeight * static_cast<double>(size[0]) / static_cast<double>(size[1]);

    result.xmin = std::max(bounds[0], focalPoint[0] - halfWidth);
    result.xmax = std::min(bounds[1], focalPoint[0] + halfWidth);
    result.ymin = std::max(bounds[2], focalPoint[1] - halfHeight);
    result.ymax = std::min(bounds[3], focalPoint[1] + halfHeight);
    result.valid = result.xmin <= result.xmax && result.ymin <= result.ymax;
    return result;
}

void configureAxisActor(vtkAxisActor2D *axis, double x1, double y1, double x2, double y2)
{
    axis->GetPoint1Coordinate()->SetCoordinateSystemToDisplay();
    axis->GetPoint2Coordinate()->SetCoordinateSystemToDisplay();
    axis->GetPoint1Coordinate()->SetValue(x1, y1);
    axis->GetPoint2Coordinate()->SetValue(x2, y2);
    axis->SetNumberOfLabels(5);
    axis->AdjustLabelsOn();
    axis->SetLabelFormat("%-#6.4g");
    axis->SetFontFactor(0.6);
    axis->SetTickLength(6);
    axis->AxisVisibilityOn();
    axis->TickVisibilityOn();
    axis->LabelVisibilityOn();
    axis->TitleVisibilityOn();
    axis->GetProperty()->SetColor(1., 1., 1.);
    axis->GetLabelTextProperty()->SetColor(1., 1., 1.);
    axis->GetTitleTextProperty()->SetColor(1., 1., 1.);
    axis->GetLabelTextProperty()->SetFontSize(14);
    axis->GetTitleTextProperty()->SetFontSize(16);
}

void configureVerticalAxisTitle(vtkAxisActor2D *axis)
{
    auto *titleProp = axis->GetTitleTextProperty();
    titleProp->SetOrientation(90.);
    titleProp->SetJustificationToCentered();
    titleProp->SetVerticalJustificationToCentered();
    titleProp->SetColor(1., 1., 1.);
    titleProp->SetFontSize(14);
    axis->SetTitlePosition(0.5);
}

QString upperCtype(const QString &value)
{
    return value.trimmed().toUpper();
}

int inferCelestialFrameFromCtypePair(const std::array<QString, 3> &ctype)
{
    const QString c1 = upperCtype(ctype[0]);
    const QString c2 = upperCtype(ctype[1]);
    if (c1.startsWith(u"GLON"_s) && c2.startsWith(u"GLAT"_s)) {
        return WCS_GALACTIC;
    }
    if (c1.startsWith(u"ELON"_s) && c2.startsWith(u"ELAT"_s)) {
        return WCS_ECLIPTIC;
    }
    if (c1.startsWith(u"RA"_s) && c2.startsWith(u"DEC"_s)) {
        return WCS_J2000;
    }
    return -1;
}

SanityReport buildImageSanityReport(bool isRemoteMode, AstroUtils *astro,
                                    const std::array<QString, 3> &ctype,
                                    const std::array<QString, 3> &cunit,
                                    const std::array<double, 3> &crval,
                                    const std::array<double, 3> &crpix,
                                    const std::array<double, 3> &cdelt,
                                    vtkImageData *imageData)
{
    const auto pairRecognized = [&ctype]() {
        const QString c1 = ctype[0].trimmed().toUpper();
        const QString c2 = ctype[1].trimmed().toUpper();
        return (c1.startsWith(u"GLON"_s) && c2.startsWith(u"GLAT"_s))
                || (c1.startsWith(u"ELON"_s) && c2.startsWith(u"ELAT"_s))
                || (c1.startsWith(u"RA"_s) && c2.startsWith(u"DEC"_s));
    };

    QStringList warnings;
    QStringList unknowns;

    for (int axis = 0; axis < 2; ++axis) {
        if (axisHasAnyMetadata(ctype[axis], cunit[axis], crval[axis], crpix[axis], cdelt[axis])
            && !axisHasLinearWcs(ctype[axis], crval[axis], crpix[axis], cdelt[axis])) {
            warnings << u"Axis %1 has incomplete WCS metadata."_s.arg(axis + 1);
        }
    }

    const bool axis0Celestial = isCelestialLikeAxis(ctype[0]);
    const bool axis1Celestial = isCelestialLikeAxis(ctype[1]);
    if (axis0Celestial != axis1Celestial) {
        warnings << u"Only one spatial axis looks celestial; the coordinate pairing is incomplete."_s;
    } else if (axis0Celestial && !pairRecognized()) {
        warnings << u"Celestial axis pairing is not recognized as FK5, Galactic, or Ecliptic."_s;
    } else if (!axis0Celestial && !axis1Celestial) {
        unknowns << u"Celestial WCS metadata unavailable; overlay may fall back to pixel coordinates."_s;
    }

    if (ctype[0].trimmed().isEmpty() || ctype[1].trimmed().isEmpty()) {
        unknowns << u"CTYPE metadata incomplete for one or more spatial axes."_s;
    }

    const double blankFraction = computeBlankFraction(imageData);
    if (blankFraction >= 0.2) {
        warnings << u"Loaded image is heavily blanked/NaN (%1%)."_s.arg(blankFraction * 100.0, 0, 'f', 1);
    }

    if (isRemoteMode && (!warnings.isEmpty() || !unknowns.isEmpty())) {
        unknowns << u"Remote image still opens, but incomplete metadata lowers WCS confidence."_s;
    }
    if (!isRemoteMode && astro && astro->isSimulation()) {
        unknowns << u"Local dataset has no supported celestial WCS; pixel/index semantics remain valid."_s;
    }

    SanityReport report;
    if (!warnings.isEmpty()) {
        report.level = SanityLevel::Warning;
        report.summary = u"Sanity: Warning (%1)"_s.arg(warnings.size());
    } else if (!unknowns.isEmpty()) {
        report.level = SanityLevel::Unknown;
        report.summary = u"Sanity: Unknown / incomplete"_s;
    } else {
        report.level = SanityLevel::Ok;
        report.summary = u"Sanity: OK"_s;
    }

    QString details;
    if (!warnings.isEmpty()) {
        details += u"Warnings:\n- %1"_s.arg(warnings.join(u"\n- "_s));
    }
    if (!unknowns.isEmpty()) {
        if (!details.isEmpty()) {
            details += u"\n\n"_s;
        }
        details += u"Unknown / incomplete:\n- %1"_s.arg(unknowns.join(u"\n- "_s));
    }
    if (details.isEmpty()) {
        details = u"No common FITS/WCS issues detected in the current image metadata."_s;
    }
    report.details = details;
    return report;
}

QString formatCelestialCoordinate(int frame, int axis, double value)
{
    char buffer[64] = { 0 };
    if (frame == WCS_J2000 && axis == 0) {
        ra2str(buffer, sizeof(buffer), value, 1);
    } else {
        dec2str(buffer, sizeof(buffer), value, axis == 0 ? 1 : 0);
    }
    return QString::fromLatin1(buffer).trimmed();
}

void configureTickLabelActor(vtkTextActor *actor, bool rightAligned)
{
    actor->GetTextProperty()->SetColor(1., 1., 1.);
    actor->GetTextProperty()->SetFontSize(12);
    actor->GetTextProperty()->SetVerticalJustificationToCentered();
    if (rightAligned) {
        actor->GetTextProperty()->SetJustificationToRight();
    } else {
        actor->GetTextProperty()->SetJustificationToCentered();
    }
}

vtkSmartPointer<vtkImageData> createPlaceholderImageData()
{
    vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
    image->SetExtent(0, 0, 0, 0, 0, 0);
    image->SetOrigin(0., 0., 0.);
    image->SetSpacing(1., 1., 1.);
    image->AllocateScalars(VTK_FLOAT, 1);
    image->SetScalarComponentFromFloat(0, 0, 0, 0, 0.f);
    return image;
}

ImageLayerLoadResult createPlaceholderRemoteLayerResult(const QString &filepath)
{
    ImageLayerLoadResult result;
    result.valid = true;
    result.filepath = filepath.toStdString();
    result.imageData = createPlaceholderImageData();
    result.scalarRange = { 0., 0. };
    return result;
}

ImageLayerLoadResult fetchRemoteImageLayer(const QString &backendUrl, const QString &datasetId,
                                           const QString &datasetPath)
{
    ImageLayerLoadResult result;
    result.filepath = datasetPath.toStdString();

    BackendClient client(backendUrl);
    const auto response = client.requestImage(datasetId);
    if (!response.valid) {
        result.errorMessage = response.error.isEmpty() ? "Remote image request failed."
                                                       : response.error.toStdString();
        return result;
    }

    if (response.scalarType != u"float32"_s) {
        result.errorMessage = "Unsupported remote image scalar type.";
        return result;
    }

    const qsizetype expectedBytes =
            static_cast<qsizetype>(response.width) * response.height * static_cast<qsizetype>(sizeof(float));
    if (response.width <= 0 || response.height <= 0 || response.data.size() != expectedBytes) {
        result.errorMessage = "Invalid remote image payload.";
        return result;
    }

    vtkSmartPointer<vtkImageData> image = vtkSmartPointer<vtkImageData>::New();
    image->SetExtent(0, response.width - 1, 0, response.height - 1, 0, 0);
    image->SetOrigin(0., 0., 0.);
    image->SetSpacing(1., 1., 1.);
    image->AllocateScalars(VTK_FLOAT, 1);
    std::memcpy(image->GetScalarPointer(), response.data.constData(),
                static_cast<std::size_t>(expectedBytes));

    const double *range = image->GetScalarRange();
    result.imageData = image;
    result.scalarRange = { range[0], range[1] };
    result.valid = true;
    return result;
}

QString formatImageBoundsSummary(vtkImageData *imageData)
{
    if (!imageData) {
        return u"unavailable"_s;
    }

    double bounds[6];
    imageData->GetBounds(bounds);
    return u"x=%1..%2 y=%3..%4"_s.arg(std::lround(bounds[0]))
            .arg(std::lround(bounds[1]))
            .arg(std::lround(bounds[2]))
            .arg(std::lround(bounds[3]));
}
}

vtkWindowImage::vtkWindowImage(const QString &filepath, QWidget *parent)
    : vtkWindowImage(filepath,
                     {},
                     {},
                     { QString(), QString(), QString() },
                     { QString(), QString(), QString() },
                     { 0.0, 0.0, 0.0 },
                     { 1.0, 1.0, 1.0 },
                     { 1.0, 1.0, 1.0 },
                     parent)
{
}

vtkWindowImage::vtkWindowImage(const QString &filepath, const QString &backendUrl,
                               const QString &datasetId,
                               const std::array<QString, 3> &remoteCtype,
                               const std::array<QString, 3> &remoteCunit,
                               const std::array<double, 3> &remoteCrval,
                               const std::array<double, 3> &remoteCrpix,
                               const std::array<double, 3> &remoteCdelt, QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::vtkWindowImage),
      filepath(filepath),
      isRemoteMode(!datasetId.isEmpty()),
      remoteBackendUrl(backendUrl),
      remoteDatasetId(datasetId),
      remoteDatasetCtype(remoteCtype),
      remoteDatasetCunit(remoteCunit),
      remoteDatasetCrval(remoteCrval),
      remoteDatasetCrpix(remoteCrpix),
      remoteDatasetCdelt(remoteCdelt),
      astro(this->isRemoteMode ? nullptr : std::make_unique<AstroUtils>(filepath.toStdString())),
      lutCustomizer(nullptr),
      profileWidget(nullptr),
      layers(nullptr),
      importService(std::make_unique<ImageLayerImportService>())
{
    ui->setupUi(this);
    if (this->isRemoteMode) {
        qDebug().noquote()
                << QStringLiteral("[wcs] remote metadata loaded ctype=%1,%2 cdelt=%3,%4")
                           .arg(this->remoteDatasetCtype[0], this->remoteDatasetCtype[1])
                           .arg(this->remoteDatasetCdelt[0], 0, 'g', 12)
                           .arg(this->remoteDatasetCdelt[1], 0, 'g', 12);
    }
    this->setWindowTitle(this->isRemoteMode ? u"%1 [remote image]"_s.arg(this->filepath)
                                            : this->filepath);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->wcsAxesCheck = new QCheckBox(u"Show WCS Axes"_s, this);
    this->wcsAxesCheck->setChecked(this->showWcsAxes);
    this->statusBar()->addPermanentWidget(this->wcsAxesCheck);
    this->hoverReadoutLabel = new QLabel(this);
    this->hoverReadoutLabel->setStyleSheet(u"QLabel { padding-left: 8px; }"_s);
    this->hoverReadoutLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    this->statusBar()->addPermanentWidget(this->hoverReadoutLabel, 1);
    this->dataStateLabel = new QLabel(this);
    this->dataStateLabel->setStyleSheet(u"QLabel { padding-left: 8px; color: palette(window-text); }"_s);
    this->dataStateLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    this->statusBar()->addPermanentWidget(this->dataStateLabel);
    this->sanityLabel = new QLabel(this);
    this->sanityLabel->setStyleSheet(u"QLabel { padding-left: 8px; }"_s);
    this->sanityLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    this->statusBar()->addPermanentWidget(this->sanityLabel);
    QObject::connect(this->wcsAxesCheck, &QCheckBox::toggled, this, [this](bool checked) {
        this->showWcsAxes = checked;
        this->lastOverlayVisibleBounds = { std::numeric_limits<double>::quiet_NaN(),
                                           std::numeric_limits<double>::quiet_NaN(),
                                           std::numeric_limits<double>::quiet_NaN(),
                                           std::numeric_limits<double>::quiet_NaN() };
        this->lastOverlayViewportSize = { -1, -1 };
        this->setWcsOverlayVisible(checked);
        this->requestWcsOverlayRender();
    });
    this->statusMessageClearTimer.setSingleShot(true);
    QObject::connect(&this->statusMessageClearTimer, &QTimer::timeout, this, [this]() {
        this->persistentStatusActive = false;
        this->statusBar()->clearMessage();
    });
    QObject::connect(&this->layerLoadWatcher, &QFutureWatcher<ImageLayerLoadResult>::finished, this,
                     [this]() {
                         this->setLayerImportEnabled(true);

                         const auto result = this->layerLoadWatcher.result();
                         if (!result.valid) {
                             if (!result.errorMessage.empty()) {
                                 this->persistentStatusActive = false;
                                 this->statusMessageClearTimer.stop();
                                 QMessageBox::warning(this, u"Import FITS file"_s,
                                                      QString::fromStdString(result.errorMessage));
                             }
                             this->clearPersistentStatusMessage();
                             return;
                         }

                         QElapsedTimer applyTimer;
                         applyTimer.start();
                         this->applyLoadedLayer(result);
                         qDebug().noquote()
                                 << QStringLiteral("[perf][layer] UI apply: %1 ms")
                                            .arg(applyTimer.elapsed());
                         this->clearPersistentStatusMessage();
                     });
    QObject::connect(&this->remoteImageWatcher, &QFutureWatcher<ImageLayerLoadResult>::finished, this,
                     [this]() {
                         const auto result = this->remoteImageWatcher.result();
                         if (!result.valid || !result.imageData) {
                             this->persistentStatusActive = false;
                             this->statusMessageClearTimer.stop();
                             this->statusBar()->showMessage(result.errorMessage.empty()
                                                                    ? u"Could not load remote image."_s
                                                                    : QString::fromStdString(result.errorMessage));
                             return;
                         }

                         this->applyRemoteMasterLayer(result);
                         this->clearPersistentStatusMessage();
                     });

    this->layers = this->isRemoteMode
            ? new LayerListModel(createPlaceholderRemoteLayerResult(this->filepath), this)
            : new LayerListModel(this->filepath.toStdString(), this);

    this->setupRenderer();
    this->updateDataStatePanel();
    this->updateSanityPanel();

    // Setup Menu File
    QObject::connect(ui->actionAddFITS, &QAction::triggered, this, &vtkWindowImage::addLocalFile);

    // Setup Menu WCS
    auto groupWCS = new QActionGroup(this);
    groupWCS->addAction(ui->actionGalactic);
    groupWCS->addAction(ui->actionFK5);
    groupWCS->addAction(ui->actionEcliptic);
    QObject::connect(groupWCS, &QActionGroup::triggered, this, &vtkWindowImage::changeLegendWCS);
    ui->menuWCS->addSeparator();
    auto *formatGroup = new QActionGroup(this);
    this->actionWcsSexagesimal = ui->menuWCS->addAction(u"Sexagesimal"_s);
    this->actionWcsDecimal = ui->menuWCS->addAction(u"Decimal"_s);
    this->actionWcsSexagesimal->setCheckable(true);
    this->actionWcsDecimal->setCheckable(true);
    formatGroup->addAction(this->actionWcsSexagesimal);
    formatGroup->addAction(this->actionWcsDecimal);
    QObject::connect(formatGroup, &QActionGroup::triggered, this, [this](QAction *action) {
        this->wcsFormatExplicitlyChosen = true;
        this->useSexagesimalWcsFormat = action == this->actionWcsSexagesimal;
        this->invalidateWcsOverlayCache();
        this->requestWcsOverlayRender();
    });
    this->applyDefaultWcsFormatForSelectedFrame();

    // Setup Menu Tools
    ui->actionProfile->setCheckable(true);
    QObject::connect(ui->actionProfile, &QAction::toggled, this, &vtkWindowImage::setProbeModeActive);
    this->actionExtractSpectrum = ui->menuTools->addAction(u"Extract Spectrum"_s);
    this->actionExtractSpectrum->setEnabled(false);
    this->actionExtractSpectrum->setToolTip(
            u"Spectrum extraction is only available for cube views."_s);
    this->actionExtractSpectrum->setStatusTip(
            u"Spectrum extraction is only available for cube views."_s);
    ui->menuTools->addSeparator();
    this->actionBoxRegion = ui->menuTools->addAction(u"Box Region Stats"_s);
    this->actionCircleRegion = ui->menuTools->addAction(u"Circle Region Stats"_s);
    this->actionPolygonRegion = ui->menuTools->addAction(u"Polygon Region Analysis"_s);
    this->actionAnnulusRegion = ui->menuTools->addAction(u"Annulus Region Analysis"_s);
    this->actionBoxRegion->setCheckable(true);
    this->actionCircleRegion->setCheckable(true);
    this->actionPolygonRegion->setCheckable(true);
    this->actionAnnulusRegion->setCheckable(true);
    auto *regionGroup = new QActionGroup(this);
    regionGroup->setExclusive(false);
    regionGroup->addAction(this->actionBoxRegion);
    regionGroup->addAction(this->actionCircleRegion);
    regionGroup->addAction(this->actionPolygonRegion);
    regionGroup->addAction(this->actionAnnulusRegion);
    QObject::connect(this->actionBoxRegion, &QAction::toggled, this,
                     [this](bool checked) { this->setRegionMode(RegionMode::Box, checked); });
    QObject::connect(this->actionCircleRegion, &QAction::toggled, this,
                     [this](bool checked) { this->setRegionMode(RegionMode::Circle, checked); });
    QObject::connect(this->actionPolygonRegion, &QAction::toggled, this,
                     [this](bool checked) { this->setRegionMode(RegionMode::Polygon, checked); });
    QObject::connect(this->actionAnnulusRegion, &QAction::toggled, this,
                     [this](bool checked) { this->setRegionMode(RegionMode::Annulus, checked); });
    ui->menuTools->addSeparator();
    this->actionLoadCatalogueOverlay = ui->menuTools->addAction(u"Load Catalogue Overlay"_s);
    this->actionShowCatalogueOverlay = ui->menuTools->addAction(u"Show Catalogue Overlay"_s);
    this->actionClearCatalogueOverlay = ui->menuTools->addAction(u"Clear Catalogue Overlay"_s);
    this->actionShowCatalogueOverlay->setCheckable(true);
    this->actionShowCatalogueOverlay->setEnabled(false);
    this->actionClearCatalogueOverlay->setEnabled(false);
    QObject::connect(this->actionLoadCatalogueOverlay, &QAction::triggered, this,
                     &vtkWindowImage::loadCatalogueOverlay);
    QObject::connect(this->actionShowCatalogueOverlay, &QAction::toggled, this,
                     &vtkWindowImage::setCatalogueOverlayVisible);
    QObject::connect(this->actionClearCatalogueOverlay, &QAction::triggered, this,
                     &vtkWindowImage::clearCatalogueOverlay);

    // Setup Buttons
    ui->btnSources->setIcon(QIcon(u":/icons/RECT_SELECT.png"_s));
    ui->btnFilaments->setIcon(QIcon(u":/icons/RECT_SELECT.png"_s));
    ui->btn3D->setIcon(QIcon(u":/icons/RECT_SELECT.png"_s));

    // Color Maps Combobox
    auto cmaps = ColorMaps::GetColorMapNames();
    std::for_each(cmaps.cbegin(), cmaps.cend(), [this](const std::string &name) {
        ui->comboLut->addItem(QString::fromStdString(name));
    });
    ui->comboLut->setCurrentText(QString::fromStdString(ColorMaps::DefaultColorMap));
    QObject::connect(ui->comboLut, &QComboBox::textActivated, this,
                     &vtkWindowImage::changeCurrentColorMap);

    // LUT Customizer
    QObject::connect(ui->btnLutEdit, &QPushButton::clicked, this,
                     &vtkWindowImage::showLUTCustomizer);

    // Scale Radio Buttons
    auto group = new QButtonGroup(this);
    group->addButton(ui->radioLinear);
    group->addButton(ui->radioLog);
    QObject::connect(group, &QButtonGroup::buttonClicked, this,
                     &vtkWindowImage::changeCurrentColorScale);

    // Layer Opacity
    QObject::connect(ui->sliderOpacity, &QSlider::actionTriggered, this,
                     &vtkWindowImage::changeCurrentLayerOpacity);

    // Setup Layer List View
    ui->listLayer->setAcceptDrops(true);
    ui->listLayer->setModel(this->layers);
    ui->listLayer->setCurrentIndex(this->layers->index(0, 0));
    QObject::connect(this->layers, &LayerListModel::dataChanged, this, &vtkWindowImage::vtkRender);
    QObject::connect(ui->listLayer->selectionModel(), &QItemSelectionModel::currentChanged, this,
                     &vtkWindowImage::showCurrentLayerSettings);
    QObject::connect(ui->listLayer->selectionModel(), &QItemSelectionModel::currentChanged, this,
                     &vtkWindowImage::updateLUTCustomizer);

    if (this->isRemoteMode) {
        ui->actionAddFITS->setEnabled(false);
        this->showPersistentStatusMessage(u"Loading remote image..."_s);
        this->remoteImageWatcher.setFuture(
                QtConcurrent::run(&fetchRemoteImageLayer, this->remoteBackendUrl,
                                  this->remoteDatasetId, this->filepath));
    }
}

vtkWindowImage::~vtkWindowImage()
{
    delete ui;
}

void vtkWindowImage::closeEvent(QCloseEvent *event)
{
    if (this->isBusy()) {
        this->showPersistentStatusMessage(this->isRemoteMode ? u"Loading remote image..."_s
                                                             : u"Loading layer..."_s);
        event->ignore();
        return;
    }

    QMainWindow::closeEvent(event);
}

void vtkWindowImage::showLUTCustomizer()
{
    const int index = this->currentLayerIndex();
    if (!this->lutCustomizer) {
        this->lutCustomizer = new LUTCustomizerDialog(this);
        QObject::connect(this->lutCustomizer, &LUTCustomizerDialog::lutUpdated, this,
                         &vtkWindowImage::showCurrentLayerSettings);
    }
    this->lutCustomizer->init(this->layers->getImageData(index),
                              this->layers->getLookupTable(index));
    this->lutCustomizer->show();
    this->lutCustomizer->raise();
    this->lutCustomizer->activateWindow();
}

void vtkWindowImage::updateLUTCustomizer()
{
    if (this->lutCustomizer) {
        const int index = this->currentLayerIndex();
        this->lutCustomizer->init(this->layers->getImageData(index),
                                  this->layers->getLookupTable(index));
    }
}

void vtkWindowImage::addLocalFile()
{
    if (this->isBusy()) {
        return;
    }

    const QString filepath = QFileDialog::getOpenFileName(this, u"Import FITS file"_s, QString(),
                                                          u"FITS files (*.fits *.fit)"_s);

    if (filepath.isEmpty()) {
        return;
    }

    const ImageLayerImportResult result =
            this->importService->inspect(ImageLayerImportRequest { this->filepath, filepath });
    if (!result.accepted) {
        QMessageBox::warning(this, u"Import FITS file"_s, result.errorMessage);
        return;
    }

    this->addLayerImage(filepath.toStdString());
}

void vtkWindowImage::setupRenderer()
{
    this->wcsOverlayInitialized = false;
    vtkNew<vtkRenderer> ren;
    ren->SetBackground(0.21, 0.23, 0.25);
    ren->GetActiveCamera()->ParallelProjectionOn();

    vtkNew<vtkGenericOpenGLRenderWindow> win;
    win->AddRenderer(ren);
    ui->vtk->setRenderWindow(win);
    ui->vtk->setEnableTouchEventProcessing(false);

    vtkNew<vtkInteractorStyleImage> style;
    win->GetInteractor()->SetInteractorStyle(style);
    win->GetInteractor()->AddObserver(vtkCommand::MouseMoveEvent, this,
                                      &vtkWindowImage::mouseCallback);
    win->GetInteractor()->AddObserver(vtkCommand::LeftButtonPressEvent, this,
                                      &vtkWindowImage::toggleProbeFreeze);
    win->GetInteractor()->AddObserver(vtkCommand::LeftButtonReleaseEvent, this,
                                      &vtkWindowImage::finishRegionInteraction);
    win->GetInteractor()->AddObserver(vtkCommand::RightButtonPressEvent, this,
                                      &vtkWindowImage::finishRegionInteraction);
    win->GetInteractor()->AddObserver(vtkCommand::LeftButtonDoubleClickEvent, this,
                                      &vtkWindowImage::finishRegionInteraction);

    this->coordinate->SetCoordinateSystemToDisplay();
    this->coordinate->SetViewport(ren);

    // Stack
    this->stack->AddImage(this->layers->getMasterLayerActor());
    this->stack->SetActiveLayer(0);
    ren->AddViewProp(this->stack);

    // Color bar
    this->colorbar->SetMaximumWidthInPixels(120);
    this->colorbar->SetPosition(0.9, 0.1);
    this->colorbar->SetLookupTable(this->layers->getLookupTable(this->layers->getMasterIndex()));
    ren->AddViewProp(this->colorbar);

    this->layerController =
            std::make_unique<ImageLayerController>(*(this->layers), this->stack, this->colorbar);

    // Legend
    if (this->astro) {
        this->legendWCS->Init(this->filepath.toStdString());
        this->legendWCS->SetWCS(WCS_GALACTIC);
        ren->AddViewProp(this->legendWCS);
    }
    ren->AddViewProp(this->overlayXAxis);
    ren->AddViewProp(this->overlayYAxis);
    ren->AddViewProp(this->overlayXTitleActor);
    ren->AddViewProp(this->overlayYTitleActor);
    vtkNew<vtkPolyDataMapper> probeHorizontalMapper;
    probeHorizontalMapper->SetInputConnection(this->probeHorizontalLine->GetOutputPort());
    this->probeHorizontalActor->SetMapper(probeHorizontalMapper);
    this->probeHorizontalActor->GetProperty()->SetColor(1.0, 0.85, 0.1);
    this->probeHorizontalActor->GetProperty()->SetLineWidth(1.5);
    this->probeHorizontalActor->VisibilityOff();
    ren->AddActor(this->probeHorizontalActor);
    vtkNew<vtkPolyDataMapper> probeVerticalMapper;
    probeVerticalMapper->SetInputConnection(this->probeVerticalLine->GetOutputPort());
    this->probeVerticalActor->SetMapper(probeVerticalMapper);
    this->probeVerticalActor->GetProperty()->SetColor(1.0, 0.85, 0.1);
    this->probeVerticalActor->GetProperty()->SetLineWidth(1.5);
    this->probeVerticalActor->VisibilityOff();
    ren->AddActor(this->probeVerticalActor);
    vtkRenderer *renderer = ren;
    const auto configureRegionLineActor = [renderer](vtkLineSource *line, vtkActor *actor) {
        vtkNew<vtkPolyDataMapper> mapper;
        mapper->SetInputConnection(line->GetOutputPort());
        actor->SetMapper(mapper);
        actor->GetProperty()->SetColor(0.2, 0.9, 1.0);
        actor->GetProperty()->SetLineWidth(2.0);
        actor->VisibilityOff();
        renderer->AddActor(actor);
    };
    configureRegionLineActor(this->regionTopLine, this->regionTopActor);
    configureRegionLineActor(this->regionBottomLine, this->regionBottomActor);
    configureRegionLineActor(this->regionLeftLine, this->regionLeftActor);
    configureRegionLineActor(this->regionRightLine, this->regionRightActor);
    this->regionCircleSource->SetNumberOfSides(96);
    this->regionCircleSource->GeneratePolygonOff();
    vtkNew<vtkPolyDataMapper> regionCircleMapper;
    regionCircleMapper->SetInputConnection(this->regionCircleSource->GetOutputPort());
    this->regionCircleActor->SetMapper(regionCircleMapper);
    this->regionCircleActor->GetProperty()->SetColor(0.2, 0.9, 1.0);
    this->regionCircleActor->GetProperty()->SetLineWidth(2.0);
    this->regionCircleActor->VisibilityOff();
    ren->AddActor(this->regionCircleActor);
    this->regionAnnulusOuterSource->SetNumberOfSides(96);
    this->regionAnnulusOuterSource->GeneratePolygonOff();
    vtkNew<vtkPolyDataMapper> regionAnnulusOuterMapper;
    regionAnnulusOuterMapper->SetInputConnection(this->regionAnnulusOuterSource->GetOutputPort());
    this->regionAnnulusOuterActor->SetMapper(regionAnnulusOuterMapper);
    this->regionAnnulusOuterActor->GetProperty()->SetColor(0.2, 0.9, 1.0);
    this->regionAnnulusOuterActor->GetProperty()->SetLineWidth(2.0);
    this->regionAnnulusOuterActor->GetProperty()->SetRepresentationToWireframe();
    this->regionAnnulusOuterActor->VisibilityOff();
    ren->AddActor(this->regionAnnulusOuterActor);
    this->regionAnnulusInnerSource->SetNumberOfSides(96);
    this->regionAnnulusInnerSource->GeneratePolygonOff();
    vtkNew<vtkPolyDataMapper> regionAnnulusInnerMapper;
    regionAnnulusInnerMapper->SetInputConnection(this->regionAnnulusInnerSource->GetOutputPort());
    this->regionAnnulusInnerActor->SetMapper(regionAnnulusInnerMapper);
    this->regionAnnulusInnerActor->GetProperty()->SetColor(0.2, 0.9, 1.0);
    this->regionAnnulusInnerActor->GetProperty()->SetLineWidth(1.5);
    this->regionAnnulusInnerActor->GetProperty()->SetRepresentationToWireframe();
    this->regionAnnulusInnerActor->VisibilityOff();
    ren->AddActor(this->regionAnnulusInnerActor);
    vtkNew<vtkPolyDataMapper> regionAnnulusFillMapper;
    regionAnnulusFillMapper->SetInputData(this->regionAnnulusFillData);
    this->regionAnnulusFillActor->SetMapper(regionAnnulusFillMapper);
    this->regionAnnulusFillActor->GetProperty()->SetColor(0.2, 0.9, 1.0);
    this->regionAnnulusFillActor->GetProperty()->SetOpacity(0.18);
    this->regionAnnulusFillActor->GetProperty()->SetRepresentationToSurface();
    this->regionAnnulusFillActor->VisibilityOff();
    ren->AddActor(this->regionAnnulusFillActor);
    this->regionPolygonTriangulator->SetInputData(this->regionPolygonFillData);
    vtkNew<vtkPolyDataMapper> regionPolygonMapper;
    regionPolygonMapper->SetInputData(this->regionPolygonData);
    this->regionPolygonActor->SetMapper(regionPolygonMapper);
    this->regionPolygonActor->GetProperty()->SetColor(0.2, 0.9, 1.0);
    this->regionPolygonActor->GetProperty()->SetLineWidth(2.0);
    this->regionPolygonActor->VisibilityOff();
    ren->AddActor(this->regionPolygonActor);
    vtkNew<vtkPolyDataMapper> regionPolygonFillMapper;
    regionPolygonFillMapper->SetInputConnection(this->regionPolygonTriangulator->GetOutputPort());
    this->regionPolygonFillActor->SetMapper(regionPolygonFillMapper);
    this->regionPolygonFillActor->GetProperty()->SetColor(0.2, 0.9, 1.0);
    this->regionPolygonFillActor->GetProperty()->SetOpacity(0.18);
    this->regionPolygonFillActor->VisibilityOff();
    ren->AddActor(this->regionPolygonFillActor);
    this->catalogueOverlayData->SetPoints(this->catalogueOverlayPoints);
    this->catalogueOverlayData->SetLines(this->catalogueOverlayCells);
    vtkNew<vtkPolyDataMapper> catalogueOverlayMapper;
    catalogueOverlayMapper->SetInputData(this->catalogueOverlayData);
    this->catalogueOverlayActor->SetMapper(catalogueOverlayMapper);
    this->catalogueOverlayActor->GetProperty()->SetColor(1.0, 0.45, 0.15);
    this->catalogueOverlayActor->GetProperty()->SetLineWidth(1.5);
    this->catalogueOverlayActor->VisibilityOff();
    ren->AddActor(this->catalogueOverlayActor);
    this->overlayXAxis->GetTitleTextProperty()->SetFontSize(16);
    this->overlayXAxis->GetTitleTextProperty()->SetBold(false);
    this->overlayYAxis->GetTitleTextProperty()->SetFontSize(16);
    this->overlayYAxis->GetTitleTextProperty()->SetOrientation(90.);
    this->overlayYAxis->GetTitleTextProperty()->SetBold(false);
    this->overlayXTitleActor->GetTextProperty()->SetColor(1., 1., 1.);
    this->overlayXTitleActor->GetTextProperty()->SetFontSize(14);
    this->overlayXTitleActor->GetTextProperty()->SetJustificationToCentered();
    this->overlayXTitleActor->GetTextProperty()->SetVerticalJustificationToCentered();
    this->overlayYTitleActor->GetTextProperty()->SetColor(1., 1., 1.);
    this->overlayYTitleActor->GetTextProperty()->SetFontSize(14);
    this->overlayYTitleActor->GetTextProperty()->SetOrientation(90.);
    this->overlayYTitleActor->GetTextProperty()->SetJustificationToCentered();
    this->overlayYTitleActor->GetTextProperty()->SetVerticalJustificationToCentered();
    this->ensureOverlayTickActors(ren);
    this->wcsOverlayInitialized = true;
    this->setWcsOverlayVisible(this->showWcsAxes);
    win->AddObserver(vtkCommand::EndEvent, this, &vtkWindowImage::updateWcsOverlay);
    win->AddObserver(vtkCommand::EndEvent, this, &vtkWindowImage::updateCatalogueOverlayLabels);

    ren->ResetCamera();
    win->Render();
}

void vtkWindowImage::mouseCallback()
{
    if (this->isBusy()) {
        return;
    }

    if (this->probeModeActive && this->probeFrozen) {
        return;
    }

    const int *position = ui->vtk->renderWindow()->GetInteractor()->GetEventPosition();
    if (!position) {
        return;
    }
    this->updateProbeFromDisplayPosition(position[0], position[1]);
    if (this->regionMode != RegionMode::None && this->regionDragging) {
        if (this->updateRegionFromDisplayPosition(position[0], position[1])) {
            ui->vtk->renderWindow()->Render();
        }
    }
}

void vtkWindowImage::toggleProbeFreeze()
{
    if (this->isBusy()) {
        return;
    }

    const int *position = ui->vtk->renderWindow()->GetInteractor()->GetEventPosition();
    if (!position) {
        return;
    }

    if (this->regionMode != RegionMode::None) {
        if (this->updateRegionFromDisplayPosition(position[0], position[1])) {
            if (this->regionMode == RegionMode::Polygon) {
                if (!this->regionDragging) {
                    this->regionPolygonVertices.clear();
                    this->regionPolygonVertices.push_back(this->regionCurrentVoxel);
                    this->regionDragging = true;
                    this->ignoreNextPolygonRelease = true;
                } else if (this->regionPolygonVertices.size() >= 3
                           && distance2d(this->regionCurrentVoxel, this->regionPolygonVertices.front())
                                   <= polygonClosureTolerance) {
                    this->regionCurrentVoxel = this->regionPolygonVertices.front();
                    this->ignoreNextPolygonRelease = false;
                    if (this->finalizePolygonRegion()) {
                        ui->vtk->renderWindow()->Render();
                    }
                    return;
                } else if (this->regionPolygonVertices.empty()
                           || this->regionPolygonVertices.back() != this->regionCurrentVoxel) {
                    this->regionPolygonVertices.push_back(this->regionCurrentVoxel);
                    this->ignoreNextPolygonRelease = true;
                }
                this->regionValid = !this->regionPolygonVertices.empty();
            } else {
                this->regionAnchorVoxel = this->regionCurrentVoxel;
                this->regionDragging = true;
                this->regionValid = true;
            }
            this->refreshRegionOverlay();
            ui->vtk->renderWindow()->Render();
        }
        return;
    }

    if (!this->probeModeActive) {
        return;
    }

    if (!this->probeFrozen) {
        if (this->updateProbeFromDisplayPosition(position[0], position[1])) {
            this->probeFrozen = true;
        }
    } else {
        this->probeFrozen = false;
        this->updateProbeFromDisplayPosition(position[0], position[1]);
    }
}

void vtkWindowImage::finishRegionInteraction()
{
    if (this->isBusy() || this->regionMode == RegionMode::None || !this->regionDragging) {
        return;
    }

    if (this->regionMode == RegionMode::Polygon) {
        if (this->ignoreNextPolygonRelease) {
            this->ignoreNextPolygonRelease = false;
            return;
        }
        if (this->finalizePolygonRegion()) {
            ui->vtk->renderWindow()->Render();
        }
        return;
    }

    const int *position = ui->vtk->renderWindow()->GetInteractor()->GetEventPosition();
    if (position) {
        this->updateRegionFromDisplayPosition(position[0], position[1]);
    }
    if (this->regionMode == RegionMode::Annulus) {
        const double dx = static_cast<double>(this->regionCurrentVoxel[0] - this->regionAnchorVoxel[0]);
        const double dy = static_cast<double>(this->regionCurrentVoxel[1] - this->regionAnchorVoxel[1]);
        const double outerRadius = std::sqrt(dx * dx + dy * dy);
        bool accepted = false;
        const double innerRadius = QInputDialog::getDouble(
                this, u"Annulus Inner Radius"_s, u"Inner radius (pixels):"_s, 0.0, 0.0,
                std::max(0.0, outerRadius), 1, &accepted);
        if (!accepted) {
            if (this->actionAnnulusRegion && this->actionAnnulusRegion->isChecked()) {
                this->actionAnnulusRegion->setChecked(false);
            }
            return;
        }
        if (innerRadius < 0.0 || innerRadius >= outerRadius) {
            this->statusBar()->showMessage(
                    u"Inner radius must be >= 0 and smaller than the outer radius."_s, 3000);
            this->regionAnnulusInnerRadius = 0.0;
            this->regionValid = true;
            this->refreshRegionOverlay();
            ui->vtk->renderWindow()->Render();
            return;
        }
        this->regionAnnulusInnerRadius = innerRadius;
        this->regionDragging = false;
        this->regionValid = true;
        this->refreshRegionOverlay();
        ui->vtk->renderWindow()->Render();
    }
    this->regionDragging = false;
    this->regionValid = true;
    this->refreshRegionOverlay();
    this->analyzeCurrentRegion();
    ui->vtk->renderWindow()->Render();
    if (this->regionMode == RegionMode::Box && this->actionBoxRegion && this->actionBoxRegion->isChecked()) {
        this->actionBoxRegion->setChecked(false);
    } else if (this->regionMode == RegionMode::Circle && this->actionCircleRegion
               && this->actionCircleRegion->isChecked()) {
        this->actionCircleRegion->setChecked(false);
    } else if (this->regionMode == RegionMode::Annulus && this->actionAnnulusRegion
               && this->actionAnnulusRegion->isChecked()) {
        this->actionAnnulusRegion->setChecked(false);
    } else if (this->regionMode == RegionMode::Polygon && this->actionPolygonRegion
               && this->actionPolygonRegion->isChecked()) {
        this->actionPolygonRegion->setChecked(false);
    }
}

bool vtkWindowImage::updateProbeFromDisplayPosition(int displayX, int displayY)
{
    auto *renderer = ui->vtk->renderWindow()->GetRenderers()->GetFirstRenderer();
    auto *imageData = this->layers ? this->layers->getImageData(this->layers->getMasterIndex()) : nullptr;
    if (!renderer || !imageData) {
        this->clearProbe();
        return false;
    }

    this->coordinate->SetValue(displayX, displayY);
    const double *worldCoord = this->coordinate->GetComputedWorldValue(renderer);
    if (!worldCoord) {
        this->clearProbe();
        return false;
    }

    int extent[6];
    imageData->GetExtent(extent);
    double origin[3];
    double spacing[3];
    imageData->GetOrigin(origin);
    imageData->GetSpacing(spacing);

    const int voxelX = std::lround(extent[0] + (worldCoord[0] - origin[0]) / spacing[0]);
    const int voxelY = std::lround(extent[2] + (worldCoord[1] - origin[1]) / spacing[1]);
    if (voxelX < extent[0] || voxelX > extent[1] || voxelY < extent[2] || voxelY > extent[3]) {
        if (!this->probeFrozen) {
            this->clearProbe();
        }
        return false;
    }

    if (this->probeValid && this->probeVoxel[0] == voxelX && this->probeVoxel[1] == voxelY) {
        return true;
    }

    this->probeValid = true;
    this->probeVoxel = { voxelX, voxelY };

    const float value = this->layers->getPixelValue(this->layers->getMasterIndex(), voxelX, voxelY);
    QString valueText = std::isfinite(value) ? QString::number(value, 'g', 8) : u"NaN"_s;
    QString message = u"X=%1  Y=%2  Value=%3"_s.arg(voxelX).arg(voxelY).arg(valueText);
    if (this->astro && !this->astro->isSimulation()) {
        message += u"  %1=%2  %3=%4"_s.arg(this->selectedFrameAxisTitle(0),
                                           this->formatLocalProbeCoordinate(0, this->probeVoxel),
                                           this->selectedFrameAxisTitle(1),
                                           this->formatLocalProbeCoordinate(1, this->probeVoxel));
    } else if (this->isRemoteMode) {
        QString axis0 = this->remoteFormatAxisCoordinate(0, voxelX);
        QString axis1 = this->remoteFormatAxisCoordinate(1, voxelY);
        if (this->remoteHasCelestialAxes()) {
            bool ok0 = false;
            bool ok1 = false;
            const double nativeX = this->remoteVoxelToWcs(0, voxelX, &ok0);
            const double nativeY = this->remoteVoxelToWcs(1, voxelY, &ok1);
            double frameX = nativeX;
            double frameY = nativeY;
            if (ok0 && ok1 && this->convertRemoteCelestialCoordinates(nativeX, nativeY, frameX, frameY)) {
                axis0 = this->formatRemoteOverlayCoordinate(0, frameX);
                axis1 = this->formatRemoteOverlayCoordinate(1, frameY);
            }
        }
        message += u"  %1=%2  %3=%4"_s.arg(this->selectedFrameAxisTitle(0),
                                           axis0,
                                           this->selectedFrameAxisTitle(1),
                                           axis1);
    }
    if (this->probeFrozen) {
        message += u"  [Frozen]"_s;
    }
    if (this->hoverReadoutLabel) {
        this->hoverReadoutLabel->setText(message);
        this->hoverReadoutLabel->setToolTip(message);
    }
    if (this->probeModeActive) {
        this->refreshProbeOverlay();
        this->updateProbeProfile();
        ui->vtk->renderWindow()->Render();
    }
    return true;
}

void vtkWindowImage::refreshProbeOverlay()
{
    auto *imageData = this->layers ? this->layers->getImageData(this->layers->getMasterIndex()) : nullptr;
    if (!imageData || !this->probeValid || !this->probeModeActive) {
        this->probeHorizontalActor->VisibilityOff();
        this->probeVerticalActor->VisibilityOff();
        return;
    }

    double bounds[6];
    imageData->GetBounds(bounds);
    this->probeHorizontalLine->SetPoint1(bounds[0], static_cast<double>(this->probeVoxel[1]), 0.0);
    this->probeHorizontalLine->SetPoint2(bounds[1], static_cast<double>(this->probeVoxel[1]), 0.0);
    this->probeVerticalLine->SetPoint1(static_cast<double>(this->probeVoxel[0]), bounds[2], 0.0);
    this->probeVerticalLine->SetPoint2(static_cast<double>(this->probeVoxel[0]), bounds[3], 0.0);
    this->probeHorizontalActor->VisibilityOn();
    this->probeVerticalActor->VisibilityOn();
}

void vtkWindowImage::clearProbe()
{
    this->probeValid = false;
    if (this->hoverReadoutLabel) {
        this->hoverReadoutLabel->clear();
    }
    this->probeHorizontalActor->VisibilityOff();
    this->probeVerticalActor->VisibilityOff();
}

void vtkWindowImage::setRegionMode(RegionMode mode, bool active)
{
    if (!active) {
        if (this->regionMode == mode) {
            this->regionMode = RegionMode::None;
            this->regionDragging = false;
            this->clearRegion();
            this->setInteractorStyleImage();
            ui->vtk->setCursor(this->probeModeActive ? Qt::CrossCursor : Qt::ArrowCursor);
            if (ui->vtk->renderWindow()) {
                ui->vtk->renderWindow()->Render();
            }
        }
        return;
    }

    if (this->probeModeActive && ui->actionProfile->isChecked()) {
        ui->actionProfile->setChecked(false);
    }
    if (mode == RegionMode::Box && this->actionCircleRegion && this->actionCircleRegion->isChecked()) {
        this->actionCircleRegion->setChecked(false);
    }
    if (mode == RegionMode::Circle && this->actionBoxRegion && this->actionBoxRegion->isChecked()) {
        this->actionBoxRegion->setChecked(false);
    }
    if (mode != RegionMode::Polygon && this->actionPolygonRegion && this->actionPolygonRegion->isChecked()) {
        this->actionPolygonRegion->setChecked(false);
    }
    if (mode != RegionMode::Annulus && this->actionAnnulusRegion && this->actionAnnulusRegion->isChecked()) {
        this->actionAnnulusRegion->setChecked(false);
    }
    this->regionMode = mode;
    this->regionDragging = false;
    this->regionValid = false;
    this->ignoreNextPolygonRelease = false;
    this->regionPolygonVertices.clear();
    this->regionAnnulusInnerRadius = 0.0;
    this->clearRegion();
    this->setInteractorStyleRegion();
    ui->vtk->setCursor(Qt::CrossCursor);
}

bool vtkWindowImage::updateRegionFromDisplayPosition(int displayX, int displayY)
{
    auto *renderer = ui->vtk->renderWindow() ? ui->vtk->renderWindow()->GetRenderers()->GetFirstRenderer()
                                             : nullptr;
    auto *imageData = this->layers ? this->layers->getImageData(this->layers->getMasterIndex()) : nullptr;
    if (!renderer || !imageData) {
        return false;
    }

    this->coordinate->SetValue(displayX, displayY);
    const double *worldCoord = this->coordinate->GetComputedWorldValue(renderer);
    if (!worldCoord) {
        return false;
    }

    int extent[6];
    imageData->GetExtent(extent);
    double origin[3];
    double spacing[3];
    imageData->GetOrigin(origin);
    imageData->GetSpacing(spacing);
    const int voxelX = std::lround(extent[0] + (worldCoord[0] - origin[0]) / spacing[0]);
    const int voxelY = std::lround(extent[2] + (worldCoord[1] - origin[1]) / spacing[1]);
    if (voxelX < extent[0] || voxelX > extent[1] || voxelY < extent[2] || voxelY > extent[3]) {
        return false;
    }

    this->regionCurrentVoxel = { voxelX, voxelY };
    this->refreshRegionOverlay();
    return true;
}

void vtkWindowImage::refreshRegionOverlay()
{
    auto *imageData = this->layers ? this->layers->getImageData(this->layers->getMasterIndex()) : nullptr;
    if (!imageData || this->regionMode == RegionMode::None || !this->regionValid) {
        this->clearRegion();
        return;
    }

    const int xmin = std::min(this->regionAnchorVoxel[0], this->regionCurrentVoxel[0]);
    const int xmax = std::max(this->regionAnchorVoxel[0], this->regionCurrentVoxel[0]);
    const int ymin = std::min(this->regionAnchorVoxel[1], this->regionCurrentVoxel[1]);
    const int ymax = std::max(this->regionAnchorVoxel[1], this->regionCurrentVoxel[1]);

    const bool showBox = this->regionMode == RegionMode::Box;
    const bool showCircle = this->regionMode == RegionMode::Circle;
    const bool showAnnulus = this->regionMode == RegionMode::Annulus;
    const bool showPolygon = this->regionMode == RegionMode::Polygon;
    this->regionTopActor->SetVisibility(showBox);
    this->regionBottomActor->SetVisibility(showBox);
    this->regionLeftActor->SetVisibility(showBox);
    this->regionRightActor->SetVisibility(showBox);
    this->regionCircleActor->SetVisibility(showCircle);
    this->regionAnnulusOuterActor->SetVisibility(showAnnulus);
    this->regionAnnulusInnerActor->SetVisibility(showAnnulus && !this->regionDragging);
    this->regionAnnulusFillActor->SetVisibility(showAnnulus && !this->regionDragging);
    this->regionPolygonActor->SetVisibility(showPolygon);
    this->regionPolygonFillActor->SetVisibility(showPolygon);

    if (showBox) {
        this->regionTopLine->SetPoint1(xmin, ymax, 0.0);
        this->regionTopLine->SetPoint2(xmax, ymax, 0.0);
        this->regionBottomLine->SetPoint1(xmin, ymin, 0.0);
        this->regionBottomLine->SetPoint2(xmax, ymin, 0.0);
        this->regionLeftLine->SetPoint1(xmin, ymin, 0.0);
        this->regionLeftLine->SetPoint2(xmin, ymax, 0.0);
        this->regionRightLine->SetPoint1(xmax, ymin, 0.0);
        this->regionRightLine->SetPoint2(xmax, ymax, 0.0);
    } else if (showCircle || showAnnulus) {
        const double dx = static_cast<double>(this->regionCurrentVoxel[0] - this->regionAnchorVoxel[0]);
        const double dy = static_cast<double>(this->regionCurrentVoxel[1] - this->regionAnchorVoxel[1]);
        const double outerRadius = std::sqrt(dx * dx + dy * dy);
        if (showCircle) {
            this->regionCircleSource->SetCenter(this->regionAnchorVoxel[0], this->regionAnchorVoxel[1], 0.0);
            this->regionCircleSource->SetRadius(outerRadius);
        } else {
            this->regionAnnulusOuterSource->SetCenter(this->regionAnchorVoxel[0], this->regionAnchorVoxel[1], 0.0);
            this->regionAnnulusOuterSource->SetRadius(outerRadius);
            if (!this->regionDragging) {
                this->regionAnnulusInnerSource->SetCenter(this->regionAnchorVoxel[0], this->regionAnchorVoxel[1], 0.0);
                const double innerRadius = std::max(0.0, std::min(this->regionAnnulusInnerRadius, outerRadius));
                this->regionAnnulusInnerSource->SetRadius(innerRadius);
                buildAnnulusFill(this->regionAnnulusFillPoints, this->regionAnnulusFillCells,
                                 this->regionAnnulusFillData, this->regionAnchorVoxel, innerRadius,
                                 outerRadius, 96);
                this->regionAnnulusFillActor->SetVisibility(innerRadius < outerRadius);
            } else {
                this->regionAnnulusFillData->Initialize();
                this->regionAnnulusInnerActor->SetVisibility(false);
                this->regionAnnulusFillActor->SetVisibility(false);
            }
        }
    } else if (showPolygon) {
        std::vector<std::array<int, 2>> drawVertices = this->regionPolygonVertices;
        if (this->regionDragging && (drawVertices.empty() || drawVertices.back() != this->regionCurrentVoxel)) {
            drawVertices.push_back(this->regionCurrentVoxel);
        }
        const bool previewClosure = this->regionDragging && drawVertices.size() >= 3
                && distance2d(this->regionCurrentVoxel, drawVertices.front()) <= polygonClosureTolerance;
        this->regionPolygonPoints->Reset();
        this->regionPolygonCells->Reset();
        this->regionPolygonFillData->Initialize();
        if (drawVertices.size() >= 2) {
            const bool closed = !this->regionDragging || previewClosure;
            const vtkIdType count = static_cast<vtkIdType>(drawVertices.size() + (closed ? 1 : 0));
            this->regionPolygonCells->InsertNextCell(count);
            for (vtkIdType i = 0; i < static_cast<vtkIdType>(drawVertices.size()); ++i) {
                this->regionPolygonPoints->InsertNextPoint(drawVertices[static_cast<std::size_t>(i)][0],
                                                           drawVertices[static_cast<std::size_t>(i)][1], 0.0);
                this->regionPolygonCells->InsertCellPoint(i);
            }
            if (closed) {
                this->regionPolygonPoints->InsertNextPoint(drawVertices.front()[0], drawVertices.front()[1], 0.0);
                this->regionPolygonCells->InsertCellPoint(static_cast<vtkIdType>(drawVertices.size()));
            }
            this->regionPolygonData->SetPoints(this->regionPolygonPoints);
            this->regionPolygonData->SetLines(this->regionPolygonCells);
            this->regionPolygonData->Modified();
            if (drawVertices.size() >= 3 && closed) {
                this->regionPolygonFillData->SetPoints(this->regionPolygonPoints);
                this->regionPolygonFillData->SetLines(this->regionPolygonCells);
                this->regionPolygonFillData->Modified();
                this->regionPolygonTriangulator->Update();
                this->regionPolygonFillActor->SetVisibility(true);
            } else {
                this->regionPolygonFillActor->SetVisibility(false);
            }
        } else {
            this->regionPolygonFillActor->SetVisibility(false);
        }
    } else {
        this->regionAnnulusFillActor->SetVisibility(false);
        this->regionPolygonFillActor->SetVisibility(false);
    }
}

void vtkWindowImage::clearRegion()
{
    this->regionValid = false;
    this->regionDragging = false;
    this->ignoreNextPolygonRelease = false;
    this->regionPolygonVertices.clear();
    this->regionAnnulusInnerRadius = 0.0;
    this->regionTopActor->VisibilityOff();
    this->regionBottomActor->VisibilityOff();
    this->regionLeftActor->VisibilityOff();
    this->regionRightActor->VisibilityOff();
    this->regionCircleActor->VisibilityOff();
    this->regionAnnulusOuterActor->VisibilityOff();
    this->regionAnnulusInnerActor->VisibilityOff();
    this->regionAnnulusFillActor->VisibilityOff();
    this->regionPolygonActor->VisibilityOff();
    this->regionPolygonFillActor->VisibilityOff();
}

void vtkWindowImage::analyzeCurrentRegion()
{
    if (this->regionMode == RegionMode::None || !this->regionValid || !this->layers) {
        return;
    }

    auto *imageData = this->layers->getImageData(this->layers->getMasterIndex());
    const auto stats = computeRegionStatistics2D(imageData, this->regionMode, this->regionAnchorVoxel,
                                                 this->regionCurrentVoxel, this->regionPolygonVertices,
                                                 this->regionAnnulusInnerRadius);
    QString text;
    if (!stats.valid) {
        text = u"No valid pixels in the selected region.\nBlanked/NaN pixels are ignored."_s;
    } else {
        text = u"Shape: %1\nValid pixels: %2 / %3\nBlanked/NaN: %4\nMin: %5\nMax: %6\nMean: %7\nMedian: %8\nStddev: %9\n\nComputed on valid pixels only."_s
                       .arg(regionModeLabel(this->regionMode))
                       .arg(stats.validCount)
                       .arg(stats.totalCount)
                       .arg(stats.blankedCount)
                       .arg(stats.minValue, 0, 'g', 8)
                       .arg(stats.maxValue, 0, 'g', 8)
                       .arg(stats.mean, 0, 'g', 8)
                       .arg(stats.median, 0, 'g', 8)
                       .arg(stats.stddev, 0, 'g', 8);
    }
    if (this->isRemoteMode) {
        text += u"\n\nComputed on the currently loaded data block."_s;
    }
    QMessageBox::information(this, u"Region Statistics"_s, text);
}

bool vtkWindowImage::finalizePolygonRegion()
{
    const int *position = ui->vtk->renderWindow()->GetInteractor()->GetEventPosition();
    if (position) {
        this->updateRegionFromDisplayPosition(position[0], position[1]);
        const bool closesToFirst = this->regionPolygonVertices.size() >= 3
                && this->regionCurrentVoxel == this->regionPolygonVertices.front();
        if (!closesToFirst && (this->regionPolygonVertices.empty()
                               || this->regionPolygonVertices.back() != this->regionCurrentVoxel)) {
            this->regionPolygonVertices.push_back(this->regionCurrentVoxel);
        }
    }

    std::vector<std::array<int, 2>> cleaned;
    cleaned.reserve(this->regionPolygonVertices.size());
    for (const auto &vertex : this->regionPolygonVertices) {
        if (cleaned.empty() || cleaned.back() != vertex) {
            cleaned.push_back(vertex);
        }
    }
    this->regionPolygonVertices = std::move(cleaned);
    if (this->regionPolygonVertices.size() < 3) {
        this->statusBar()->showMessage(u"Define at least three points for a polygon region."_s, 2500);
        return false;
    }

    this->regionDragging = false;
    this->regionValid = true;
    this->refreshRegionOverlay();
    this->analyzeCurrentRegion();
    if (this->actionPolygonRegion && this->actionPolygonRegion->isChecked()) {
        this->actionPolygonRegion->setChecked(false);
    }
    return true;
}

void vtkWindowImage::loadCatalogueOverlay()
{
    const QString path = QFileDialog::getOpenFileName(
            this, u"Load Catalogue Overlay"_s, QString(),
            u"Catalogue files (*.reg *.csv);;DS9 region (*.reg);;CSV (*.csv)"_s);
    if (path.isEmpty()) {
        return;
    }

    const CatalogueOverlayParseResult parsed = CatalogueOverlayUtils::parseFile(path);
    if (!parsed.valid) {
        QMessageBox::warning(this, u"Catalogue Overlay"_s, parsed.errorMessage);
        return;
    }

    if ((!this->isRemoteMode && !this->astro) || (this->isRemoteMode && !this->remoteHasCelestialAxes())) {
        QMessageBox::warning(this, u"Catalogue Overlay"_s,
                             u"Catalogue overlay requires celestial WCS information."_s);
        return;
    }

    this->catalogueOverlayPixels.clear();
    this->catalogueOverlayLabels.clear();
    this->catalogueOverlayLabelIndices.clear();
    int skippedProjection = 0;
    for (const CatalogueOverlayEntry &entry : parsed.entries) {
        std::array<double, 2> pixel{ 0.0, 0.0 };
        if (!this->catalogueWorldToPixel(entry.raDeg, entry.decDeg, pixel)) {
            ++skippedProjection;
            continue;
        }
        this->catalogueOverlayPixels.push_back(pixel);
        this->catalogueOverlayLabels.push_back(entry.label);
    }

    if (this->catalogueOverlayPixels.empty()) {
        QMessageBox::warning(this, u"Catalogue Overlay"_s,
                             u"No sources could be projected onto the current image WCS."_s);
        return;
    }

    this->catalogueOverlayLoaded = true;
    this->catalogueOverlaySummary =
            u"%1 sources from %2 (%3)"_s.arg(this->catalogueOverlayPixels.size())
                    .arg(parsed.sourceLabel, parsed.frameLabel);
    this->rebuildCatalogueOverlay();
    if (this->actionShowCatalogueOverlay) {
        this->actionShowCatalogueOverlay->setEnabled(true);
        this->actionShowCatalogueOverlay->setChecked(true);
    }
    if (this->actionClearCatalogueOverlay) {
        this->actionClearCatalogueOverlay->setEnabled(true);
    }
    this->setCatalogueOverlayVisible(true);
    this->showPersistentStatusMessage(
            u"Catalogue overlay loaded: %1 sources (%2)."_s.arg(this->catalogueOverlayPixels.size())
                    .arg(parsed.frameLabel),
            3000);
    if (skippedProjection > 0 || parsed.skippedEntries > 0) {
        qDebug().noquote() << QStringLiteral("[catalogue] loaded=%1 skipped_parse=%2 skipped_wcs=%3")
                                          .arg(this->catalogueOverlayPixels.size())
                                          .arg(parsed.skippedEntries)
                                          .arg(skippedProjection);
    }
    ui->vtk->renderWindow()->Render();
}

void vtkWindowImage::clearCatalogueOverlay()
{
    this->catalogueOverlayLoaded = false;
    this->catalogueOverlayPixels.clear();
    this->catalogueOverlayLabels.clear();
    this->catalogueOverlayLabelIndices.clear();
    this->catalogueOverlaySummary.clear();
    this->catalogueOverlayPoints->Reset();
    this->catalogueOverlayCells->Reset();
    this->catalogueOverlayData->Modified();
    if (auto *renderer = ui->vtk->renderWindow()->GetRenderers()->GetFirstRenderer()) {
        for (const auto &actor : this->catalogueOverlayLabelActors) {
            renderer->RemoveViewProp(actor);
        }
    }
    this->catalogueOverlayLabelActors.clear();
    this->catalogueOverlayActor->VisibilityOff();
    if (this->actionShowCatalogueOverlay) {
        const QSignalBlocker blocker(this->actionShowCatalogueOverlay);
        this->actionShowCatalogueOverlay->setChecked(false);
        this->actionShowCatalogueOverlay->setEnabled(false);
    }
    if (this->actionClearCatalogueOverlay) {
        this->actionClearCatalogueOverlay->setEnabled(false);
    }
    ui->vtk->renderWindow()->Render();
}

void vtkWindowImage::setCatalogueOverlayVisible(bool visible)
{
    const bool effectiveVisible = visible && this->catalogueOverlayLoaded;
    this->catalogueOverlayActor->SetVisibility(effectiveVisible ? 1 : 0);
    for (const auto &actor : this->catalogueOverlayLabelActors) {
        actor->SetVisibility(effectiveVisible ? 1 : 0);
    }
    if (effectiveVisible) {
        this->updateCatalogueOverlayLabels();
    }
    if (ui && ui->vtk && ui->vtk->renderWindow()) {
        ui->vtk->renderWindow()->Render();
    }
}

void vtkWindowImage::rebuildCatalogueOverlay()
{
    this->catalogueOverlayPoints->Reset();
    this->catalogueOverlayCells->Reset();
    this->catalogueOverlayLabelIndices.clear();
    for (std::size_t i = 0; i < this->catalogueOverlayPixels.size(); ++i) {
        const double x = this->catalogueOverlayPixels[i][0];
        const double y = this->catalogueOverlayPixels[i][1];
        const vtkIdType p0 =
                this->catalogueOverlayPoints->InsertNextPoint(x - catalogueMarkerHalfSize, y, 0.0);
        const vtkIdType p1 =
                this->catalogueOverlayPoints->InsertNextPoint(x + catalogueMarkerHalfSize, y, 0.0);
        const vtkIdType p2 =
                this->catalogueOverlayPoints->InsertNextPoint(x, y - catalogueMarkerHalfSize, 0.0);
        const vtkIdType p3 =
                this->catalogueOverlayPoints->InsertNextPoint(x, y + catalogueMarkerHalfSize, 0.0);
        this->catalogueOverlayCells->InsertNextCell(2);
        this->catalogueOverlayCells->InsertCellPoint(p0);
        this->catalogueOverlayCells->InsertCellPoint(p1);
        this->catalogueOverlayCells->InsertNextCell(2);
        this->catalogueOverlayCells->InsertCellPoint(p2);
        this->catalogueOverlayCells->InsertCellPoint(p3);
        if (!this->catalogueOverlayLabels.value(static_cast<qsizetype>(i)).trimmed().isEmpty()
            && static_cast<int>(this->catalogueOverlayLabelIndices.size()) < maxCatalogueLabelCount) {
            this->catalogueOverlayLabelIndices.push_back(static_cast<int>(i));
        }
    }
    this->catalogueOverlayData->SetPoints(this->catalogueOverlayPoints);
    this->catalogueOverlayData->SetLines(this->catalogueOverlayCells);
    this->catalogueOverlayData->Modified();

    auto *renderer = ui->vtk->renderWindow()->GetRenderers()->GetFirstRenderer();
    if (!renderer) {
        return;
    }
    for (const auto &actor : this->catalogueOverlayLabelActors) {
        renderer->RemoveViewProp(actor);
    }
    this->catalogueOverlayLabelActors.clear();
    for (int index : this->catalogueOverlayLabelIndices) {
        auto actor = vtkSmartPointer<vtkTextActor>::New();
        actor->SetInput(this->catalogueOverlayLabels.value(index).toUtf8().constData());
        actor->GetTextProperty()->SetColor(1.0, 0.65, 0.25);
        actor->GetTextProperty()->SetFontSize(12);
        actor->GetTextProperty()->SetBold(false);
        actor->GetTextProperty()->SetShadow(true);
        actor->SetVisibility(this->actionShowCatalogueOverlay && this->actionShowCatalogueOverlay->isChecked());
        renderer->AddViewProp(actor);
        this->catalogueOverlayLabelActors.push_back(actor);
    }
    this->updateCatalogueOverlayLabels();
}

void vtkWindowImage::updateCatalogueOverlayLabels()
{
    if (!this->catalogueOverlayLoaded || !this->actionShowCatalogueOverlay
        || !this->actionShowCatalogueOverlay->isChecked()) {
        return;
    }
    auto *renderer = ui->vtk->renderWindow()->GetRenderers()->GetFirstRenderer();
    if (!renderer) {
        return;
    }

    vtkNew<vtkCoordinate> coordinate;
    coordinate->SetCoordinateSystemToWorld();
    for (std::size_t i = 0; i < this->catalogueOverlayLabelActors.size()
                             && i < this->catalogueOverlayLabelIndices.size();
         ++i) {
        const int pointIndex = this->catalogueOverlayLabelIndices[i];
        if (pointIndex < 0 || static_cast<std::size_t>(pointIndex) >= this->catalogueOverlayPixels.size()) {
            continue;
        }
        coordinate->SetValue(this->catalogueOverlayPixels[pointIndex][0],
                             this->catalogueOverlayPixels[pointIndex][1], 0.0);
        int *display = coordinate->GetComputedDisplayValue(renderer);
        if (!display) {
            continue;
        }
        this->catalogueOverlayLabelActors[i]->SetDisplayPosition(display[0] + 4, display[1] + 4);
    }
}

bool vtkWindowImage::catalogueWorldToPixel(double raDeg, double decDeg, std::array<double, 2> &pixel) const
{
    if (!std::isfinite(raDeg) || !std::isfinite(decDeg)) {
        return false;
    }

    if (!this->isRemoteMode) {
        if (!this->astro) {
            return false;
        }
        double pos[2] = { raDeg, decDeg };
        double pix[2] = { 0.0, 0.0 };
        this->astro->sky2xy(pos, pix, WCS_J2000);
        if (!std::isfinite(pix[0]) || !std::isfinite(pix[1])) {
            return false;
        }
        pixel = { pix[0], pix[1] };
        return true;
    }

    if (!this->remoteHasCelestialAxes()) {
        return false;
    }

    double nativeX = raDeg;
    double nativeY = decDeg;
    const int nativeFrame = this->remoteNativeCelestialFrame();
    if (nativeFrame < 0) {
        return false;
    }
    if (nativeFrame != WCS_J2000) {
        wcscon(WCS_J2000, nativeFrame, 2000.0, 2000.0, &nativeX, &nativeY, 2000.0);
    }
    if (std::abs(this->remoteDatasetCdelt[0]) <= std::numeric_limits<double>::epsilon()
        || std::abs(this->remoteDatasetCdelt[1]) <= std::numeric_limits<double>::epsilon()) {
        return false;
    }
    pixel[0] = ((nativeX - this->remoteDatasetCrval[0]) / this->remoteDatasetCdelt[0])
            + this->remoteDatasetCrpix[0] - 1.0;
    pixel[1] = ((nativeY - this->remoteDatasetCrval[1]) / this->remoteDatasetCdelt[1])
            + this->remoteDatasetCrpix[1] - 1.0;
    return std::isfinite(pixel[0]) && std::isfinite(pixel[1]);
}

void vtkWindowImage::setProbeModeActive(bool active)
{
    this->probeModeActive = active;
    if (active) {
        if (this->actionBoxRegion && this->actionBoxRegion->isChecked()) {
            this->actionBoxRegion->setChecked(false);
        }
        if (this->actionCircleRegion && this->actionCircleRegion->isChecked()) {
            this->actionCircleRegion->setChecked(false);
        }
        if (this->actionPolygonRegion && this->actionPolygonRegion->isChecked()) {
            this->actionPolygonRegion->setChecked(false);
        }
        if (this->actionAnnulusRegion && this->actionAnnulusRegion->isChecked()) {
            this->actionAnnulusRegion->setChecked(false);
        }
    }
    ui->vtk->setCursor(active ? Qt::CrossCursor : Qt::ArrowCursor);
    this->probeFrozen = false;
    if (!active) {
        this->probeHorizontalActor->VisibilityOff();
        this->probeVerticalActor->VisibilityOff();
        if (this->profileWidget) {
            this->profileWidget->close();
        }
        this->vtkRender();
        return;
    }

    if (!this->profileWidget) {
        this->profileWidget = new ProfileWidget(this);
        this->profileWidget->setUsageMode(ProfileWidget::UsageMode::ProbeLive, u"Profile"_s);
        this->profileWidget->setupImagePlots(u"Pixel"_s, u"Value"_s);
        QObject::connect(this->profileWidget, &ProfileWidget::destroyed, this, [this]() {
            this->profileWidget = nullptr;
            if (ui->actionProfile->isChecked()) {
                ui->actionProfile->setChecked(false);
            }
        });
    } else {
        this->profileWidget->setUsageMode(ProfileWidget::UsageMode::ProbeLive, u"Profile"_s);
    }

    if (!this->probeValid) {
        auto *imageData = this->layers ? this->layers->getImageData(this->layers->getMasterIndex()) : nullptr;
        if (imageData) {
            int extent[6];
            imageData->GetExtent(extent);
            this->probeValid = true;
            this->probeVoxel = { (extent[0] + extent[1]) / 2, (extent[2] + extent[3]) / 2 };
        }
    }
    this->refreshProbeOverlay();
    this->updateProbeProfile();
    this->profileWidget->show();
    this->profileWidget->raise();
    this->vtkRender();
}

void vtkWindowImage::updateProbeProfile()
{
    if (!this->probeModeActive || !this->probeValid || !this->profileWidget || !this->layers) {
        return;
    }

    auto *imageData = this->layers->getImageData(this->layers->getMasterIndex());
    if (!imageData) {
        return;
    }
    int extent[6];
    imageData->GetExtent(extent);
    if (this->probeVoxel[0] < extent[0] || this->probeVoxel[0] > extent[1] || this->probeVoxel[1] < extent[2]
        || this->probeVoxel[1] > extent[3]) {
        return;
    }

    const int width = extent[1] - extent[0] + 1;
    const int height = extent[3] - extent[2] + 1;
    QVector<double> keyX(width), valuesX(width), keyY(height), valuesY(height);
    for (int x = extent[0]; x <= extent[1]; ++x) {
        const int idx = x - extent[0];
        keyX[idx] = x;
        valuesX[idx] = this->layers->getPixelValue(this->layers->getMasterIndex(), x, this->probeVoxel[1]);
    }
    for (int y = extent[2]; y <= extent[3]; ++y) {
        const int idx = y - extent[2];
        keyY[idx] = y;
        valuesY[idx] = this->layers->getPixelValue(this->layers->getMasterIndex(), this->probeVoxel[0], y);
    }
    this->profileWidget->updateImageProfiles(keyX, valuesX, keyY, valuesY, this->probeVoxel[0],
                                             this->probeVoxel[1], !this->probeFrozen);
}

void vtkWindowImage::setInteractorStyleImage()
{
    vtkNew<vtkInteractorStyleImage> style;
    ui->vtk->renderWindow()->GetInteractor()->SetInteractorStyle(style);
    this->vtkRender();
}

void vtkWindowImage::setInteractorStyleRegion()
{
    vtkNew<vtkInteractorStyleUser> style;
    ui->vtk->renderWindow()->GetInteractor()->SetInteractorStyle(style);
    this->vtkRender();
}

void vtkWindowImage::setInteractorStyleProfile()
{
    if (!this->profileWidget) {
        vtkNew<vtkInteractorStyleProfile> style;
        ui->vtk->renderWindow()->GetInteractor()->SetInteractorStyle(style);
        this->vtkRender();

        this->profileWidget =
                new ProfileWidget(style, this->layers->getImageData(this->layers->getMasterIndex()),
                                  this->filepath.toStdString(), this);
        this->profileWidget->setUsageMode(ProfileWidget::UsageMode::ProbeLive, u"Profile"_s);
        this->profileWidget->setupImagePlots();
        QObject::connect(this->profileWidget, &ProfileWidget::destroyed, this,
                         &vtkWindowImage::setInteractorStyleImage, Qt::QueuedConnection);
    }
}

int vtkWindowImage::currentLayerIndex() const
{
    return ui->listLayer->currentIndex().row();
}

void vtkWindowImage::addLayerImage(const std::string &filepath)
{
    this->setLayerImportEnabled(false);
    this->showPersistentStatusMessage(u"Loading layer..."_s);
    this->layerLoadWatcher.setFuture(QtConcurrent::run(
            &loadImageLayer, ImageLayerLoadRequest { this->filepath.toStdString(), filepath }));
}

void vtkWindowImage::applyLoadedLayer(const ImageLayerLoadResult &result)
{
    QElapsedTimer timer;
    timer.start();
    this->stack->AddImage(this->layers->addLayer(result));
    if (this->probeValid) {
        this->refreshProbeOverlay();
    }
    this->updateDataStatePanel();
    this->updateSanityPanel();
    this->vtkRender();
    qDebug().noquote()
            << QStringLiteral("[perf][layer] render after apply: %1 ms").arg(timer.elapsed());
}

void vtkWindowImage::applyRemoteMasterLayer(const ImageLayerLoadResult &result)
{
    auto *oldActor = this->layers->getMasterLayerActor();
    auto *newActor = this->layers->replaceMasterLayer(result);
    if (!newActor) {
        return;
    }

    if (oldActor) {
        this->stack->RemoveImage(oldActor);
    }
    this->stack->AddImage(newActor);
    this->stack->SetActiveLayer(this->layers->getMasterIndex());
    this->layerController->activateLayer(this->layers->getMasterIndex());
    auto *renderer = ui->vtk->renderWindow()->GetRenderers()->GetFirstRenderer();
    if (renderer) {
        renderer->ResetCamera();
    }
    if (this->probeValid) {
        this->refreshProbeOverlay();
    }
    this->updateDataStatePanel();
    this->updateSanityPanel();
    this->vtkRender();
}

bool vtkWindowImage::isBusy() const
{
    return this->layerLoadWatcher.isRunning() || this->remoteImageWatcher.isRunning();
}

void vtkWindowImage::showPersistentStatusMessage(const QString &text, int minDurationMs)
{
    this->statusMessageClearTimer.stop();
    this->persistentStatusActive = true;
    this->statusMessageMinDurationMs = minDurationMs;
    this->statusMessageElapsed.restart();
    this->statusBar()->showMessage(text);
}

void vtkWindowImage::clearPersistentStatusMessage()
{
    if (!this->persistentStatusActive) {
        this->statusBar()->clearMessage();
        return;
    }

    const int remaining = this->statusMessageMinDurationMs - this->statusMessageElapsed.elapsed();
    if (remaining <= 0) {
        this->persistentStatusActive = false;
        this->statusBar()->clearMessage();
        return;
    }

    this->statusMessageClearTimer.start(remaining);
}

void vtkWindowImage::setLayerImportEnabled(bool enabled)
{
    ui->actionAddFITS->setEnabled(enabled && !this->isRemoteMode);
}

void vtkWindowImage::updateWcsOverlay()
{
    if (!this->wcsOverlayInitialized) {
        qDebug().noquote() << QStringLiteral("[wcs-overlay] vtkWindowImage not initialized yet");
        return;
    }

    if (!ui || !ui->vtk) {
        qDebug().noquote() << QStringLiteral("[wcs-overlay] vtkWindowImage missing ui/ui->vtk ui=%1 vtk=%2")
                                      .arg(reinterpret_cast<quintptr>(ui), 0, 16)
                                      .arg(reinterpret_cast<quintptr>(ui ? ui->vtk : nullptr), 0, 16);
        return;
    }

    auto *renderWindow = ui->vtk->renderWindow();
    auto *rendererCollection = renderWindow ? renderWindow->GetRenderers() : nullptr;
    auto *renderer = rendererCollection ? rendererCollection->GetFirstRenderer() : nullptr;
    auto *imageData = this->layers ? this->layers->getImageData(this->layers->getMasterIndex()) : nullptr;
    if (!renderWindow || !renderer || !this->overlayXAxis.GetPointer() || !this->overlayYAxis.GetPointer()
        || !this->overlayXTitleActor.GetPointer() || !this->overlayYTitleActor.GetPointer()) {
        qDebug().noquote()
                << QStringLiteral("[wcs-overlay] vtkWindowImage missing objects renderer=%1 renderWindow=%2 xAxis=%3 yAxis=%4 xTitle=%5 yTitle=%6 image=%7")
                           .arg(reinterpret_cast<quintptr>(renderer), 0, 16)
                           .arg(reinterpret_cast<quintptr>(renderWindow), 0, 16)
                           .arg(reinterpret_cast<quintptr>(this->overlayXAxis.GetPointer()), 0, 16)
                           .arg(reinterpret_cast<quintptr>(this->overlayYAxis.GetPointer()), 0, 16)
                           .arg(reinterpret_cast<quintptr>(this->overlayXTitleActor.GetPointer()), 0, 16)
                           .arg(reinterpret_cast<quintptr>(this->overlayYTitleActor.GetPointer()), 0, 16)
                           .arg(reinterpret_cast<quintptr>(imageData), 0, 16);
        return;
    }
    const bool useLegend = this->astro && !this->astro->isSimulation();
    this->setWcsOverlayVisible(this->showWcsAxes);
    if (!this->showWcsAxes || useLegend || !renderer || !imageData) {
        return;
    }
    this->ensureOverlayTickActors(renderer);
    if (this->overlayXTickActors.size() < overlayTickCount
        || this->overlayYTickActors.size() < overlayTickCount) {
        qDebug().noquote()
                << QStringLiteral("[wcs-overlay] vtkWindowImage tick actors incomplete x=%1 y=%2")
                           .arg(this->overlayXTickActors.size())
                           .arg(this->overlayYTickActors.size());
        return;
    }

    const auto visible = computeVisibleImageBounds2D(renderer, imageData);
    if (!visible.valid) {
        this->overlayXAxis->VisibilityOff();
        this->overlayYAxis->VisibilityOff();
        this->overlayXTitleActor->VisibilityOff();
        this->overlayYTitleActor->VisibilityOff();
        return;
    }

    const int *size = renderer->GetSize();
    if (!size || size[0] <= 0 || size[1] <= 0) {
        return;
    }

    const std::array<double, 4> visibleBounds = { visible.xmin, visible.xmax, visible.ymin, visible.ymax };
    const std::array<int, 2> viewportSize = { size[0], size[1] };
    if (visibleBounds == this->lastOverlayVisibleBounds && viewportSize == this->lastOverlayViewportSize) {
        return;
    }
    this->lastOverlayVisibleBounds = visibleBounds;
    this->lastOverlayViewportSize = viewportSize;

    constexpr double leftMargin = 168.;
    constexpr double axisX = 136.;
    constexpr double bottomMargin = 58.;
    constexpr double rightMargin = 34.;
    constexpr double topMargin = 28.;
    configureAxisActor(this->overlayXAxis, axisX, bottomMargin, size[0] - rightMargin, bottomMargin);
    configureAxisActor(this->overlayYAxis, axisX, size[1] - topMargin, axisX, bottomMargin);
    this->overlayXAxis->SetTitle("");
    this->overlayXTitleActor->SetInput(this->remoteOverlayAxisTitle(0).toStdString().c_str());
    this->overlayXTitleActor->SetDisplayPosition((axisX + (size[0] - rightMargin)) / 2,
                                                 static_cast<int>(bottomMargin / 2.0) - 2);
    this->overlayYAxis->SetTitle("");
    this->overlayYTitleActor->SetInput(this->remoteOverlayAxisTitle(1).toStdString().c_str());
    this->overlayYTitleActor->SetDisplayPosition(static_cast<int>(leftMargin / 3.0), size[1] / 2);

    bool xOk = false;
    const double xMin = this->remoteVoxelToWcs(0, visible.xmin, &xOk);
    const double xMax = this->remoteVoxelToWcs(0, visible.xmax, &xOk);
    bool yOk = false;
    const double yMin = this->remoteVoxelToWcs(1, visible.ymin, &yOk);
    const double yMax = this->remoteVoxelToWcs(1, visible.ymax, &yOk);
    this->overlayXAxis->SetRange(xOk ? xMin : visible.xmin, xOk ? xMax : visible.xmax);
    this->overlayYAxis->SetRange(yOk ? yMax : visible.ymax, yOk ? yMin : visible.ymin);
    this->overlayXAxis->LabelVisibilityOff();
    this->overlayYAxis->LabelVisibilityOff();
    this->overlayXAxis->VisibilityOn();
    this->overlayYAxis->VisibilityOn();
    this->overlayXTitleActor->VisibilityOn();
    this->overlayYTitleActor->VisibilityOn();
    for (int i = 0; i < overlayTickCount; ++i) {
        const double t = overlayTickCount == 1 ? 0.0
                                               : static_cast<double>(i)
                        / static_cast<double>(overlayTickCount - 1);
        const double voxelX = visible.xmin + t * (visible.xmax - visible.xmin);
        const double voxelY = visible.ymin + t * (visible.ymax - visible.ymin);
        bool tickXOk = false;
        bool tickYOk = false;
        double tickX = this->remoteVoxelToWcs(0, voxelX, &tickXOk);
        double tickY = this->remoteVoxelToWcs(1, voxelY, &tickYOk);
        double frameX = tickX;
        double frameY = tickY;
        if (tickXOk && tickYOk && this->remoteHasCelestialAxes()) {
            this->convertRemoteCelestialCoordinates(tickX, tickY, frameX, frameY);
        }

        this->overlayXTickActors[static_cast<std::size_t>(i)]->SetInput(
                this->formatRemoteOverlayCoordinate(0, frameX).toStdString().c_str());
        this->overlayXTickActors[static_cast<std::size_t>(i)]->SetDisplayPosition(
                static_cast<int>(axisX + t * ((size[0] - rightMargin) - axisX)),
                static_cast<int>(bottomMargin - 18));
        this->overlayXTickActors[static_cast<std::size_t>(i)]->VisibilityOn();

        this->overlayYTickActors[static_cast<std::size_t>(i)]->SetInput(
                this->formatRemoteOverlayCoordinate(1, frameY).toStdString().c_str());
        this->overlayYTickActors[static_cast<std::size_t>(i)]->SetDisplayPosition(
                static_cast<int>(axisX - 10),
                static_cast<int>(bottomMargin + t * ((size[1] - topMargin) - bottomMargin)));
        this->overlayYTickActors[static_cast<std::size_t>(i)]->VisibilityOn();
    }
    qDebug().noquote()
            << QStringLiteral("[wcs-overlay] updated ticks x=%1..%2 y=%3..%4 size=%5x%6 renderer=%7 renderWindow=%8 xAxis=%9 yAxis=%10 xTitle=%11 yTitle=%12 endpoints=(%13,%14)->(%15,%16) outer=%17")
                       .arg(visible.xmin, 0, 'g', 12)
                       .arg(visible.xmax, 0, 'g', 12)
                       .arg(visible.ymin, 0, 'g', 12)
                       .arg(visible.ymax, 0, 'g', 12)
                       .arg(size[0])
                       .arg(size[1])
                       .arg(reinterpret_cast<quintptr>(renderer), 0, 16)
                       .arg(reinterpret_cast<quintptr>(renderWindow), 0, 16)
                       .arg(reinterpret_cast<quintptr>(this->overlayXAxis.GetPointer()), 0, 16)
                       .arg(reinterpret_cast<quintptr>(this->overlayYAxis.GetPointer()), 0, 16)
                       .arg(reinterpret_cast<quintptr>(this->overlayXTitleActor.GetPointer()), 0, 16)
                       .arg(reinterpret_cast<quintptr>(this->overlayYTitleActor.GetPointer()), 0, 16)
                       .arg(axisX, 0, 'g', 12)
                       .arg(size[1] - topMargin, 0, 'g', 12)
                       .arg(axisX, 0, 'g', 12)
                       .arg(bottomMargin, 0, 'g', 12)
                       .arg(leftMargin, 0, 'g', 12);
}

void vtkWindowImage::setWcsOverlayVisible(bool visible)
{
    if (!this->wcsOverlayInitialized) {
        return;
    }
    const bool useLegend = this->astro && !this->astro->isSimulation();
    if (this->legendWCS) {
        this->legendWCS->SetVisibility(visible && useLegend);
    }
    this->overlayXAxis->SetVisibility(visible && !useLegend);
    this->overlayYAxis->SetVisibility(visible && !useLegend);
    this->overlayXTitleActor->SetVisibility(visible && !useLegend);
    this->overlayYTitleActor->SetVisibility(visible && !useLegend);
    for (const auto &actor : this->overlayXTickActors) {
        if (actor) {
            actor->SetVisibility(visible && !useLegend);
        }
    }
    for (const auto &actor : this->overlayYTickActors) {
        if (actor) {
            actor->SetVisibility(visible && !useLegend);
        }
    }
}

void vtkWindowImage::ensureOverlayTickActors(vtkRenderer *renderer)
{
    if (!renderer) {
        return;
    }
    if (this->overlayXTickActors.empty()) {
        for (int i = 0; i < overlayTickCount; ++i) {
            auto actor = vtkSmartPointer<vtkTextActor>::New();
            configureTickLabelActor(actor, false);
            renderer->AddViewProp(actor);
            this->overlayXTickActors.push_back(actor);
        }
    }
    if (this->overlayYTickActors.empty()) {
        for (int i = 0; i < overlayTickCount; ++i) {
            auto actor = vtkSmartPointer<vtkTextActor>::New();
            configureTickLabelActor(actor, true);
            renderer->AddViewProp(actor);
            this->overlayYTickActors.push_back(actor);
        }
    }
}

void vtkWindowImage::invalidateWcsOverlayCache()
{
    this->lastOverlayVisibleBounds = { std::numeric_limits<double>::quiet_NaN(),
                                       std::numeric_limits<double>::quiet_NaN(),
                                       std::numeric_limits<double>::quiet_NaN(),
                                       std::numeric_limits<double>::quiet_NaN() };
    this->lastOverlayViewportSize = { -1, -1 };
}

void vtkWindowImage::applyDefaultWcsFormatForSelectedFrame()
{
    if (this->wcsFormatExplicitlyChosen) {
        return;
    }

    this->useSexagesimalWcsFormat = this->selectedWcsFrame() == WCS_J2000;
    if (this->actionWcsSexagesimal) {
        this->actionWcsSexagesimal->setChecked(this->useSexagesimalWcsFormat);
    }
    if (this->actionWcsDecimal) {
        this->actionWcsDecimal->setChecked(!this->useSexagesimalWcsFormat);
    }
}

void vtkWindowImage::requestWcsOverlayRender()
{
    QMetaObject::invokeMethod(
            this,
            [this]() {
                if (!ui || !ui->vtk || !ui->vtk->renderWindow()) {
                    return;
                }
                this->updateWcsOverlay();
                if (this->probeValid) {
                    this->refreshProbeOverlay();
                }
                ui->vtk->renderWindow()->Render();
                ui->vtk->update();
            },
            Qt::QueuedConnection);
}

QString vtkWindowImage::currentWcsFrameLabel() const
{
    const int frame = this->selectedWcsFrame();
    return frame == WCS_GALACTIC ? u"Galactic"_s
            : (frame == WCS_J2000 ? u"FK5"_s : u"Ecliptic"_s);
}

void vtkWindowImage::updateDataStatePanel()
{
    if (!this->dataStateLabel) {
        return;
    }

    auto *imageData = this->layers ? this->layers->getImageData(this->layers->getMasterIndex()) : nullptr;
    const QString origin = this->isRemoteMode ? u"Remote"_s : u"Local"_s;
    const QString representation = u"Full image"_s;
    const QString loadedBounds = formatImageBoundsSummary(imageData);
    const QString datasetBounds = loadedBounds;
    const QString axis3 = u"Axis3: n/a"_s;
    this->dataStateLabel->setText(
            u"%1 | %2 | Loaded: %3 | Dataset: %4 | WCS: %5 | %6"_s.arg(origin,
                                                                        representation,
                                                                        loadedBounds,
                                                                        datasetBounds,
                                                                        this->currentWcsFrameLabel(),
                                                                        axis3));
    this->dataStateLabel->setToolTip(
            u"Persistent data state: origin, representation, loaded bounds, dataset bounds, current WCS frame."_s);
}

void vtkWindowImage::updateSanityPanel()
{
    if (!this->sanityLabel) {
        return;
    }

    std::array<QString, 3> ctype = this->remoteDatasetCtype;
    std::array<QString, 3> cunit = this->remoteDatasetCunit;
    std::array<double, 3> crval = this->remoteDatasetCrval;
    std::array<double, 3> crpix = this->remoteDatasetCrpix;
    std::array<double, 3> cdelt = this->remoteDatasetCdelt;
    if (!this->isRemoteMode && this->astro) {
        const auto *refValues = this->astro->getReferenceValues();
        const auto *refPixels = this->astro->getReferencePixels();
        const auto *increments = this->astro->getIncrements();
        for (int axis = 0; axis < 3; ++axis) {
            ctype[axis] = QString::fromStdString(this->astro->getAxisType(axis));
            cunit[axis] = QString::fromStdString(this->astro->getAxisUnit(axis));
            crval[axis] = refValues[axis];
            crpix[axis] = refPixels[axis];
            cdelt[axis] = increments[axis];
        }
    }

    auto *imageData = this->layers ? this->layers->getImageData(this->layers->getMasterIndex()) : nullptr;
    const auto report = buildImageSanityReport(this->isRemoteMode, this->astro.get(), ctype, cunit,
                                               crval, crpix, cdelt, imageData);
    this->sanityLabel->setText(report.summary);
    this->sanityLabel->setToolTip(report.details);
}

bool vtkWindowImage::remoteHasWcsAxis(int axis) const
{
    return axis >= 0 && axis < 3 && std::isfinite(this->remoteDatasetCrval[axis])
            && std::isfinite(this->remoteDatasetCrpix[axis])
            && std::isfinite(this->remoteDatasetCdelt[axis])
            && std::abs(this->remoteDatasetCdelt[axis]) > 1e-12;
}

double vtkWindowImage::remoteVoxelToWcs(int axis, double voxelIndex, bool *ok) const
{
    const bool valid = this->remoteHasWcsAxis(axis);
    if (ok) {
        *ok = valid;
    }
    if (!valid) {
        return voxelIndex;
    }

    return this->remoteDatasetCrval[axis]
            + ((voxelIndex + 1.0) - this->remoteDatasetCrpix[axis]) * this->remoteDatasetCdelt[axis];
}

QString vtkWindowImage::remoteFormatAxisCoordinate(int axis, double voxelIndex) const
{
    bool ok = false;
    const double world = this->remoteVoxelToWcs(axis, voxelIndex, &ok);
    if (!ok) {
        return QString::number(voxelIndex, 'g', 12);
    }

    const QString unit = (axis >= 0 && axis < 3) ? this->remoteDatasetCunit[axis].trimmed() : QString();
    if (unit.isEmpty()) {
        return QString::number(world, 'g', 12);
    }
    return u"%1 %2"_s.arg(QString::number(world, 'g', 12), unit);
}

QString vtkWindowImage::remoteAxisTitle(int axis) const
{
    const QString base = axis == 0 ? u"X"_s : (axis == 1 ? u"Y"_s : u"Z"_s);
    const QString ctype = (axis >= 0 && axis < 3) ? this->remoteDatasetCtype[axis].trimmed() : QString();
    const QString cunit = (axis >= 0 && axis < 3) ? this->remoteDatasetCunit[axis].trimmed() : QString();
    const QString label = ctype.isEmpty() ? base : ctype;
    return cunit.isEmpty() ? label : u"%1 (%2)"_s.arg(label, cunit);
}

int vtkWindowImage::selectedWcsFrame() const
{
    return ui->actionGalactic->isChecked() ? WCS_GALACTIC
            : (ui->actionFK5->isChecked() ? WCS_J2000 : WCS_ECLIPTIC);
}

int vtkWindowImage::remoteNativeCelestialFrame() const
{
    return inferCelestialFrameFromCtypePair(this->remoteDatasetCtype);
}

bool vtkWindowImage::remoteHasCelestialAxes() const
{
    return this->remoteNativeCelestialFrame() >= 0 && this->remoteHasWcsAxis(0)
            && this->remoteHasWcsAxis(1);
}

bool vtkWindowImage::convertRemoteCelestialCoordinates(double nativeX, double nativeY, double &frameX,
                                                       double &frameY) const
{
    frameX = nativeX;
    frameY = nativeY;
    const int nativeFrame = this->remoteNativeCelestialFrame();
    const int targetFrame = this->selectedWcsFrame();
    if (nativeFrame < 0) {
        return false;
    }
    if (nativeFrame != targetFrame) {
        wcscon(nativeFrame, targetFrame, 2000.0, 2000.0, &frameX, &frameY, 2000.0);
    }
    return true;
}

QString vtkWindowImage::formatRemoteOverlayCoordinate(int axis, double value) const
{
    if (!this->remoteHasCelestialAxes()) {
        return QString::number(value, 'g', 8);
    }
    if (!this->useSexagesimalWcsFormat) {
        return this->formatDegreeCoordinate(value);
    }
    return formatCelestialCoordinate(this->selectedWcsFrame(), axis, value);
}

QString vtkWindowImage::remoteOverlayAxisTitle(int axis) const
{
    if (!this->remoteHasCelestialAxes()) {
        return this->remoteAxisTitle(axis);
    }
    const int frame = this->selectedWcsFrame();
    if (frame == WCS_J2000) {
        return axis == 0 ? u"Right Ascension"_s : u"Declination"_s;
    }
    if (frame == WCS_GALACTIC) {
        return axis == 0 ? u"Galactic Longitude"_s : u"Galactic Latitude"_s;
    }
    return axis == 0 ? u"Ecliptic Longitude"_s : u"Ecliptic Latitude"_s;
}

QString vtkWindowImage::formatDegreeCoordinate(double value) const
{
    return QString::number(value, 'f', 2);
}

QString vtkWindowImage::selectedFrameAxisTitle(int axis) const
{
    if (this->astro && !this->astro->isSimulation()) {
        const int frame = this->selectedWcsFrame();
        if (frame == WCS_J2000) {
            return axis == 0 ? u"Right Ascension"_s : u"Declination"_s;
        }
        if (frame == WCS_GALACTIC) {
            return axis == 0 ? u"Galactic Longitude"_s : u"Galactic Latitude"_s;
        }
        if (frame == WCS_ECLIPTIC) {
            return axis == 0 ? u"Ecliptic Longitude"_s : u"Ecliptic Latitude"_s;
        }
    }
    return this->remoteOverlayAxisTitle(axis);
}

QString vtkWindowImage::formatLocalProbeCoordinate(int axis, const std::array<int, 2> &voxel) const
{
    if (!this->astro || this->astro->isSimulation()) {
        return QString::number(axis == 0 ? voxel[0] : voxel[1]);
    }

    const double pix[2] = { static_cast<double>(voxel[0]), static_cast<double>(voxel[1]) };
    double pos[2] = { 0., 0. };
    this->astro->xy2sky(pix, pos, this->selectedWcsFrame());
    if (!this->useSexagesimalWcsFormat) {
        return this->formatDegreeCoordinate(pos[axis]);
    }
    return formatCelestialCoordinate(this->selectedWcsFrame(), axis, pos[axis]);
}

void vtkWindowImage::vtkRender()
{
    ui->vtk->renderWindow()->Render();
}

void vtkWindowImage::changeLegendWCS()
{
    const int wcs = (ui->actionGalactic->isChecked()
                             ? WCS_GALACTIC
                             : (ui->actionFK5->isChecked() ? WCS_J2000 : WCS_ECLIPTIC));
    if (this->astro && !this->astro->isSimulation()) {
        this->legendWCS->SetWCS(wcs);
    }
    this->applyDefaultWcsFormatForSelectedFrame();
    this->invalidateWcsOverlayCache();
    qDebug().noquote()
            << QStringLiteral("[wcs] overlay using selected frame %1")
                       .arg(wcs == WCS_GALACTIC ? u"Galactic"_s
                                                : (wcs == WCS_J2000 ? u"FK5"_s : u"Ecliptic"_s));
    this->updateDataStatePanel();
    this->updateSanityPanel();
    this->requestWcsOverlayRender();
}

void vtkWindowImage::changeCurrentColorMap()
{
    this->layerController->setCurrentColorMap(this->currentLayerIndex(),
                                              ui->comboLut->currentText().toStdString());
    this->vtkRender();
}

void vtkWindowImage::changeCurrentColorScale()
{
    this->layerController->setCurrentLogScale(this->currentLayerIndex(), ui->radioLog->isChecked());
    this->vtkRender();
}

void vtkWindowImage::changeCurrentLayerOpacity()
{
    const double opacity = ui->sliderOpacity->sliderPosition() / 100.;
    this->layerController->setCurrentOpacity(this->currentLayerIndex(), opacity);
    this->vtkRender();
}

void vtkWindowImage::showCurrentLayerSettings()
{
    const int index = this->currentLayerIndex();
    const auto state = this->layerController->layerViewState(index);
    if (!state.valid) {
        return;
    }

    ui->comboLut->setCurrentText(QString::fromStdString(state.colorMapName));
    if (state.usingLogScale) {
        ui->radioLog->setChecked(true);
    } else {
        ui->radioLinear->setChecked(true);
    }
    ui->sliderOpacity->setValue(state.opacityPercent);

    this->layerController->activateLayer(index);
    this->vtkRender();
}
