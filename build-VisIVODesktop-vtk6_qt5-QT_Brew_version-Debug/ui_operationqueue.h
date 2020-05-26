/********************************************************************************
** Form generated from reading UI file 'operationqueue.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_OPERATIONQUEUE_H
#define UI_OPERATIONQUEUE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_OperationQueue
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton_2;
    QPushButton *pushButton;
    QTableWidget *tableWidget;

    void setupUi(QWidget *OperationQueue)
    {
        if (OperationQueue->objectName().isEmpty())
            OperationQueue->setObjectName(QString::fromUtf8("OperationQueue"));
        OperationQueue->resize(685, 551);
        verticalLayout = new QVBoxLayout(OperationQueue);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        pushButton_2 = new QPushButton(OperationQueue);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));
        pushButton_2->setEnabled(false);

        horizontalLayout->addWidget(pushButton_2);

        pushButton = new QPushButton(OperationQueue);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setEnabled(false);

        horizontalLayout->addWidget(pushButton);


        verticalLayout->addLayout(horizontalLayout);

        tableWidget = new QTableWidget(OperationQueue);
        if (tableWidget->columnCount() < 3)
            tableWidget->setColumnCount(3);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        tableWidget->setObjectName(QString::fromUtf8("tableWidget"));
        tableWidget->horizontalHeader()->setVisible(true);
        tableWidget->horizontalHeader()->setCascadingSectionResizes(false);

        verticalLayout->addWidget(tableWidget);


        retranslateUi(OperationQueue);

        QMetaObject::connectSlotsByName(OperationQueue);
    } // setupUi

    void retranslateUi(QWidget *OperationQueue)
    {
        OperationQueue->setWindowTitle(QCoreApplication::translate("OperationQueue", "Queue", nullptr));
        pushButton_2->setText(QCoreApplication::translate("OperationQueue", "Refresh", nullptr));
        pushButton->setText(QCoreApplication::translate("OperationQueue", "Clear", nullptr));
        QTableWidgetItem *___qtablewidgetitem = tableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("OperationQueue", "Id", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tableWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("OperationQueue", "Name", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = tableWidget->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("OperationQueue", "Status", nullptr));
    } // retranslateUi

};

namespace Ui {
    class OperationQueue: public Ui_OperationQueue {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_OPERATIONQUEUE_H
