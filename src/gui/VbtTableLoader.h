#ifndef VbtTableLoader_h
#define VbtTableLoader_h

#include <QString>
#include <QStringList>
#include <QByteArray>

#include <array>
#include <cstdint>
#include <vector>

enum class VbtScalarType
{
    Float32,
    Float64,
};

enum class VbtEndian
{
    Little,
    Big,
};

enum class VbtDatasetKind
{
    Point,
    Volume,
};

struct VbtHeader
{
    QString headerPath;
    QString binaryPath;
    VbtScalarType scalarType{ VbtScalarType::Float32 };
    VbtEndian endian{ VbtEndian::Little };
    VbtDatasetKind kind{ VbtDatasetKind::Point };
    int fieldCount{ 0 };
    std::int64_t rowCount{ 0 };
    std::array<int, 3> dimensions{ { 0, 0, 0 } };
    std::array<double, 3> spacing{ { 1.0, 1.0, 1.0 } };
    QStringList fieldNames;
};

struct VbtTableData
{
    bool valid{ false };
    QString errorMessage;
    VbtHeader header;
    std::vector<std::vector<double>> columns;
    QByteArray rawBinary;
    int xIndex{ -1 };
    int yIndex{ -1 };
    int zIndex{ -1 };
};

namespace VbtTableLoader {

VbtTableData load(const QString &headerPath);
QString scalarTypeName(VbtScalarType scalarType);
QString endianName(VbtEndian endian);

} // namespace VbtTableLoader

#endif
