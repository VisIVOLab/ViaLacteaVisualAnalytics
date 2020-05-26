/********************************************************************************
** Form generated from reading UI file 'lutcustomize.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LUTCUSTOMIZE_H
#define UI_LUTCUSTOMIZE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "src/qcustomplot.h"

QT_BEGIN_NAMESPACE

class Ui_LutCustomize
{
public:
    QVBoxLayout *verticalLayout;
    QCustomPlot *histogramWidget;
    QLabel *label_3;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label;
    QSlider *fromSlider;
    QLineEdit *fromValue;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_2;
    QSlider *toSlider;
    QLineEdit *toValue;
    QCheckBox *ShowColorbarCheckBox;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout;
    QPushButton *cancelPushButton;
    QPushButton *okPushButton;

    void setupUi(QWidget *LutCustomize)
    {
        if (LutCustomize->objectName().isEmpty())
            LutCustomize->setObjectName(QString::fromUtf8("LutCustomize"));
        LutCustomize->resize(400, 391);
        verticalLayout = new QVBoxLayout(LutCustomize);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        histogramWidget = new QCustomPlot(LutCustomize);
        histogramWidget->setObjectName(QString::fromUtf8("histogramWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(histogramWidget->sizePolicy().hasHeightForWidth());
        histogramWidget->setSizePolicy(sizePolicy);
        histogramWidget->setMinimumSize(QSize(0, 200));

        verticalLayout->addWidget(histogramWidget);

        label_3 = new QLabel(LutCustomize);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        verticalLayout->addWidget(label_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label = new QLabel(LutCustomize);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_4->addWidget(label);

        fromSlider = new QSlider(LutCustomize);
        fromSlider->setObjectName(QString::fromUtf8("fromSlider"));
        fromSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_4->addWidget(fromSlider);

        fromValue = new QLineEdit(LutCustomize);
        fromValue->setObjectName(QString::fromUtf8("fromValue"));

        horizontalLayout_4->addWidget(fromValue);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_2 = new QLabel(LutCustomize);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_5->addWidget(label_2);

        toSlider = new QSlider(LutCustomize);
        toSlider->setObjectName(QString::fromUtf8("toSlider"));
        toSlider->setSliderPosition(99);
        toSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_5->addWidget(toSlider);

        toValue = new QLineEdit(LutCustomize);
        toValue->setObjectName(QString::fromUtf8("toValue"));

        horizontalLayout_5->addWidget(toValue);


        verticalLayout->addLayout(horizontalLayout_5);

        ShowColorbarCheckBox = new QCheckBox(LutCustomize);
        ShowColorbarCheckBox->setObjectName(QString::fromUtf8("ShowColorbarCheckBox"));
        ShowColorbarCheckBox->setChecked(true);

        verticalLayout->addWidget(ShowColorbarCheckBox);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        cancelPushButton = new QPushButton(LutCustomize);
        cancelPushButton->setObjectName(QString::fromUtf8("cancelPushButton"));

        horizontalLayout->addWidget(cancelPushButton);

        okPushButton = new QPushButton(LutCustomize);
        okPushButton->setObjectName(QString::fromUtf8("okPushButton"));

        horizontalLayout->addWidget(okPushButton);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(LutCustomize);

        QMetaObject::connectSlotsByName(LutCustomize);
    } // setupUi

    void retranslateUi(QWidget *LutCustomize)
    {
        LutCustomize->setWindowTitle(QCoreApplication::translate("LutCustomize", "Form", nullptr));
        label_3->setText(QCoreApplication::translate("LutCustomize", "Range", nullptr));
        label->setText(QCoreApplication::translate("LutCustomize", "Min:", nullptr));
        label_2->setText(QCoreApplication::translate("LutCustomize", "Max:", nullptr));
        ShowColorbarCheckBox->setText(QCoreApplication::translate("LutCustomize", "Show Colorbar", nullptr));
        cancelPushButton->setText(QCoreApplication::translate("LutCustomize", "Cancel", nullptr));
        okPushButton->setText(QCoreApplication::translate("LutCustomize", "Ok", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LutCustomize: public Ui_LutCustomize {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LUTCUSTOMIZE_H
