/********************************************************************************
** Form generated from reading UI file 'plotwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PLOTWINDOW_H
#define UI_PLOTWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "src/qcustomplot.h"

QT_BEGIN_NAMESPACE

class Ui_PlotWindow
{
public:
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QLabel *label_2;
    QComboBox *xComboBox;
    QCheckBox *logxCheckBox;
    QSpacerItem *horizontalSpacer;
    QLabel *label;
    QComboBox *yComboBox;
    QCheckBox *logyCheckBox;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *plotButton;
    QCustomPlot *customPlot;

    void setupUi(QWidget *PlotWindow)
    {
        if (PlotWindow->objectName().isEmpty())
            PlotWindow->setObjectName(QString::fromUtf8("PlotWindow"));
        PlotWindow->resize(729, 597);
        verticalLayout_2 = new QVBoxLayout(PlotWindow);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_2 = new QLabel(PlotWindow);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label_2);

        xComboBox = new QComboBox(PlotWindow);
        xComboBox->setObjectName(QString::fromUtf8("xComboBox"));

        horizontalLayout->addWidget(xComboBox);

        logxCheckBox = new QCheckBox(PlotWindow);
        logxCheckBox->setObjectName(QString::fromUtf8("logxCheckBox"));

        horizontalLayout->addWidget(logxCheckBox);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        label = new QLabel(PlotWindow);
        label->setObjectName(QString::fromUtf8("label"));
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        horizontalLayout->addWidget(label);

        yComboBox = new QComboBox(PlotWindow);
        yComboBox->setObjectName(QString::fromUtf8("yComboBox"));

        horizontalLayout->addWidget(yComboBox);

        logyCheckBox = new QCheckBox(PlotWindow);
        logyCheckBox->setObjectName(QString::fromUtf8("logyCheckBox"));

        horizontalLayout->addWidget(logyCheckBox);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        plotButton = new QPushButton(PlotWindow);
        plotButton->setObjectName(QString::fromUtf8("plotButton"));

        horizontalLayout->addWidget(plotButton);


        verticalLayout_2->addLayout(horizontalLayout);

        customPlot = new QCustomPlot(PlotWindow);
        customPlot->setObjectName(QString::fromUtf8("customPlot"));

        verticalLayout_2->addWidget(customPlot);


        retranslateUi(PlotWindow);

        QMetaObject::connectSlotsByName(PlotWindow);
    } // setupUi

    void retranslateUi(QWidget *PlotWindow)
    {
        PlotWindow->setWindowTitle(QCoreApplication::translate("PlotWindow", "Form", nullptr));
        label_2->setText(QCoreApplication::translate("PlotWindow", "X:", nullptr));
        logxCheckBox->setText(QCoreApplication::translate("PlotWindow", "Log", nullptr));
        label->setText(QCoreApplication::translate("PlotWindow", "Y:", nullptr));
        logyCheckBox->setText(QCoreApplication::translate("PlotWindow", "Log", nullptr));
        plotButton->setText(QCoreApplication::translate("PlotWindow", "Plot", nullptr));
    } // retranslateUi

};

namespace Ui {
    class PlotWindow: public Ui_PlotWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PLOTWINDOW_H
