#ifndef DatasetOpenService_h
#define DatasetOpenService_h

#include "DatasetOpenRequest.h"
#include "DatasetOpenTypes.h"

class DatasetOpenService
{
public:
    DatasetOpenService() = default;

    DatasetOpenInfo inspect(const DatasetOpenRequest &request) const;
};

#endif
