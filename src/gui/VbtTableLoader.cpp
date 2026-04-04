#include "VbtTableLoader.h"

#include <QFile>
#include <QFileInfo>
#include <QSysInfo>
#include <QTextStream>

#include <algorithm>
#include <array>
#include <cstring>

namespace {

template <typename T>
T maybeSwapValue(const char *ptr, bool swapBytes)
{
    std::array<unsigned char, sizeof(T)> bytes{};
    std::memcpy(bytes.data(), ptr, sizeof(T));
    if (swapBytes) {
        std::reverse(bytes.begin(), bytes.end());
    }
    T value{};
    std::memcpy(&value, bytes.data(), sizeof(T));
    return value;
}

QString normalizedFieldName(const QString &fieldName)
{
    return fieldName.trimmed().toLower();
}

} // namespace

namespace VbtTableLoader {

QString scalarTypeName(VbtScalarType scalarType)
{
    return scalarType == VbtScalarType::Float64 ? QStringLiteral("float64")
                                                : QStringLiteral("float32");
}

QString endianName(VbtEndian endian)
{
    return endian == VbtEndian::Big ? QStringLiteral("big") : QStringLiteral("little");
}

VbtTableData load(const QString &headerPath)
{
    VbtTableData result;
    result.header.headerPath = headerPath;

    QFile headerFile(headerPath);
    if (!headerFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.errorMessage = QStringLiteral("Could not open VBT header file.");
        return result;
    }

    QTextStream stream(&headerFile);
    QStringList lines;
    while (!stream.atEnd()) {
        const QString line = stream.readLine().trimmed();
        if (!line.isEmpty()) {
            lines.append(line);
        }
    }

    if (lines.size() < 4) {
        result.errorMessage = QStringLiteral("Malformed VBT header: expected at least 4 lines.");
        return result;
    }

    const QString typeLine = lines.at(0).toLower();
    if (typeLine == QStringLiteral("f")) {
        result.header.scalarType = VbtScalarType::Float32;
    } else if (typeLine == QStringLiteral("d")) {
        result.header.scalarType = VbtScalarType::Float64;
    } else {
        result.errorMessage = QStringLiteral("Unsupported VBT scalar type: %1").arg(lines.at(0));
        return result;
    }

    bool okFieldCount = false;
    const int fieldCount = lines.at(1).toInt(&okFieldCount);
    bool okRowCount = false;
    const qlonglong rowCount = lines.at(2).toLongLong(&okRowCount);
    if (!okFieldCount || fieldCount <= 0 || !okRowCount || rowCount <= 0) {
        result.errorMessage = QStringLiteral("Malformed VBT header: invalid field or row count.");
        return result;
    }
    result.header.fieldCount = fieldCount;
    result.header.rowCount = rowCount;

    const QString endianLine = lines.at(3).toLower();
    if (endianLine == QStringLiteral("l")) {
        result.header.endian = VbtEndian::Little;
    } else if (endianLine == QStringLiteral("b")) {
        result.header.endian = VbtEndian::Big;
    } else {
        result.errorMessage = QStringLiteral("Unsupported VBT endian flag: %1").arg(lines.at(3));
        return result;
    }

    if (lines.size() != 4 + fieldCount) {
        result.errorMessage = QStringLiteral("Malformed VBT header: field count does not match field-name list.");
        return result;
    }

    for (int i = 0; i < fieldCount; ++i) {
        result.header.fieldNames.append(lines.at(4 + i));
    }

    QString binaryPath = headerPath;
    if (binaryPath.endsWith(QStringLiteral(".head"), Qt::CaseInsensitive)) {
        binaryPath.chop(5);
    } else {
        result.errorMessage = QStringLiteral("VBT header file must end with .head");
        return result;
    }
    result.header.binaryPath = binaryPath;

    QFileInfo binaryInfo(binaryPath);
    if (!binaryInfo.exists()) {
        result.errorMessage = QStringLiteral("Matching VBT binary file not found: %1").arg(binaryPath);
        return result;
    }

    QFile binaryFile(binaryPath);
    if (!binaryFile.open(QIODevice::ReadOnly)) {
        result.errorMessage = QStringLiteral("Could not open VBT binary file.");
        return result;
    }

    const qsizetype scalarSize =
            result.header.scalarType == VbtScalarType::Float64 ? sizeof(double) : sizeof(float);
    const qint64 expectedSize =
            static_cast<qint64>(result.header.rowCount) * result.header.fieldCount * scalarSize;
    if (binaryFile.size() != expectedSize) {
        result.errorMessage = QStringLiteral("VBT binary size mismatch. Expected %1 bytes, found %2 bytes.")
                                      .arg(expectedSize)
                                      .arg(binaryFile.size());
        return result;
    }

    const QByteArray raw = binaryFile.readAll();
    if (raw.size() != expectedSize) {
        result.errorMessage = QStringLiteral("Failed to read complete VBT binary payload.");
        return result;
    }

    const bool hostLittleEndian = QSysInfo::ByteOrder == QSysInfo::LittleEndian;
    const bool fileLittleEndian = result.header.endian == VbtEndian::Little;
    const bool swapBytes = hostLittleEndian != fileLittleEndian;

    result.columns.assign(static_cast<std::size_t>(fieldCount),
                          std::vector<double>(static_cast<std::size_t>(rowCount), 0.0));

    const char *ptr = raw.constData();
    for (std::int64_t row = 0; row < result.header.rowCount; ++row) {
        for (int field = 0; field < result.header.fieldCount; ++field) {
            double value = 0.0;
            if (result.header.scalarType == VbtScalarType::Float64) {
                value = maybeSwapValue<double>(ptr, swapBytes);
            } else {
                value = maybeSwapValue<float>(ptr, swapBytes);
            }
            result.columns[static_cast<std::size_t>(field)][static_cast<std::size_t>(row)] = value;
            ptr += scalarSize;
        }
    }

    for (int field = 0; field < result.header.fieldNames.size(); ++field) {
        const QString name = normalizedFieldName(result.header.fieldNames.at(field));
        if (name == QStringLiteral("x")) {
            result.xIndex = field;
        } else if (name == QStringLiteral("y")) {
            result.yIndex = field;
        } else if (name == QStringLiteral("z")) {
            result.zIndex = field;
        }
    }

    if (result.xIndex < 0 || result.yIndex < 0 || result.zIndex < 0) {
        result.errorMessage = QStringLiteral(
                "This first VBT viewer version only supports point tables with X, Y, Z columns.");
        return result;
    }

    result.valid = true;
    return result;
}

} // namespace VbtTableLoader
