/********************************************************************************
** Form generated from reading UI file 'filtercustomize.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FILTERCUSTOMIZE_H
#define UI_FILTERCUSTOMIZE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FilterCustomize
{
public:
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QComboBox *fieldComboBox;
    QLineEdit *minValueLineEdit;
    QLineEdit *maxValueLineEdit;
    QPushButton *addPushButton;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QTableWidget *addedFilterTableWidget;

    void setupUi(QWidget *FilterCustomize)
    {
        if (FilterCustomize->objectName().isEmpty())
            FilterCustomize->setObjectName(QString::fromUtf8("FilterCustomize"));
        FilterCustomize->resize(479, 453);
        verticalLayout = new QVBoxLayout(FilterCustomize);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        fieldComboBox = new QComboBox(FilterCustomize);
        fieldComboBox->setObjectName(QString::fromUtf8("fieldComboBox"));

        gridLayout->addWidget(fieldComboBox, 1, 0, 1, 1);

        minValueLineEdit = new QLineEdit(FilterCustomize);
        minValueLineEdit->setObjectName(QString::fromUtf8("minValueLineEdit"));

        gridLayout->addWidget(minValueLineEdit, 1, 1, 1, 1);

        maxValueLineEdit = new QLineEdit(FilterCustomize);
        maxValueLineEdit->setObjectName(QString::fromUtf8("maxValueLineEdit"));

        gridLayout->addWidget(maxValueLineEdit, 1, 2, 1, 1);

        addPushButton = new QPushButton(FilterCustomize);
        addPushButton->setObjectName(QString::fromUtf8("addPushButton"));

        gridLayout->addWidget(addPushButton, 1, 3, 1, 1);

        label = new QLabel(FilterCustomize);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        label_2 = new QLabel(FilterCustomize);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 0, 1, 1, 1);

        label_3 = new QLabel(FilterCustomize);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 0, 2, 1, 1);


        verticalLayout->addLayout(gridLayout);

        addedFilterTableWidget = new QTableWidget(FilterCustomize);
        if (addedFilterTableWidget->columnCount() < 4)
            addedFilterTableWidget->setColumnCount(4);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        addedFilterTableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        addedFilterTableWidget->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        addedFilterTableWidget->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        addedFilterTableWidget->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        addedFilterTableWidget->setObjectName(QString::fromUtf8("addedFilterTableWidget"));

        verticalLayout->addWidget(addedFilterTableWidget);


        retranslateUi(FilterCustomize);

        QMetaObject::connectSlotsByName(FilterCustomize);
    } // setupUi

    void retranslateUi(QWidget *FilterCustomize)
    {
        FilterCustomize->setWindowTitle(QCoreApplication::translate("FilterCustomize", "Form", nullptr));
        addPushButton->setText(QCoreApplication::translate("FilterCustomize", "Add", nullptr));
        label->setText(QCoreApplication::translate("FilterCustomize", "Field", nullptr));
        label_2->setText(QCoreApplication::translate("FilterCustomize", "Min Value", nullptr));
        label_3->setText(QCoreApplication::translate("FilterCustomize", "Max Value", nullptr));
        QTableWidgetItem *___qtablewidgetitem = addedFilterTableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("FilterCustomize", "Field", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = addedFilterTableWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("FilterCustomize", "Min Value", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = addedFilterTableWidget->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("FilterCustomize", "Max Value", nullptr));
    } // retranslateUi

};

namespace Ui {
    class FilterCustomize: public Ui_FilterCustomize {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FILTERCUSTOMIZE_H
