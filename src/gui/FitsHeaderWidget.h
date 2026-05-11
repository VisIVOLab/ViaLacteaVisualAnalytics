#ifndef FitsHeaderWidget_h
#define FitsHeaderWidget_h

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class FitsHeaderWidget;
}
QT_END_NAMESPACE

class FitsHeaderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FitsHeaderWidget(QWidget *parent = nullptr);
    ~FitsHeaderWidget() override;

public slots:
    void showHeader(const QString &filepath);

private:
    Ui::FitsHeaderWidget *ui;

    [[nodiscard]]
    static QString highlightKeyword(const QString &card, const QColor &color = Qt::blue);
};

#endif
