#ifndef CatalogueOverlayUtils_h
#define CatalogueOverlayUtils_h

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include <vector>

struct CatalogueOverlayEntry
{
    double raDeg{ 0.0 };
    double decDeg{ 0.0 };
    QString label;
};

struct CatalogueOverlayParseResult
{
    bool valid{ false };
    QString errorMessage;
    QString frameLabel;
    QString sourceLabel;
    int skippedEntries{ 0 };
    std::vector<CatalogueOverlayEntry> entries;
};

namespace CatalogueOverlayUtils {

inline QStringList splitSimpleCsvLine(const QString &line)
{
    return line.split(u',', Qt::KeepEmptyParts);
}

inline int findHeaderIndex(const QStringList &headers, const QStringList &aliases)
{
    for (int i = 0; i < headers.size(); ++i) {
        const QString normalized = headers.at(i).trimmed().toUpper();
        for (const QString &alias : aliases) {
            if (normalized == alias) {
                return i;
            }
        }
    }
    return -1;
}

inline CatalogueOverlayParseResult parseCsv(const QString &path)
{
    CatalogueOverlayParseResult result;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.errorMessage = QStringLiteral("Unable to open catalogue file.");
        return result;
    }

    QTextStream stream(&file);
    QString headerLine;
    while (!stream.atEnd() && headerLine.trimmed().isEmpty()) {
        headerLine = stream.readLine();
    }
    if (headerLine.trimmed().isEmpty()) {
        result.errorMessage = QStringLiteral("CSV catalogue is empty.");
        return result;
    }

    const QStringList headers = splitSimpleCsvLine(headerLine);
    const int raIndex = findHeaderIndex(headers, { QStringLiteral("RA") });
    const int decIndex = findHeaderIndex(headers, { QStringLiteral("DEC"), QStringLiteral("DEC_DEG") });
    const int labelIndex = findHeaderIndex(headers, { QStringLiteral("LABEL"), QStringLiteral("ID"),
                                                      QStringLiteral("NAME") });
    if (raIndex < 0 || decIndex < 0) {
        result.errorMessage =
                QStringLiteral("CSV catalogue requires RA and DEC header columns in degrees.");
        return result;
    }

    while (!stream.atEnd()) {
        const QString line = stream.readLine().trimmed();
        if (line.isEmpty()) {
            continue;
        }
        const QStringList cols = splitSimpleCsvLine(line);
        if (cols.size() <= std::max(raIndex, decIndex)) {
            ++result.skippedEntries;
            continue;
        }
        bool okRa = false;
        bool okDec = false;
        const double ra = cols.at(raIndex).trimmed().toDouble(&okRa);
        const double dec = cols.at(decIndex).trimmed().toDouble(&okDec);
        if (!okRa || !okDec) {
            ++result.skippedEntries;
            continue;
        }

        CatalogueOverlayEntry entry;
        entry.raDeg = ra;
        entry.decDeg = dec;
        if (labelIndex >= 0 && labelIndex < cols.size()) {
            entry.label = cols.at(labelIndex).trimmed();
        }
        result.entries.push_back(entry);
    }

    if (result.entries.empty()) {
        result.errorMessage = QStringLiteral("No valid RA/DEC rows were found in the CSV catalogue.");
        return result;
    }

    result.valid = true;
    result.frameLabel = QStringLiteral("FK5/ICRS (RA/DEC degrees)");
    result.sourceLabel = QFileInfo(path).fileName();
    return result;
}

inline CatalogueOverlayParseResult parseDs9(const QString &path)
{
    CatalogueOverlayParseResult result;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.errorMessage = QStringLiteral("Unable to open DS9 region file.");
        return result;
    }

    const QRegularExpression pointRe(
            QStringLiteral("^point\\s*\\(\\s*([^,\\)]+)\\s*,\\s*([^,\\)]+)\\s*\\)"),
            QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression circleRe(
            QStringLiteral("^circle\\s*\\(\\s*([^,\\)]+)\\s*,\\s*([^,\\)]+)\\s*,\\s*([^\\)]+)\\)"),
            QRegularExpression::CaseInsensitiveOption);
    const QRegularExpression textRe(QStringLiteral("text\\s*=\\s*\\{([^}]*)\\}"),
                                    QRegularExpression::CaseInsensitiveOption);

    QTextStream stream(&file);
    QString currentFrame = QStringLiteral("fk5");
    while (!stream.atEnd()) {
        const QString rawLine = stream.readLine().trimmed();
        if (rawLine.isEmpty() || rawLine.startsWith(u'#')) {
            continue;
        }

        QString line = rawLine.section(u'#', 0, 0).trimmed();
        if (line.isEmpty()) {
            continue;
        }

        const QStringList semicolonParts = line.split(u';', Qt::SkipEmptyParts);
        if (semicolonParts.size() >= 2) {
            const QString maybeFrame = semicolonParts.front().trimmed().toLower();
            if (maybeFrame == QStringLiteral("fk5") || maybeFrame == QStringLiteral("icrs")
                || maybeFrame == QStringLiteral("image") || maybeFrame == QStringLiteral("physical")
                || maybeFrame == QStringLiteral("galactic")) {
                currentFrame = maybeFrame;
                line = semicolonParts.mid(1).join(QStringLiteral(";")).trimmed();
            }
        }

        const QString lower = line.toLower();
        if (lower == QStringLiteral("fk5") || lower == QStringLiteral("icrs")) {
            currentFrame = lower;
            continue;
        }
        if (lower == QStringLiteral("global")) {
            continue;
        }
        if (lower == QStringLiteral("image") || lower == QStringLiteral("physical")
            || lower == QStringLiteral("galactic")) {
            currentFrame = lower;
            continue;
        }

        if (currentFrame != QStringLiteral("fk5") && currentFrame != QStringLiteral("icrs")) {
            ++result.skippedEntries;
            continue;
        }

        const auto pointMatch = pointRe.match(line);
        const auto circleMatch = circleRe.match(line);
        QString label;
        const auto textMatch = textRe.match(rawLine);
        if (textMatch.hasMatch()) {
            label = textMatch.captured(1).trimmed();
        }

        bool okRa = false;
        bool okDec = false;
        double ra = 0.0;
        double dec = 0.0;
        if (pointMatch.hasMatch()) {
            ra = pointMatch.captured(1).trimmed().toDouble(&okRa);
            dec = pointMatch.captured(2).trimmed().toDouble(&okDec);
        } else if (circleMatch.hasMatch()) {
            ra = circleMatch.captured(1).trimmed().toDouble(&okRa);
            dec = circleMatch.captured(2).trimmed().toDouble(&okDec);
        } else {
            ++result.skippedEntries;
            continue;
        }

        if (!okRa || !okDec) {
            ++result.skippedEntries;
            continue;
        }

        CatalogueOverlayEntry entry;
        entry.raDeg = ra;
        entry.decDeg = dec;
        entry.label = label;
        result.entries.push_back(entry);
    }

    if (result.entries.empty()) {
        result.errorMessage =
                QStringLiteral("No supported fk5/icrs point or circle entries were found in the DS9 file.");
        return result;
    }

    result.valid = true;
    result.frameLabel = QStringLiteral("DS9 fk5/icrs (RA/DEC degrees)");
    result.sourceLabel = QFileInfo(path).fileName();
    return result;
}

inline CatalogueOverlayParseResult parseFile(const QString &path)
{
    const QString suffix = QFileInfo(path).suffix().trimmed().toLower();
    if (suffix == QStringLiteral("csv")) {
        return parseCsv(path);
    }
    return parseDs9(path);
}

} // namespace CatalogueOverlayUtils

#endif
