#ifndef DatasetOpenService_h
#define DatasetOpenService_h

#include "DatasetOpenTypes.h"

class DatasetOpenService
{
public:
    DatasetOpenService() = default;

    DatasetOpenInfo inspect(const QString &filepath) const;
};

#endif
