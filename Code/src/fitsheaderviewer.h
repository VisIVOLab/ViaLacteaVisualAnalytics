#ifndef FITSHEADERVIEWER_H
#define FITSHEADERVIEWER_H

#include <QWidget>

namespace Ui {
class FitsHeaderViewer;
}

class FitsHeaderViewer : public QWidget
{
    Q_OBJECT

public:
    explicit FitsHeaderViewer(QWidget *parent = nullptr);
    ~FitsHeaderViewer();

    void showHeader(const QString &filepath);

private:
    Ui::FitsHeaderViewer *ui;

    QString highlightKeyword(QString card) const;
};

#endif // FITSHEADERVIEWER_H
