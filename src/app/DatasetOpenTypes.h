#ifndef DatasetOpenTypes_h
#define DatasetOpenTypes_h

#include <QString>

enum class DatasetKind { Unknown, Image, Cube };

struct DatasetOpenInfo
{
    QString filepath;
    DatasetKind kind{ DatasetKind::Unknown };
    QString errorMessage;
    int activeAxes{ 0 };
    QString degenerateAxesSummary;

    bool isValid() const
    {
        return !this->filepath.isEmpty() && this->kind != DatasetKind::Unknown
                && this->errorMessage.isEmpty();
    }
};

#endif
