/********************************************************************************
** Form generated from reading UI file 'vlkbquerycomposer.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VLKBQUERYCOMPOSER_H
#define UI_VLKBQUERYCOMPOSER_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_VLKBQueryComposer
{
public:
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *tapUrlLineEdit;
    QPushButton *connectButton;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_2;
    QComboBox *tableListComboBox;
    QTableWidget *columnTableWidget;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *syncCheckBox;
    QLabel *label_3;
    QComboBox *queryLangComboBox;
    QTextEdit *queryTextEdit;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QPushButton *okButton;
    QSpacerItem *horizontalSpacer_2;

    void setupUi(QWidget *VLKBQueryComposer)
    {
        if (VLKBQueryComposer->objectName().isEmpty())
            VLKBQueryComposer->setObjectName(QString::fromUtf8("VLKBQueryComposer"));
        VLKBQueryComposer->resize(733, 461);
        verticalLayout_2 = new QVBoxLayout(VLKBQueryComposer);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(VLKBQueryComposer);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        tapUrlLineEdit = new QLineEdit(VLKBQueryComposer);
        tapUrlLineEdit->setObjectName(QString::fromUtf8("tapUrlLineEdit"));
        tapUrlLineEdit->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(tapUrlLineEdit);

        connectButton = new QPushButton(VLKBQueryComposer);
        connectButton->setObjectName(QString::fromUtf8("connectButton"));

        horizontalLayout->addWidget(connectButton);


        verticalLayout_2->addLayout(horizontalLayout);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_2 = new QLabel(VLKBQueryComposer);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_4->addWidget(label_2);

        tableListComboBox = new QComboBox(VLKBQueryComposer);
        tableListComboBox->setObjectName(QString::fromUtf8("tableListComboBox"));
        tableListComboBox->setEnabled(false);

        horizontalLayout_4->addWidget(tableListComboBox);


        verticalLayout_2->addLayout(horizontalLayout_4);

        columnTableWidget = new QTableWidget(VLKBQueryComposer);
        if (columnTableWidget->columnCount() < 2)
            columnTableWidget->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        columnTableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        columnTableWidget->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        columnTableWidget->setObjectName(QString::fromUtf8("columnTableWidget"));
        columnTableWidget->setEnabled(false);

        verticalLayout_2->addWidget(columnTableWidget);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        syncCheckBox = new QCheckBox(VLKBQueryComposer);
        syncCheckBox->setObjectName(QString::fromUtf8("syncCheckBox"));
        syncCheckBox->setEnabled(false);
        syncCheckBox->setChecked(true);

        horizontalLayout_3->addWidget(syncCheckBox);

        label_3 = new QLabel(VLKBQueryComposer);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout_3->addWidget(label_3);

        queryLangComboBox = new QComboBox(VLKBQueryComposer);
        queryLangComboBox->addItem(QString());
        queryLangComboBox->setObjectName(QString::fromUtf8("queryLangComboBox"));
        queryLangComboBox->setEnabled(false);

        horizontalLayout_3->addWidget(queryLangComboBox);


        verticalLayout_2->addLayout(horizontalLayout_3);

        queryTextEdit = new QTextEdit(VLKBQueryComposer);
        queryTextEdit->setObjectName(QString::fromUtf8("queryTextEdit"));
        queryTextEdit->setEnabled(false);

        verticalLayout_2->addWidget(queryTextEdit);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        okButton = new QPushButton(VLKBQueryComposer);
        okButton->setObjectName(QString::fromUtf8("okButton"));
        okButton->setEnabled(false);

        horizontalLayout_2->addWidget(okButton);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);


        verticalLayout_2->addLayout(horizontalLayout_2);


        retranslateUi(VLKBQueryComposer);

        QMetaObject::connectSlotsByName(VLKBQueryComposer);
    } // setupUi

    void retranslateUi(QWidget *VLKBQueryComposer)
    {
        VLKBQueryComposer->setWindowTitle(QCoreApplication::translate("VLKBQueryComposer", "Form", nullptr));
        label->setText(QCoreApplication::translate("VLKBQueryComposer", "TAP URL:", nullptr));
        tapUrlLineEdit->setText(QCoreApplication::translate("VLKBQueryComposer", "http://palantir19.oats.inaf.it:8080/vlkb/", nullptr));
        connectButton->setText(QCoreApplication::translate("VLKBQueryComposer", "Connect", nullptr));
        label_2->setText(QCoreApplication::translate("VLKBQueryComposer", "Table:", nullptr));
        QTableWidgetItem *___qtablewidgetitem = columnTableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("VLKBQueryComposer", "Name", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = columnTableWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("VLKBQueryComposer", "DataType", nullptr));
        syncCheckBox->setText(QCoreApplication::translate("VLKBQueryComposer", "Synchronous", nullptr));
        label_3->setText(QCoreApplication::translate("VLKBQueryComposer", "Query Languages", nullptr));
        queryLangComboBox->setItemText(0, QCoreApplication::translate("VLKBQueryComposer", "ADQL", nullptr));

        okButton->setText(QCoreApplication::translate("VLKBQueryComposer", "Ok", nullptr));
    } // retranslateUi

};

namespace Ui {
    class VLKBQueryComposer: public Ui_VLKBQueryComposer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VLKBQUERYCOMPOSER_H
