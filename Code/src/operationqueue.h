#ifndef OPERATIONQUEUE_H
#define OPERATIONQUEUE_H

#include <QWidget>

namespace Ui {
class OperationQueue;
}

class OperationQueue : public QWidget
{
    Q_OBJECT

public:
    explicit OperationQueue(QWidget *parent = 0);
    ~OperationQueue();
    int addOperation(QString name);
    void editOperation(int id, QString status);


private:
    Ui::OperationQueue *ui;

};

#endif // OPERATIONQUEUE_H
