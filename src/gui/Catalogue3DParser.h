#ifndef Catalogue3DParser_h
#define Catalogue3DParser_h

#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include <cmath>
#include <initializer_list>
#include <vector>

// ---------------------------------------------------------------------------
// Data model for a single 3D catalogue source
// ---------------------------------------------------------------------------
struct Catalogue3DEntry
{
    int id{ 0 };
    QString name;
    double raDeg{ 0.0 };
    double decDeg{ 0.0 };
    double redshift{ 0.0 };
    double distanceMpc{ 0.0 };
    QString morphology; // e.g. "halo", "relic", "mini-halo"
    double majorAxisArcmin{ 0.0 };
    double minorAxisArcmin{ 0.0 };
    double fluxMJy{ 0.0 };
    // 3D Cartesian scene coordinates (Mpc)
    double sceneX{ 0.0 };
    double sceneY{ 0.0 };
    double sceneZ{ 0.0 };
};

struct Catalogue3DParseResult
{
    bool valid{ false };
    QString errorMessage;
    int skippedEntries{ 0 };
    std::vector<Catalogue3DEntry> entries;
};

// ---------------------------------------------------------------------------
// Parser
// ---------------------------------------------------------------------------
namespace Catalogue3DParser {

namespace detail {

inline QStringList splitLine(const QString &line)
{
    return line.split(u',', Qt::KeepEmptyParts);
}

inline int findCol(const QStringList &headers,
                   std::initializer_list<const char *> aliases)
{
    for (int i = 0; i < headers.size(); ++i) {
        const QString h = headers.at(i).trimmed().toUpper();
        for (const char *a : aliases) {
            if (h == QString::fromUtf8(a))
                return i;
        }
    }
    return -1;
}

inline double colDouble(const QStringList &cols, int idx, bool *ok = nullptr)
{
    if (idx < 0 || idx >= cols.size()) {
        if (ok)
            *ok = false;
        return 0.0;
    }
    return cols.at(idx).trimmed().toDouble(ok);
}

inline QString colString(const QStringList &cols, int idx)
{
    if (idx < 0 || idx >= cols.size())
        return {};
    return cols.at(idx).trimmed();
}

// Hubble approximation: d_Mpc = z * c / H0  (H0 = 70 km/s/Mpc)
inline constexpr double cOverH0 = 4285.7; // Mpc

inline void fillCartesian(Catalogue3DEntry &e)
{
    // Resolve distance
    if (e.distanceMpc <= 0.0 && e.redshift > 0.0)
        e.distanceMpc = e.redshift * cOverH0;
    if (e.distanceMpc <= 0.0)
        e.distanceMpc = 1.0; // place on unit sphere as fallback

    constexpr double pi = 3.14159265358979323846;
    const double ra  = e.raDeg  * pi / 180.0;
    const double dec = e.decDeg * pi / 180.0;
    const double d   = e.distanceMpc;

    e.sceneX = d * std::cos(dec) * std::cos(ra);
    e.sceneY = d * std::cos(dec) * std::sin(ra);
    e.sceneZ = d * std::sin(dec);
}

} // namespace detail

// ---------------------------------------------------------------------------
// parseFile — accepts any CSV with at least RA and DEC columns (degrees).
// Optional columns: Z/REDSHIFT, DISTANCE, MORPHOLOGY/TYPE, MAJOR_AXIS/LLS,
//                   MINOR_AXIS, FLUX, NAME/ID/LABEL.
// ---------------------------------------------------------------------------
inline Catalogue3DParseResult parseFile(const QString &path)
{
    Catalogue3DParseResult result;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.errorMessage = QStringLiteral("Cannot open file: ") + path;
        return result;
    }

    QTextStream stream(&file);

    // Find header line (skip blank / comment lines)
    QString headerLine;
    while (!stream.atEnd()) {
        headerLine = stream.readLine();
        if (!headerLine.trimmed().isEmpty() && !headerLine.trimmed().startsWith(u'#'))
            break;
        headerLine.clear();
    }
    if (headerLine.trimmed().isEmpty()) {
        result.errorMessage = QStringLiteral("File is empty or has no header.");
        return result;
    }

    const QStringList headers = detail::splitLine(headerLine);

    const int iRA    = detail::findCol(headers, {"RA", "RA_DEG", "RA_J2000", "ALPHA"});
    const int iDec   = detail::findCol(headers, {"DEC", "DEC_DEG", "DEC_J2000", "DE", "DELTA"});
    const int iZ     = detail::findCol(headers, {"Z", "REDSHIFT", "Z_SPEC", "ZSPEC", "Z_MEAN"});
    const int iDist  = detail::findCol(headers, {"DISTANCE", "DIST", "D_MPC", "DIST_MPC"});
    const int iMorph = detail::findCol(headers, {"MORPHOLOGY", "TYPE", "CLASS", "MORPH",
                                                  "CLASSIFICATION", "DIFFUSE_TYPE"});
    const int iMajor = detail::findCol(headers, {"MAJOR_AXIS", "LLS", "MAJOR", "A", "AXIS_A",
                                                  "R_X", "SIZE", "LLS_MPC"});
    const int iMinor = detail::findCol(headers, {"MINOR_AXIS", "MINOR", "B", "AXIS_B"});
    const int iFlux  = detail::findCol(headers, {"FLUX", "FLUX_DENSITY", "S", "S_1_4", "S_MJY",
                                                  "FLUX_MJY"});
    const int iName  = detail::findCol(headers, {"NAME", "ID", "LABEL", "SOURCE_NAME", "SOURCE",
                                                  "OBJNAME"});

    if (iRA < 0 || iDec < 0) {
        result.errorMessage =
                QStringLiteral("CSV must contain RA and DEC columns (degrees).");
        return result;
    }

    int entryId = 0;
    while (!stream.atEnd()) {
        const QString line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith(u'#'))
            continue;

        const QStringList cols = detail::splitLine(line);

        bool okRA = false, okDec = false;
        const double ra  = detail::colDouble(cols, iRA,  &okRA);
        const double dec = detail::colDouble(cols, iDec, &okDec);
        if (!okRA || !okDec) {
            ++result.skippedEntries;
            continue;
        }

        Catalogue3DEntry e;
        e.id     = entryId++;
        e.raDeg  = ra;
        e.decDeg = dec;

        e.name = detail::colString(cols, iName);
        if (e.name.isEmpty())
            e.name = QStringLiteral("Source %1").arg(e.id);

        if (iZ >= 0)
            e.redshift = detail::colDouble(cols, iZ);
        if (iDist >= 0)
            e.distanceMpc = detail::colDouble(cols, iDist);

        e.morphology = detail::colString(cols, iMorph).toLower();
        if (e.morphology.isEmpty())
            e.morphology = QStringLiteral("other");

        if (iMajor >= 0)
            e.majorAxisArcmin = detail::colDouble(cols, iMajor);
        if (iMinor >= 0)
            e.minorAxisArcmin = detail::colDouble(cols, iMinor);
        if (iFlux >= 0)
            e.fluxMJy = detail::colDouble(cols, iFlux);

        detail::fillCartesian(e);
        result.entries.push_back(e);
    }

    if (result.entries.empty()) {
        result.errorMessage = QStringLiteral("No valid RA/DEC entries found in the file.");
        return result;
    }

    result.valid = true;
    return result;
}

} // namespace Catalogue3DParser

#endif // Catalogue3DParser_h
