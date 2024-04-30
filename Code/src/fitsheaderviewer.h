#ifndef FITSHEADERVIEWER_H
#define FITSHEADERVIEWER_H

#include <QWidget>
#include "fitsio.h"

namespace Ui {
class FitsHeaderViewer;
}

class FitsHeaderViewer : public QWidget
{
    Q_OBJECT

public:
    explicit     FitsHeaderViewer(const QString &fitsFilePath, QWidget *parent = nullptr);

    ~FitsHeaderViewer();

private:
    Ui::FitsHeaderViewer *ui;
};

#endif // FITSHEADERVIEWER_H
