#ifndef SOURCEWIDGET_H
#define SOURCEWIDGET_H

#include <QWidget>

class Source;

namespace Ui {
class SourceWidget;
}

class SourceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SourceWidget(QWidget *parent = nullptr);
    ~SourceWidget();

public slots:
    void showSourceInfo(const Source *source);

private:
    Ui::SourceWidget *ui;
};

#endif // SOURCEWIDGET_H
