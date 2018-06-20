#ifndef SELECTEDSOURCESFORM_H
#define SELECTEDSOURCESFORM_H

#include <QWidget>

namespace Ui {
class SelectedSourcesForm;
}

class SelectedSourcesForm : public QWidget
{
    Q_OBJECT

public:
    explicit SelectedSourcesForm(QWidget *parent = 0);
    ~SelectedSourcesForm();

private:
    Ui::SelectedSourcesForm *ui;
};

#endif // SELECTEDSOURCESFORM_H
