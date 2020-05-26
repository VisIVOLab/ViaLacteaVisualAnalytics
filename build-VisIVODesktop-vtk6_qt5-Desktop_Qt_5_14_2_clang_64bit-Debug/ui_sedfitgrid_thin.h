/********************************************************************************
** Form generated from reading UI file 'sedfitgrid_thin.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SEDFITGRID_THIN_H
#define UI_SEDFITGRID_THIN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SedFitGrid_thin
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QHBoxLayout *horizontalLayout;
    QLabel *label_2;
    QLineEdit *minMassLineEdit;
    QLabel *label_3;
    QLineEdit *maxMassLineEdit;
    QLabel *label_4;
    QLineEdit *stepMassLineEdit;
    QFrame *line;
    QLabel *label_5;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_6;
    QLineEdit *tempMinLineEdit;
    QLabel *label_7;
    QLineEdit *tempMaxLineEdit;
    QLabel *label_8;
    QLineEdit *tempStepLineEdit;
    QFrame *line_2;
    QLabel *label_9;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_10;
    QLineEdit *betaMinLineEdit;
    QLabel *label_11;
    QLineEdit *betaMaxLineEdit;
    QLabel *label_12;
    QLineEdit *betaStepLineEdit;
    QFrame *line_3;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_13;
    QLineEdit *distLineEdit;
    QLabel *label_14;
    QLineEdit *srefOpacityLineEdit;
    QLabel *label_15;
    QLineEdit *srefWavelengthLineEdit;
    QFrame *line_4;
    QTableWidget *tableWidget;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton;
    QSpacerItem *horizontalSpacer_2;

    void setupUi(QWidget *SedFitGrid_thin)
    {
        if (SedFitGrid_thin->objectName().isEmpty())
            SedFitGrid_thin->setObjectName(QString::fromUtf8("SedFitGrid_thin"));
        SedFitGrid_thin->resize(534, 508);
        verticalLayout = new QVBoxLayout(SedFitGrid_thin);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(SedFitGrid_thin);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout->addWidget(label);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_2 = new QLabel(SedFitGrid_thin);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout->addWidget(label_2);

        minMassLineEdit = new QLineEdit(SedFitGrid_thin);
        minMassLineEdit->setObjectName(QString::fromUtf8("minMassLineEdit"));

        horizontalLayout->addWidget(minMassLineEdit);

        label_3 = new QLabel(SedFitGrid_thin);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout->addWidget(label_3);

        maxMassLineEdit = new QLineEdit(SedFitGrid_thin);
        maxMassLineEdit->setObjectName(QString::fromUtf8("maxMassLineEdit"));

        horizontalLayout->addWidget(maxMassLineEdit);

        label_4 = new QLabel(SedFitGrid_thin);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout->addWidget(label_4);

        stepMassLineEdit = new QLineEdit(SedFitGrid_thin);
        stepMassLineEdit->setObjectName(QString::fromUtf8("stepMassLineEdit"));

        horizontalLayout->addWidget(stepMassLineEdit);


        verticalLayout->addLayout(horizontalLayout);

        line = new QFrame(SedFitGrid_thin);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line);

        label_5 = new QLabel(SedFitGrid_thin);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        verticalLayout->addWidget(label_5);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_6 = new QLabel(SedFitGrid_thin);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        horizontalLayout_3->addWidget(label_6);

        tempMinLineEdit = new QLineEdit(SedFitGrid_thin);
        tempMinLineEdit->setObjectName(QString::fromUtf8("tempMinLineEdit"));

        horizontalLayout_3->addWidget(tempMinLineEdit);

        label_7 = new QLabel(SedFitGrid_thin);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        horizontalLayout_3->addWidget(label_7);

        tempMaxLineEdit = new QLineEdit(SedFitGrid_thin);
        tempMaxLineEdit->setObjectName(QString::fromUtf8("tempMaxLineEdit"));

        horizontalLayout_3->addWidget(tempMaxLineEdit);

        label_8 = new QLabel(SedFitGrid_thin);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        horizontalLayout_3->addWidget(label_8);

        tempStepLineEdit = new QLineEdit(SedFitGrid_thin);
        tempStepLineEdit->setObjectName(QString::fromUtf8("tempStepLineEdit"));

        horizontalLayout_3->addWidget(tempStepLineEdit);


        verticalLayout->addLayout(horizontalLayout_3);

        line_2 = new QFrame(SedFitGrid_thin);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line_2);

        label_9 = new QLabel(SedFitGrid_thin);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        verticalLayout->addWidget(label_9);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_10 = new QLabel(SedFitGrid_thin);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        horizontalLayout_4->addWidget(label_10);

        betaMinLineEdit = new QLineEdit(SedFitGrid_thin);
        betaMinLineEdit->setObjectName(QString::fromUtf8("betaMinLineEdit"));

        horizontalLayout_4->addWidget(betaMinLineEdit);

        label_11 = new QLabel(SedFitGrid_thin);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        horizontalLayout_4->addWidget(label_11);

        betaMaxLineEdit = new QLineEdit(SedFitGrid_thin);
        betaMaxLineEdit->setObjectName(QString::fromUtf8("betaMaxLineEdit"));

        horizontalLayout_4->addWidget(betaMaxLineEdit);

        label_12 = new QLabel(SedFitGrid_thin);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        horizontalLayout_4->addWidget(label_12);

        betaStepLineEdit = new QLineEdit(SedFitGrid_thin);
        betaStepLineEdit->setObjectName(QString::fromUtf8("betaStepLineEdit"));

        horizontalLayout_4->addWidget(betaStepLineEdit);


        verticalLayout->addLayout(horizontalLayout_4);

        line_3 = new QFrame(SedFitGrid_thin);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line_3);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_13 = new QLabel(SedFitGrid_thin);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        horizontalLayout_6->addWidget(label_13);

        distLineEdit = new QLineEdit(SedFitGrid_thin);
        distLineEdit->setObjectName(QString::fromUtf8("distLineEdit"));

        horizontalLayout_6->addWidget(distLineEdit);

        label_14 = new QLabel(SedFitGrid_thin);
        label_14->setObjectName(QString::fromUtf8("label_14"));

        horizontalLayout_6->addWidget(label_14);

        srefOpacityLineEdit = new QLineEdit(SedFitGrid_thin);
        srefOpacityLineEdit->setObjectName(QString::fromUtf8("srefOpacityLineEdit"));

        horizontalLayout_6->addWidget(srefOpacityLineEdit);

        label_15 = new QLabel(SedFitGrid_thin);
        label_15->setObjectName(QString::fromUtf8("label_15"));

        horizontalLayout_6->addWidget(label_15);

        srefWavelengthLineEdit = new QLineEdit(SedFitGrid_thin);
        srefWavelengthLineEdit->setObjectName(QString::fromUtf8("srefWavelengthLineEdit"));

        horizontalLayout_6->addWidget(srefWavelengthLineEdit);


        verticalLayout->addLayout(horizontalLayout_6);

        line_4 = new QFrame(SedFitGrid_thin);
        line_4->setObjectName(QString::fromUtf8("line_4"));
        line_4->setFrameShape(QFrame::HLine);
        line_4->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line_4);

        tableWidget = new QTableWidget(SedFitGrid_thin);
        if (tableWidget->columnCount() < 2)
            tableWidget->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        tableWidget->setObjectName(QString::fromUtf8("tableWidget"));

        verticalLayout->addWidget(tableWidget);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        pushButton = new QPushButton(SedFitGrid_thin);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        horizontalLayout_2->addWidget(pushButton);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(horizontalLayout_2);


        retranslateUi(SedFitGrid_thin);

        QMetaObject::connectSlotsByName(SedFitGrid_thin);
    } // setupUi

    void retranslateUi(QWidget *SedFitGrid_thin)
    {
        SedFitGrid_thin->setWindowTitle(QCoreApplication::translate("SedFitGrid_thin", "Form", nullptr));
        label->setText(QCoreApplication::translate("SedFitGrid_thin", "Mass", nullptr));
        label_2->setText(QCoreApplication::translate("SedFitGrid_thin", "Min: ", nullptr));
        minMassLineEdit->setText(QCoreApplication::translate("SedFitGrid_thin", "10", nullptr));
        label_3->setText(QCoreApplication::translate("SedFitGrid_thin", "Max: ", nullptr));
        maxMassLineEdit->setText(QCoreApplication::translate("SedFitGrid_thin", "5000", nullptr));
        label_4->setText(QCoreApplication::translate("SedFitGrid_thin", "Step:", nullptr));
        stepMassLineEdit->setText(QCoreApplication::translate("SedFitGrid_thin", "100", nullptr));
        label_5->setText(QCoreApplication::translate("SedFitGrid_thin", "Temperature", nullptr));
        label_6->setText(QCoreApplication::translate("SedFitGrid_thin", "Min:", nullptr));
        tempMinLineEdit->setText(QCoreApplication::translate("SedFitGrid_thin", "7", nullptr));
        label_7->setText(QCoreApplication::translate("SedFitGrid_thin", "Max:", nullptr));
        tempMaxLineEdit->setText(QCoreApplication::translate("SedFitGrid_thin", "40", nullptr));
        label_8->setText(QCoreApplication::translate("SedFitGrid_thin", "Step:", nullptr));
        tempStepLineEdit->setText(QCoreApplication::translate("SedFitGrid_thin", "50", nullptr));
        label_9->setText(QCoreApplication::translate("SedFitGrid_thin", "Beta", nullptr));
        label_10->setText(QCoreApplication::translate("SedFitGrid_thin", "Min:", nullptr));
        betaMinLineEdit->setText(QCoreApplication::translate("SedFitGrid_thin", "2", nullptr));
        label_11->setText(QCoreApplication::translate("SedFitGrid_thin", "Max:", nullptr));
        betaMaxLineEdit->setText(QCoreApplication::translate("SedFitGrid_thin", "2", nullptr));
        label_12->setText(QCoreApplication::translate("SedFitGrid_thin", "Step:", nullptr));
        betaStepLineEdit->setText(QCoreApplication::translate("SedFitGrid_thin", "1", nullptr));
        label_13->setText(QCoreApplication::translate("SedFitGrid_thin", "Dist:", nullptr));
        distLineEdit->setText(QCoreApplication::translate("SedFitGrid_thin", "1000", nullptr));
        label_14->setText(QCoreApplication::translate("SedFitGrid_thin", "sref_opacity:", nullptr));
        srefOpacityLineEdit->setText(QCoreApplication::translate("SedFitGrid_thin", "0.1", nullptr));
        label_15->setText(QCoreApplication::translate("SedFitGrid_thin", "sref_wavelength:", nullptr));
        srefWavelengthLineEdit->setText(QCoreApplication::translate("SedFitGrid_thin", "300", nullptr));
        QTableWidgetItem *___qtablewidgetitem = tableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("SedFitGrid_thin", "Wavelength", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tableWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("SedFitGrid_thin", "Ulimit", nullptr));
        pushButton->setText(QCoreApplication::translate("SedFitGrid_thin", "Fit", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SedFitGrid_thin: public Ui_SedFitGrid_thin {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SEDFITGRID_THIN_H
