#ifndef Catalogue3DParser_h
#define Catalogue3DParser_h

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
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

// ---------------------------------------------------------------------------
// Column-name normalisation
// Strip ALL non-alphanumeric characters (parentheses, underscores, spaces,
// dots, …) and convert to upper-case so that all of these map to the same
// token:
//   RA_(J2000)_Obs  →  RAJ2000OBS
//   RAJ2000         →  RAJ2000
//   RA_J2000        →  RAJ2000
//   RA              →  RA
// ---------------------------------------------------------------------------
inline QString normalizeColName(const QString &raw)
{
    QString s;
    s.reserve(raw.size());
    for (const QChar c : raw) {
        if (c.isLetterOrNumber())
            s += c.toUpper();
    }
    return s;
}

inline int findCol(const QStringList &headers,
                   std::initializer_list<const char *> aliases)
{
    for (int i = 0; i < headers.size(); ++i) {
        const QString norm = normalizeColName(headers.at(i));
        for (const char *a : aliases) {
            // aliases are already expected in normalised (uppercase, no punctuation) form
            if (norm == QString::fromUtf8(a))
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

// ---------------------------------------------------------------------------
// Coordinate parsing
//
// Accepts decimal degrees directly or sexagesimal:
//   RA  → HH:MM:SS.s  (hours × 15 = degrees)
//   Dec → ±DD:MM:SS.s (degrees, sign from leading '-')
// ---------------------------------------------------------------------------
inline bool parseSexagesimal(const QString &raw, double &outDeg, bool isRA)
{
    // Split on ':' or whitespace
    const QStringList parts =
            raw.trimmed().split(QRegularExpression(QStringLiteral("[: ]+")),
                                Qt::SkipEmptyParts);
    if (parts.size() < 2)
        return false;

    bool ok0 = false, ok1 = false;
    const double d0 = parts.at(0).toDouble(&ok0);
    const double d1 = parts.at(1).toDouble(&ok1);
    if (!ok0 || !ok1)
        return false;

    double d2 = 0.0;
    if (parts.size() >= 3) {
        bool ok2 = false;
        d2 = parts.at(2).toDouble(&ok2);
        if (!ok2)
            d2 = 0.0;
    }

    const bool negative = raw.trimmed().startsWith(u'-');
    const double absVal = std::abs(d0) + d1 / 60.0 + d2 / 3600.0;
    outDeg = (negative ? -1.0 : 1.0) * absVal;

    if (isRA)
        outDeg *= 15.0; // hours → degrees

    return true;
}

// Parse a coordinate value that may be decimal or sexagesimal.
inline bool parseCoord(const QString &raw, double &outDeg, bool isRA)
{
    const QString s = raw.trimmed();
    if (s.isEmpty())
        return false;

    // Fast path: pure decimal
    bool ok = false;
    const double v = s.toDouble(&ok);
    if (ok) {
        outDeg = v;
        return true;
    }

    // Sexagesimal fallback
    return parseSexagesimal(s, outDeg, isRA);
}

// Hubble approximation: d_Mpc = z * c / H0  (H0 = 70 km/s/Mpc)
inline constexpr double cOverH0 = 4285.7; // Mpc

inline void fillCartesian(Catalogue3DEntry &e)
{
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
// parseFile — accepts any CSV with at least RA and DEC columns.
//
// Column name matching is normalised (punctuation/case-insensitive), so the
// following headers are all accepted for right-ascension:
//   RA, RAJ2000, RA_J2000, RAJ2000OBS, RA_(J2000)_Obs, ALPHA, …
//
// RA/Dec values may be decimal degrees or sexagesimal (HH:MM:SS / DD:MM:SS).
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

    // All aliases are already in normalised form (uppercase, alphanumeric only).
    const int iRA = detail::findCol(headers, {
        // Bare / standard
        "RA", "RADEG",
        // J2000 variants
        "RAJ2000", "RAJ2000OBS", "RAJ2000DEG",
        // Underscored variants (normalised away)
        "RAJ2000",                    // RA_J2000 → RAJ2000
        "RAICRS", "RAICRSOBS",
        // FK5 / misc
        "RAFK5", "RAEQ",
        // Physical
        "ALPHA", "ALPHAJ2000",
    });

    const int iDec = detail::findCol(headers, {
        // Bare / standard
        "DEC", "DE", "DECDEG",
        // J2000 variants
        "DEJ2000", "DEJ2000OBS", "DECJ2000", "DECJ2000OBS", "DECJ2000DEG",
        "DECOBS",
        // ICRS / FK5
        "DECICRS", "DECICRSOBS", "DECFK5",
        // Physical
        "DELTA", "DELTAJ2000",
    });

    const int iZ = detail::findCol(headers, {
        "Z", "REDSHIFT", "REDSHIFTZ",         // Redshift_(z) → REDSHIFTZ
        "ZSPEC", "ZMEAN", "ZPHOT", "ZPHOTOZ",
    });

    const int iDist = detail::findCol(headers, {
        "DISTANCE", "DIST", "DMPC", "DISTMPC",
        "DL", "DLMPC",                         // DL_(Mpc) → DLMPC
        "LUMINOSITYDISTANCE", "COMOVINGDIST",
    });

    const int iMorph = detail::findCol(headers, {
        "MORPHOLOGY", "TYPE", "CLASS", "MORPH", "CLASSIFICATION",
        "DIFFUSETYPE",
        "DIFFMORPHFINAL",                       // DIFF_MORPH_FINAL
        "DIFFUSEMORPHOLOGY",                    // Diffuse_Morphology
        "RADIOMORPHOLOGY",                      // Radio_Morphology_literature (truncated match handled below)
        "RADIOMORPHOLOGYLITERATURE",
    });

    const int iFlux = detail::findCol(headers, {
        "FLUX", "FLUXDENSITY", "S", "S14", "SMJY", "FLUXMJY",
        "DIFFUSEFLUXDENSITY",                   // Diffuse_Flux Density S_… → DIFFUSEFLUXDENSITY…
        "DIFFUSEFLUXDENSITYS128GHZMJY",
        "FLUXDENSITYS128GHZ",
    });

    const int iName = detail::findCol(headers, {
        "NAME", "ID", "LABEL", "SOURCENAME", "SOURCE", "OBJNAME",
        "CLUSTERS",                              // MGCLS catalogue uses "Clusters"
        "CLUSTER", "OBJECT",
    });

    if (iRA < 0 || iDec < 0) {
        result.errorMessage =
                QStringLiteral("CSV must contain RA and DEC columns "
                               "(decimal degrees or sexagesimal HH:MM:SS / DD:MM:SS). "
                               "Supported header variants: RA, RAJ2000, RA_(J2000)_Obs, …");
        return result;
    }

    int entryId = 0;
    while (!stream.atEnd()) {
        const QString line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith(u'#'))
            continue;

        const QStringList cols = detail::splitLine(line);
        if (cols.size() <= std::max(iRA, iDec)) {
            ++result.skippedEntries;
            continue;
        }

        double ra = 0.0, dec = 0.0;
        if (!detail::parseCoord(detail::colString(cols, iRA),  ra,  true) ||
            !detail::parseCoord(detail::colString(cols, iDec), dec, false)) {
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

        e.morphology = detail::colString(cols, iMorph).toLower().trimmed();
        if (e.morphology.isEmpty())
            e.morphology = QStringLiteral("other");

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
