#ifndef VbtTableLoader_h
#define VbtTableLoader_h

#include <QString>
#include <QStringList>

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

struct VbtHeader
{
    QString headerPath;
    QString binaryPath;
    VbtScalarType scalarType{ VbtScalarType::Float32 };
    VbtEndian endian{ VbtEndian::Little };
    int fieldCount{ 0 };
    std::int64_t rowCount{ 0 };
    QStringList fieldNames;
};

struct VbtTableData
{
    bool valid{ false };
    QString errorMessage;
    VbtHeader header;
    std::vector<std::vector<double>> columns;
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
