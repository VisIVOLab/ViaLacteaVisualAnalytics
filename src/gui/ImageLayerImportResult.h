#ifndef ImageLayerImportResult_h
#define ImageLayerImportResult_h

#include <QString>

struct ImageLayerImportResult
{
    QString filepath;
    bool accepted;
    QString errorMessage;
};

#endif
