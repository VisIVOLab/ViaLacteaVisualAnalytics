#ifndef PQMEMINSPECTOR_H
#define PQMEMINSPECTOR_H

#include <pqMemoryInspectorPanel.h>

#include <QDialog>

namespace Ui {
class pqMemInspector;
}

/**
 * @brief The pqMemInspector class
 * A container class to display the existing pqMemoryInspectorPanel widget.
 */
class pqMemInspector : public QDialog
{
    Q_OBJECT

public:
    explicit pqMemInspector(QWidget *parent = nullptr);
    ~pqMemInspector();

private:
    Ui::pqMemInspector *ui;
    pqMemoryInspectorPanel* memWidget;
};

#endif // PQMEMINSPECTOR_H
