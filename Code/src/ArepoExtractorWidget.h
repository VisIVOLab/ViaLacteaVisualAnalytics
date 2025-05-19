#ifndef AREPOEXTRACTORWIDGET_H
#define AREPOEXTRACTORWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class ArepoExtractorWidget;
}
QT_END_NAMESPACE

namespace H5 {
class Group;
class H5Object;
}

class QTreeWidgetItem;

class ArepoExtractorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ArepoExtractorWidget(const QString &filepath, QWidget *parent = nullptr);
    ~ArepoExtractorWidget() override;

private slots:
    void updatePhysicalResolution();
    void extract();

private:
    Ui::ArepoExtractorWidget *ui;
    const QString filepath;

    void loadSimulation();

    void buildTreeItem(const H5::Group &group, QTreeWidgetItem *parent);
    void buildTreeItemAttributes(const H5::H5Object &obj, QTreeWidgetItem *parent);

    void openSimulation(const QString &filepath);
};

#endif // AREPOEXTRACTORWIDGET_H
