/********************************************************************************
** Form generated from reading UI file 'vialacteainitialquery.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VIALACTEAINITIALQUERY_H
#define UI_VIALACTEAINITIALQUERY_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_VialacteaInitialQuery
{
public:
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QGroupBox *selectioTypeGroupBox;
    QHBoxLayout *horizontalLayout_5;
    QRadioButton *pointRadioButton;
    QRadioButton *rectRadioButton;
    QSpacerItem *horizontalSpacer;
    QGroupBox *pointGroupBox;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_2;
    QLineEdit *l_lineEdit;
    QLabel *label_3;
    QLineEdit *b_lineEdit;
    QLabel *label;
    QLineEdit *r_lineEdit;
    QGroupBox *rectGroupBox;
    QHBoxLayout *horizontalLayout_7;
    QVBoxLayout *boxQueryLayout;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_7;
    QLineEdit *dlLineEdit;
    QLabel *label_6;
    QLineEdit *dbLineEdit;
    QHBoxLayout *horizontalLayout_2;
    QHBoxLayout *horizontalLayout_4;
    QPushButton *pushButton;
    QPushButton *queryPushButton;

    void setupUi(QWidget *VialacteaInitialQuery)
    {
        if (VialacteaInitialQuery->objectName().isEmpty())
            VialacteaInitialQuery->setObjectName(QString::fromUtf8("VialacteaInitialQuery"));
        VialacteaInitialQuery->resize(652, 312);
        verticalLayout_2 = new QVBoxLayout(VialacteaInitialQuery);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        selectioTypeGroupBox = new QGroupBox(VialacteaInitialQuery);
        selectioTypeGroupBox->setObjectName(QString::fromUtf8("selectioTypeGroupBox"));
        horizontalLayout_5 = new QHBoxLayout(selectioTypeGroupBox);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        pointRadioButton = new QRadioButton(selectioTypeGroupBox);
        pointRadioButton->setObjectName(QString::fromUtf8("pointRadioButton"));
        pointRadioButton->setChecked(true);

        horizontalLayout_5->addWidget(pointRadioButton);

        rectRadioButton = new QRadioButton(selectioTypeGroupBox);
        rectRadioButton->setObjectName(QString::fromUtf8("rectRadioButton"));
        rectRadioButton->setEnabled(false);

        horizontalLayout_5->addWidget(rectRadioButton);


        horizontalLayout->addWidget(selectioTypeGroupBox);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout_2->addLayout(horizontalLayout);

        pointGroupBox = new QGroupBox(VialacteaInitialQuery);
        pointGroupBox->setObjectName(QString::fromUtf8("pointGroupBox"));
        horizontalLayout_6 = new QHBoxLayout(pointGroupBox);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_2 = new QLabel(pointGroupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_6->addWidget(label_2);

        l_lineEdit = new QLineEdit(pointGroupBox);
        l_lineEdit->setObjectName(QString::fromUtf8("l_lineEdit"));

        horizontalLayout_6->addWidget(l_lineEdit);

        label_3 = new QLabel(pointGroupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_6->addWidget(label_3);

        b_lineEdit = new QLineEdit(pointGroupBox);
        b_lineEdit->setObjectName(QString::fromUtf8("b_lineEdit"));

        horizontalLayout_6->addWidget(b_lineEdit);

        label = new QLabel(pointGroupBox);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_6->addWidget(label);

        r_lineEdit = new QLineEdit(pointGroupBox);
        r_lineEdit->setObjectName(QString::fromUtf8("r_lineEdit"));

        horizontalLayout_6->addWidget(r_lineEdit);


        verticalLayout_2->addWidget(pointGroupBox);

        rectGroupBox = new QGroupBox(VialacteaInitialQuery);
        rectGroupBox->setObjectName(QString::fromUtf8("rectGroupBox"));
        horizontalLayout_7 = new QHBoxLayout(rectGroupBox);
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        boxQueryLayout = new QVBoxLayout();
        boxQueryLayout->setObjectName(QString::fromUtf8("boxQueryLayout"));
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_7 = new QLabel(rectGroupBox);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        horizontalLayout_3->addWidget(label_7);

        dlLineEdit = new QLineEdit(rectGroupBox);
        dlLineEdit->setObjectName(QString::fromUtf8("dlLineEdit"));

        horizontalLayout_3->addWidget(dlLineEdit);

        label_6 = new QLabel(rectGroupBox);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        horizontalLayout_3->addWidget(label_6);

        dbLineEdit = new QLineEdit(rectGroupBox);
        dbLineEdit->setObjectName(QString::fromUtf8("dbLineEdit"));

        horizontalLayout_3->addWidget(dbLineEdit);


        boxQueryLayout->addLayout(horizontalLayout_3);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));

        boxQueryLayout->addLayout(horizontalLayout_2);


        horizontalLayout_7->addLayout(boxQueryLayout);


        verticalLayout_2->addWidget(rectGroupBox);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        pushButton = new QPushButton(VialacteaInitialQuery);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        horizontalLayout_4->addWidget(pushButton);

        queryPushButton = new QPushButton(VialacteaInitialQuery);
        queryPushButton->setObjectName(QString::fromUtf8("queryPushButton"));

        horizontalLayout_4->addWidget(queryPushButton);


        verticalLayout_2->addLayout(horizontalLayout_4);


        retranslateUi(VialacteaInitialQuery);

        QMetaObject::connectSlotsByName(VialacteaInitialQuery);
    } // setupUi

    void retranslateUi(QWidget *VialacteaInitialQuery)
    {
        VialacteaInitialQuery->setWindowTitle(QCoreApplication::translate("VialacteaInitialQuery", "Form", nullptr));
        selectioTypeGroupBox->setTitle(QCoreApplication::translate("VialacteaInitialQuery", "Selection type", nullptr));
        pointRadioButton->setText(QCoreApplication::translate("VialacteaInitialQuery", "Point", nullptr));
        rectRadioButton->setText(QCoreApplication::translate("VialacteaInitialQuery", "Rectangular", nullptr));
        pointGroupBox->setTitle(QCoreApplication::translate("VialacteaInitialQuery", "Point selection", nullptr));
        label_2->setText(QCoreApplication::translate("VialacteaInitialQuery", "glon", nullptr));
        l_lineEdit->setText(QCoreApplication::translate("VialacteaInitialQuery", "0", nullptr));
        label_3->setText(QCoreApplication::translate("VialacteaInitialQuery", "glat", nullptr));
        b_lineEdit->setText(QCoreApplication::translate("VialacteaInitialQuery", "0", nullptr));
        label->setText(QCoreApplication::translate("VialacteaInitialQuery", "r [deg]", nullptr));
        r_lineEdit->setText(QCoreApplication::translate("VialacteaInitialQuery", "0.1", nullptr));
        rectGroupBox->setTitle(QCoreApplication::translate("VialacteaInitialQuery", "Rectangular  selection", nullptr));
        label_7->setText(QCoreApplication::translate("VialacteaInitialQuery", "dl", nullptr));
        label_6->setText(QCoreApplication::translate("VialacteaInitialQuery", "db", nullptr));
        pushButton->setText(QCoreApplication::translate("VialacteaInitialQuery", "Cancel", nullptr));
        queryPushButton->setText(QCoreApplication::translate("VialacteaInitialQuery", "Query", nullptr));
    } // retranslateUi

};

namespace Ui {
    class VialacteaInitialQuery: public Ui_VialacteaInitialQuery {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VIALACTEAINITIALQUERY_H
