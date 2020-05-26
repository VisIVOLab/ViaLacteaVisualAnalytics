/********************************************************************************
** Form generated from reading UI file 'sedfitgrid_thick.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SEDFITGRID_THICK_H
#define UI_SEDFITGRID_THICK_H

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

class Ui_SedFitgrid_thick
{
public:
    QVBoxLayout *verticalLayout;
    QLabel *label;
    QHBoxLayout *horizontalLayout;
    QLabel *label_2;
    QLineEdit *minLambda_0LineEdit;
    QLabel *label_3;
    QLineEdit *maxLambda_0LineEdit;
    QLabel *label_4;
    QLineEdit *stepLambda_0LineEdit;
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
    QLineEdit *sizeLineEdit;
    QSpacerItem *horizontalSpacer_3;
    QLabel *label_20;
    QLineEdit *distLineEdit;
    QLabel *label_16;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_17;
    QLineEdit *scaleMinLineEdit;
    QLabel *label_18;
    QLineEdit *scaleMaxLineEdit;
    QLabel *label_19;
    QLineEdit *scaleStepLineEdit;
    QFrame *line_4;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_14;
    QLineEdit *srefOpacityLineEdit;
    QLabel *label_15;
    QLineEdit *srefWavelengthLineEdit;
    QFrame *line_5;
    QTableWidget *tableWidget;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton;
    QSpacerItem *horizontalSpacer_2;

    void setupUi(QWidget *SedFitgrid_thick)
    {
        if (SedFitgrid_thick->objectName().isEmpty())
            SedFitgrid_thick->setObjectName(QString::fromUtf8("SedFitgrid_thick"));
        SedFitgrid_thick->resize(489, 640);
        verticalLayout = new QVBoxLayout(SedFitgrid_thick);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label = new QLabel(SedFitgrid_thick);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout->addWidget(label);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_2 = new QLabel(SedFitgrid_thick);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout->addWidget(label_2);

        minLambda_0LineEdit = new QLineEdit(SedFitgrid_thick);
        minLambda_0LineEdit->setObjectName(QString::fromUtf8("minLambda_0LineEdit"));

        horizontalLayout->addWidget(minLambda_0LineEdit);

        label_3 = new QLabel(SedFitgrid_thick);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout->addWidget(label_3);

        maxLambda_0LineEdit = new QLineEdit(SedFitgrid_thick);
        maxLambda_0LineEdit->setObjectName(QString::fromUtf8("maxLambda_0LineEdit"));

        horizontalLayout->addWidget(maxLambda_0LineEdit);

        label_4 = new QLabel(SedFitgrid_thick);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout->addWidget(label_4);

        stepLambda_0LineEdit = new QLineEdit(SedFitgrid_thick);
        stepLambda_0LineEdit->setObjectName(QString::fromUtf8("stepLambda_0LineEdit"));

        horizontalLayout->addWidget(stepLambda_0LineEdit);


        verticalLayout->addLayout(horizontalLayout);

        line = new QFrame(SedFitgrid_thick);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line);

        label_5 = new QLabel(SedFitgrid_thick);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        verticalLayout->addWidget(label_5);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_6 = new QLabel(SedFitgrid_thick);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        horizontalLayout_3->addWidget(label_6);

        tempMinLineEdit = new QLineEdit(SedFitgrid_thick);
        tempMinLineEdit->setObjectName(QString::fromUtf8("tempMinLineEdit"));

        horizontalLayout_3->addWidget(tempMinLineEdit);

        label_7 = new QLabel(SedFitgrid_thick);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        horizontalLayout_3->addWidget(label_7);

        tempMaxLineEdit = new QLineEdit(SedFitgrid_thick);
        tempMaxLineEdit->setObjectName(QString::fromUtf8("tempMaxLineEdit"));

        horizontalLayout_3->addWidget(tempMaxLineEdit);

        label_8 = new QLabel(SedFitgrid_thick);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        horizontalLayout_3->addWidget(label_8);

        tempStepLineEdit = new QLineEdit(SedFitgrid_thick);
        tempStepLineEdit->setObjectName(QString::fromUtf8("tempStepLineEdit"));

        horizontalLayout_3->addWidget(tempStepLineEdit);


        verticalLayout->addLayout(horizontalLayout_3);

        line_2 = new QFrame(SedFitgrid_thick);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line_2);

        label_9 = new QLabel(SedFitgrid_thick);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        verticalLayout->addWidget(label_9);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_10 = new QLabel(SedFitgrid_thick);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        horizontalLayout_4->addWidget(label_10);

        betaMinLineEdit = new QLineEdit(SedFitgrid_thick);
        betaMinLineEdit->setObjectName(QString::fromUtf8("betaMinLineEdit"));

        horizontalLayout_4->addWidget(betaMinLineEdit);

        label_11 = new QLabel(SedFitgrid_thick);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        horizontalLayout_4->addWidget(label_11);

        betaMaxLineEdit = new QLineEdit(SedFitgrid_thick);
        betaMaxLineEdit->setObjectName(QString::fromUtf8("betaMaxLineEdit"));

        horizontalLayout_4->addWidget(betaMaxLineEdit);

        label_12 = new QLabel(SedFitgrid_thick);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        horizontalLayout_4->addWidget(label_12);

        betaStepLineEdit = new QLineEdit(SedFitgrid_thick);
        betaStepLineEdit->setObjectName(QString::fromUtf8("betaStepLineEdit"));

        horizontalLayout_4->addWidget(betaStepLineEdit);


        verticalLayout->addLayout(horizontalLayout_4);

        line_3 = new QFrame(SedFitgrid_thick);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line_3);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_13 = new QLabel(SedFitgrid_thick);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        horizontalLayout_6->addWidget(label_13);

        sizeLineEdit = new QLineEdit(SedFitgrid_thick);
        sizeLineEdit->setObjectName(QString::fromUtf8("sizeLineEdit"));

        horizontalLayout_6->addWidget(sizeLineEdit);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_3);

        label_20 = new QLabel(SedFitgrid_thick);
        label_20->setObjectName(QString::fromUtf8("label_20"));

        horizontalLayout_6->addWidget(label_20);

        distLineEdit = new QLineEdit(SedFitgrid_thick);
        distLineEdit->setObjectName(QString::fromUtf8("distLineEdit"));

        horizontalLayout_6->addWidget(distLineEdit);


        verticalLayout->addLayout(horizontalLayout_6);

        label_16 = new QLabel(SedFitgrid_thick);
        label_16->setObjectName(QString::fromUtf8("label_16"));

        verticalLayout->addWidget(label_16);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_17 = new QLabel(SedFitgrid_thick);
        label_17->setObjectName(QString::fromUtf8("label_17"));

        horizontalLayout_5->addWidget(label_17);

        scaleMinLineEdit = new QLineEdit(SedFitgrid_thick);
        scaleMinLineEdit->setObjectName(QString::fromUtf8("scaleMinLineEdit"));

        horizontalLayout_5->addWidget(scaleMinLineEdit);

        label_18 = new QLabel(SedFitgrid_thick);
        label_18->setObjectName(QString::fromUtf8("label_18"));

        horizontalLayout_5->addWidget(label_18);

        scaleMaxLineEdit = new QLineEdit(SedFitgrid_thick);
        scaleMaxLineEdit->setObjectName(QString::fromUtf8("scaleMaxLineEdit"));

        horizontalLayout_5->addWidget(scaleMaxLineEdit);

        label_19 = new QLabel(SedFitgrid_thick);
        label_19->setObjectName(QString::fromUtf8("label_19"));

        horizontalLayout_5->addWidget(label_19);

        scaleStepLineEdit = new QLineEdit(SedFitgrid_thick);
        scaleStepLineEdit->setObjectName(QString::fromUtf8("scaleStepLineEdit"));

        horizontalLayout_5->addWidget(scaleStepLineEdit);


        verticalLayout->addLayout(horizontalLayout_5);

        line_4 = new QFrame(SedFitgrid_thick);
        line_4->setObjectName(QString::fromUtf8("line_4"));
        line_4->setFrameShape(QFrame::HLine);
        line_4->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line_4);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        label_14 = new QLabel(SedFitgrid_thick);
        label_14->setObjectName(QString::fromUtf8("label_14"));

        horizontalLayout_7->addWidget(label_14);

        srefOpacityLineEdit = new QLineEdit(SedFitgrid_thick);
        srefOpacityLineEdit->setObjectName(QString::fromUtf8("srefOpacityLineEdit"));

        horizontalLayout_7->addWidget(srefOpacityLineEdit);

        label_15 = new QLabel(SedFitgrid_thick);
        label_15->setObjectName(QString::fromUtf8("label_15"));

        horizontalLayout_7->addWidget(label_15);

        srefWavelengthLineEdit = new QLineEdit(SedFitgrid_thick);
        srefWavelengthLineEdit->setObjectName(QString::fromUtf8("srefWavelengthLineEdit"));

        horizontalLayout_7->addWidget(srefWavelengthLineEdit);


        verticalLayout->addLayout(horizontalLayout_7);

        line_5 = new QFrame(SedFitgrid_thick);
        line_5->setObjectName(QString::fromUtf8("line_5"));
        line_5->setFrameShape(QFrame::HLine);
        line_5->setFrameShadow(QFrame::Sunken);

        verticalLayout->addWidget(line_5);

        tableWidget = new QTableWidget(SedFitgrid_thick);
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

        pushButton = new QPushButton(SedFitgrid_thick);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        horizontalLayout_2->addWidget(pushButton);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(horizontalLayout_2);


        retranslateUi(SedFitgrid_thick);

        QMetaObject::connectSlotsByName(SedFitgrid_thick);
    } // setupUi

    void retranslateUi(QWidget *SedFitgrid_thick)
    {
        SedFitgrid_thick->setWindowTitle(QCoreApplication::translate("SedFitgrid_thick", "Form", nullptr));
        label->setText(QCoreApplication::translate("SedFitgrid_thick", "Mass", nullptr));
        label_2->setText(QCoreApplication::translate("SedFitgrid_thick", "lambda_0", nullptr));
        minLambda_0LineEdit->setText(QCoreApplication::translate("SedFitgrid_thick", "10", nullptr));
        label_3->setText(QCoreApplication::translate("SedFitgrid_thick", "Max: ", nullptr));
        maxLambda_0LineEdit->setText(QCoreApplication::translate("SedFitgrid_thick", "300", nullptr));
        label_4->setText(QCoreApplication::translate("SedFitgrid_thick", "Step:", nullptr));
        stepLambda_0LineEdit->setText(QCoreApplication::translate("SedFitgrid_thick", "100", nullptr));
        label_5->setText(QCoreApplication::translate("SedFitgrid_thick", "Temperature", nullptr));
        label_6->setText(QCoreApplication::translate("SedFitgrid_thick", "Min:", nullptr));
        tempMinLineEdit->setText(QCoreApplication::translate("SedFitgrid_thick", "7", nullptr));
        label_7->setText(QCoreApplication::translate("SedFitgrid_thick", "Max:", nullptr));
        tempMaxLineEdit->setText(QCoreApplication::translate("SedFitgrid_thick", "40", nullptr));
        label_8->setText(QCoreApplication::translate("SedFitgrid_thick", "Step:", nullptr));
        tempStepLineEdit->setText(QCoreApplication::translate("SedFitgrid_thick", "50", nullptr));
        label_9->setText(QCoreApplication::translate("SedFitgrid_thick", "Beta", nullptr));
        label_10->setText(QCoreApplication::translate("SedFitgrid_thick", "Min:", nullptr));
        betaMinLineEdit->setText(QCoreApplication::translate("SedFitgrid_thick", "2", nullptr));
        label_11->setText(QCoreApplication::translate("SedFitgrid_thick", "Max:", nullptr));
        betaMaxLineEdit->setText(QCoreApplication::translate("SedFitgrid_thick", "2", nullptr));
        label_12->setText(QCoreApplication::translate("SedFitgrid_thick", "Step:", nullptr));
        betaStepLineEdit->setText(QCoreApplication::translate("SedFitgrid_thick", "1", nullptr));
        label_13->setText(QCoreApplication::translate("SedFitgrid_thick", "Size:", nullptr));
        sizeLineEdit->setText(QCoreApplication::translate("SedFitgrid_thick", "20", nullptr));
        label_20->setText(QCoreApplication::translate("SedFitgrid_thick", "Dist", nullptr));
        distLineEdit->setText(QCoreApplication::translate("SedFitgrid_thick", "1000", nullptr));
        label_16->setText(QCoreApplication::translate("SedFitgrid_thick", "Scale Factor", nullptr));
        label_17->setText(QCoreApplication::translate("SedFitgrid_thick", "Min:", nullptr));
        scaleMinLineEdit->setText(QCoreApplication::translate("SedFitgrid_thick", "1", nullptr));
        label_18->setText(QCoreApplication::translate("SedFitgrid_thick", "Max:", nullptr));
        scaleMaxLineEdit->setText(QCoreApplication::translate("SedFitgrid_thick", "1", nullptr));
        label_19->setText(QCoreApplication::translate("SedFitgrid_thick", "Step:", nullptr));
        scaleStepLineEdit->setText(QCoreApplication::translate("SedFitgrid_thick", "1", nullptr));
        label_14->setText(QCoreApplication::translate("SedFitgrid_thick", "sref_opacity:", nullptr));
        srefOpacityLineEdit->setText(QCoreApplication::translate("SedFitgrid_thick", "0.1", nullptr));
        label_15->setText(QCoreApplication::translate("SedFitgrid_thick", "sref_wavelength:", nullptr));
        srefWavelengthLineEdit->setText(QCoreApplication::translate("SedFitgrid_thick", "300", nullptr));
        QTableWidgetItem *___qtablewidgetitem = tableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("SedFitgrid_thick", "Wavelength", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = tableWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("SedFitgrid_thick", "Ulimit", nullptr));
        pushButton->setText(QCoreApplication::translate("SedFitgrid_thick", "Fit", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SedFitgrid_thick: public Ui_SedFitgrid_thick {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SEDFITGRID_THICK_H
