#ifndef DatasetOpenRequest_h
#define DatasetOpenRequest_h

#include <QString>

struct DatasetOpenRequest
{
    // Current implementation expects a local desktop FITS file path.
    QString filepath;
};

#endif
