#ifndef DatasetWindowFactory_h
#define DatasetWindowFactory_h

#include "DatasetOpenTypes.h"

class QWidget;

class DatasetWindowFactory
{
public:
    QWidget *createWindow(const DatasetOpenInfo &dataset, QWidget *parent = nullptr) const;
};

#endif
