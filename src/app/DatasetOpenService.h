#ifndef DatasetOpenService_h
#define DatasetOpenService_h

#include "DatasetOpenRequest.h"
#include "DatasetOpenTypes.h"

class DatasetOpenService
{
public:
    DatasetOpenService() = default;

    // Inspects a local dataset request and classifies the FITS resource for the desktop client.
    DatasetOpenInfo inspect(const DatasetOpenRequest &request) const;
};

#endif
