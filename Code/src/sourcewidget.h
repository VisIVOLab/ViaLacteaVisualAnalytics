#ifndef SOURCEWIDGET_H
#define SOURCEWIDGET_H

#include <QPointer>
#include <QWidget>

class QListWidgetItem;
class Catalogue;
class Island;

namespace Ui {
class SourceWidget;
}

class SourceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SourceWidget(QWidget *parent, Catalogue *catalogue);
    ~SourceWidget();

public slots:
    void showIslandInfo(const Island *island);

private slots:
    void on_listExtracted_itemClicked(QListWidgetItem *item);

private:
    Ui::SourceWidget *ui;
    QPointer<Catalogue> catalogue;
};

#endif // SOURCEWIDGET_H
