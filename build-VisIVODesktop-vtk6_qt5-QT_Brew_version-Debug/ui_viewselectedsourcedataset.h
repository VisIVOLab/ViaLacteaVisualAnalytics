/********************************************************************************
** Form generated from reading UI file 'viewselectedsourcedataset.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VIEWSELECTEDSOURCEDATASET_H
#define UI_VIEWSELECTEDSOURCEDATASET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ViewSelectedSourceDataset
{
public:
    QVBoxLayout *verticalLayout_2;
    QTableWidget *datasetTableWidget;

    void setupUi(QWidget *ViewSelectedSourceDataset)
    {
        if (ViewSelectedSourceDataset->objectName().isEmpty())
            ViewSelectedSourceDataset->setObjectName(QString::fromUtf8("ViewSelectedSourceDataset"));
        ViewSelectedSourceDataset->resize(811, 246);
        verticalLayout_2 = new QVBoxLayout(ViewSelectedSourceDataset);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        datasetTableWidget = new QTableWidget(ViewSelectedSourceDataset);
        datasetTableWidget->setObjectName(QString::fromUtf8("datasetTableWidget"));

        verticalLayout_2->addWidget(datasetTableWidget);


        retranslateUi(ViewSelectedSourceDataset);

        QMetaObject::connectSlotsByName(ViewSelectedSourceDataset);
    } // setupUi

    void retranslateUi(QWidget *ViewSelectedSourceDataset)
    {
        ViewSelectedSourceDataset->setWindowTitle(QCoreApplication::translate("ViewSelectedSourceDataset", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ViewSelectedSourceDataset: public Ui_ViewSelectedSourceDataset {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VIEWSELECTEDSOURCEDATASET_H
