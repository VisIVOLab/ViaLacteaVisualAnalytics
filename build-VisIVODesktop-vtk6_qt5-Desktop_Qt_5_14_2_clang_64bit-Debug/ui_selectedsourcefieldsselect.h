/********************************************************************************
** Form generated from reading UI file 'selectedsourcefieldsselect.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SELECTEDSOURCEFIELDSSELECT_H
#define UI_SELECTEDSOURCEFIELDSSELECT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_selectedSourceFieldsSelect
{
public:
    QVBoxLayout *verticalLayout_2;
    QTableWidget *fieldListTableWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *selectAllButton;
    QPushButton *deselectAllButton;
    QPushButton *okButton;

    void setupUi(QWidget *selectedSourceFieldsSelect)
    {
        if (selectedSourceFieldsSelect->objectName().isEmpty())
            selectedSourceFieldsSelect->setObjectName(QString::fromUtf8("selectedSourceFieldsSelect"));
        selectedSourceFieldsSelect->resize(498, 490);
        verticalLayout_2 = new QVBoxLayout(selectedSourceFieldsSelect);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        fieldListTableWidget = new QTableWidget(selectedSourceFieldsSelect);
        if (fieldListTableWidget->columnCount() < 2)
            fieldListTableWidget->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        fieldListTableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        fieldListTableWidget->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        fieldListTableWidget->setObjectName(QString::fromUtf8("fieldListTableWidget"));

        verticalLayout_2->addWidget(fieldListTableWidget);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        selectAllButton = new QPushButton(selectedSourceFieldsSelect);
        selectAllButton->setObjectName(QString::fromUtf8("selectAllButton"));

        horizontalLayout->addWidget(selectAllButton);

        deselectAllButton = new QPushButton(selectedSourceFieldsSelect);
        deselectAllButton->setObjectName(QString::fromUtf8("deselectAllButton"));

        horizontalLayout->addWidget(deselectAllButton);


        verticalLayout_2->addLayout(horizontalLayout);

        okButton = new QPushButton(selectedSourceFieldsSelect);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        verticalLayout_2->addWidget(okButton);


        retranslateUi(selectedSourceFieldsSelect);

        QMetaObject::connectSlotsByName(selectedSourceFieldsSelect);
    } // setupUi

    void retranslateUi(QWidget *selectedSourceFieldsSelect)
    {
        selectedSourceFieldsSelect->setWindowTitle(QCoreApplication::translate("selectedSourceFieldsSelect", "Form", nullptr));
        QTableWidgetItem *___qtablewidgetitem = fieldListTableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("selectedSourceFieldsSelect", "Selected", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = fieldListTableWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("selectedSourceFieldsSelect", "Column", nullptr));
        selectAllButton->setText(QCoreApplication::translate("selectedSourceFieldsSelect", "Select all", nullptr));
        deselectAllButton->setText(QCoreApplication::translate("selectedSourceFieldsSelect", "Deselect all", nullptr));
        okButton->setText(QCoreApplication::translate("selectedSourceFieldsSelect", "Ok", nullptr));
    } // retranslateUi

};

namespace Ui {
    class selectedSourceFieldsSelect: public Ui_selectedSourceFieldsSelect {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SELECTEDSOURCEFIELDSSELECT_H
