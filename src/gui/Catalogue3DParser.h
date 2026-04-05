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
#include <limits>
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
    double llsMajorKpc{ 0.0 };
    double llsMinorKpc{ 0.0 };
    double fluxMJy{ 0.0 };
    // 3D Cartesian scene coordinates (Mpc)
    double sceneX{ 0.0 };
    double sceneY{ 0.0 };
    double sceneZ{ 0.0 };
    QStringList rawFieldValues;
};

struct Catalogue3DParseResult
{
    bool valid{ false };
    QString errorMessage;
    int skippedEntries{ 0 };
    QStringList headers;
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

inline bool maybeNumeric(const QString &raw, double &value)
{
    const QString trimmed = raw.trimmed();
    if (trimmed.isEmpty()) {
        return false;
    }
    bool ok = false;
    const double parsed = trimmed.toDouble(&ok);
    if (!ok || !std::isfinite(parsed)) {
        return false;
    }
    value = parsed;
    return true;
}

inline QString colString(const QStringList &cols, int idx)
{
    if (idx < 0 || idx >= cols.size())
        return {};
    return cols.at(idx).trimmed();
}

inline void parseSizePair(const QString &raw, double &minorValue, double &majorValue)
{
    minorValue = 0.0;
    majorValue = 0.0;
    const QRegularExpression re(QStringLiteral("([+-]?\\d+(?:\\.\\d+)?)\\s*x\\s*([+-]?\\d+(?:\\.\\d+)?)"),
                                QRegularExpression::CaseInsensitiveOption);
    const auto match = re.match(raw);
    if (!match.hasMatch()) {
        return;
    }
    bool okMinor = false;
    bool okMajor = false;
    const double first = match.captured(1).toDouble(&okMinor);
    const double second = match.captured(2).toDouble(&okMajor);
    if (okMinor && okMajor) {
        minorValue = std::min(first, second);
        majorValue = std::max(first, second);
    }
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

// Comoving distance in flat ΛCDM (Planck18: H0=67.74, Ω_m=0.3089, Ω_Λ=0.6911).
// Uses a simple 1000-step trapezoidal integration of 1/E(z), where
// E(z) = sqrt(Ω_m·(1+z)³ + Ω_Λ). Accurate to <0.1% for z ≤ 10.
// The linear Hubble approximation (c/H0 · z) is only valid for z ≪ 1 and
// diverges by 40–60% at z~2 — making it wrong for Euclid-scale catalogues.
inline double comovingDistanceMpc(double z)
{
    if (z <= 0.0)
        return 0.0;
    constexpr double H0      = 67.74;    // km/s/Mpc (Planck18)
    constexpr double cKms    = 2.998e5;  // speed of light in km/s
    constexpr double DH      = cKms / H0; // Hubble distance in Mpc (~4426 Mpc)
    constexpr double Omega_m = 0.3089;
    constexpr double Omega_L = 0.6911;
    constexpr int    N       = 1000;     // trapezoidal steps

    auto E_inv = [&](double zp) -> double {
        const double factor = Omega_m * (1.0 + zp) * (1.0 + zp) * (1.0 + zp) + Omega_L;
        return (factor > 0.0) ? 1.0 / std::sqrt(factor) : 0.0;
    };

    const double dz = z / static_cast<double>(N);
    double integral = 0.5 * (E_inv(0.0) + E_inv(z));
    for (int i = 1; i < N; ++i)
        integral += E_inv(dz * static_cast<double>(i));
    integral *= dz;
    return DH * integral;
}

inline void fillCartesian(Catalogue3DEntry &e)
{
    if (e.distanceMpc <= 0.0 && e.redshift > 0.0)
        e.distanceMpc = comovingDistanceMpc(e.redshift);
    if (e.distanceMpc <= 0.0)
        e.distanceMpc = 300.0; // fixed-depth fallback for catalogues without z/d

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
    result.headers = headers;

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
        "FLUXDENSITYS128GHZTOTALMJY",
    });

    const int iMajorAxis = detail::findCol(headers, {
        "MAJORAXIS", "MAJAXIS", "BMAJ", "THETAMAJ", "MAJORAXISARCMIN", "MAJORAXISARCMIN"
    });
    const int iMinorAxis = detail::findCol(headers, {
        "MINORAXIS", "MINAXIS", "BMIN", "THETAMIN", "MINORAXISARCMIN", "MINORAXISARCmin"
    });
    const int iLls = detail::findCol(headers, {
        "LLS", "LLSKPC", "SIZE", "SIZELLSAXISMINXMAJKPCXKPC", "SIZEFULLEXTENT", "LARGESTLINEARSIZE"
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
        e.rawFieldValues = cols;
        while (e.rawFieldValues.size() < headers.size()) {
            e.rawFieldValues.append(QString());
        }

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
        if (iMajorAxis >= 0)
            e.majorAxisArcmin = detail::colDouble(cols, iMajorAxis);
        if (iMinorAxis >= 0)
            e.minorAxisArcmin = detail::colDouble(cols, iMinorAxis);
        if (iLls >= 0) {
            detail::parseSizePair(detail::colString(cols, iLls), e.llsMinorKpc, e.llsMajorKpc);
        }

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
