/********************************************************************************
** Form generated from reading UI file 'contour.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CONTOUR_H
#define UI_CONTOUR_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_contour
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QCheckBox *activateCheckBox;
    QCheckBox *LogLinearCheckBox;
    QCheckBox *labelCheckBox;
    QCheckBox *idCheckBox;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_contours;
    QLineEdit *numOfContourText;
    QLabel *label_min;
    QLineEdit *minValueText;
    QLabel *label_max;
    QLineEdit *maxValueText;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_5;
    QLineEdit *slicesText;
    QLabel *label_7;
    QLineEdit *minFitsText;
    QLabel *label_8;
    QLineEdit *maxFitsText;
    QLabel *label_4;
    QLineEdit *RMSText;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_3;
    QDoubleSpinBox *redText;
    QLabel *label_2;
    QDoubleSpinBox *greenText;
    QLabel *label;
    QDoubleSpinBox *blueText;
    QHBoxLayout *horizontalLayout_6;
    QSpacerItem *horizontalSpacer;
    QPushButton *okButton;
    QSpacerItem *horizontalSpacer_2;

    void setupUi(QWidget *contour)
    {
        if (contour->objectName().isEmpty())
            contour->setObjectName(QString::fromUtf8("contour"));
        contour->resize(490, 421);
        verticalLayout = new QVBoxLayout(contour);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        activateCheckBox = new QCheckBox(contour);
        activateCheckBox->setObjectName(QString::fromUtf8("activateCheckBox"));
        activateCheckBox->setChecked(false);

        horizontalLayout->addWidget(activateCheckBox);

        LogLinearCheckBox = new QCheckBox(contour);
        LogLinearCheckBox->setObjectName(QString::fromUtf8("LogLinearCheckBox"));
        LogLinearCheckBox->setChecked(false);

        horizontalLayout->addWidget(LogLinearCheckBox);

        labelCheckBox = new QCheckBox(contour);
        labelCheckBox->setObjectName(QString::fromUtf8("labelCheckBox"));
        labelCheckBox->setEnabled(false);

        horizontalLayout->addWidget(labelCheckBox);

        idCheckBox = new QCheckBox(contour);
        idCheckBox->setObjectName(QString::fromUtf8("idCheckBox"));
        idCheckBox->setEnabled(false);

        horizontalLayout->addWidget(idCheckBox);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_contours = new QLabel(contour);
        label_contours->setObjectName(QString::fromUtf8("label_contours"));

        horizontalLayout_3->addWidget(label_contours);

        numOfContourText = new QLineEdit(contour);
        numOfContourText->setObjectName(QString::fromUtf8("numOfContourText"));

        horizontalLayout_3->addWidget(numOfContourText);

        label_min = new QLabel(contour);
        label_min->setObjectName(QString::fromUtf8("label_min"));

        horizontalLayout_3->addWidget(label_min);

        minValueText = new QLineEdit(contour);
        minValueText->setObjectName(QString::fromUtf8("minValueText"));

        horizontalLayout_3->addWidget(minValueText);

        label_max = new QLabel(contour);
        label_max->setObjectName(QString::fromUtf8("label_max"));

        horizontalLayout_3->addWidget(label_max);

        maxValueText = new QLineEdit(contour);
        maxValueText->setObjectName(QString::fromUtf8("maxValueText"));

        horizontalLayout_3->addWidget(maxValueText);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_5 = new QLabel(contour);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        horizontalLayout_5->addWidget(label_5);

        slicesText = new QLineEdit(contour);
        slicesText->setObjectName(QString::fromUtf8("slicesText"));

        horizontalLayout_5->addWidget(slicesText);

        label_7 = new QLabel(contour);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        horizontalLayout_5->addWidget(label_7);

        minFitsText = new QLineEdit(contour);
        minFitsText->setObjectName(QString::fromUtf8("minFitsText"));

        horizontalLayout_5->addWidget(minFitsText);

        label_8 = new QLabel(contour);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        horizontalLayout_5->addWidget(label_8);

        maxFitsText = new QLineEdit(contour);
        maxFitsText->setObjectName(QString::fromUtf8("maxFitsText"));

        horizontalLayout_5->addWidget(maxFitsText);

        label_4 = new QLabel(contour);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_5->addWidget(label_4);

        RMSText = new QLineEdit(contour);
        RMSText->setObjectName(QString::fromUtf8("RMSText"));

        horizontalLayout_5->addWidget(RMSText);


        verticalLayout->addLayout(horizontalLayout_5);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_3 = new QLabel(contour);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_4->addWidget(label_3);

        redText = new QDoubleSpinBox(contour);
        redText->setObjectName(QString::fromUtf8("redText"));

        horizontalLayout_4->addWidget(redText);

        label_2 = new QLabel(contour);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_4->addWidget(label_2);

        greenText = new QDoubleSpinBox(contour);
        greenText->setObjectName(QString::fromUtf8("greenText"));

        horizontalLayout_4->addWidget(greenText);

        label = new QLabel(contour);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_4->addWidget(label);

        blueText = new QDoubleSpinBox(contour);
        blueText->setObjectName(QString::fromUtf8("blueText"));

        horizontalLayout_4->addWidget(blueText);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer);

        okButton = new QPushButton(contour);
        okButton->setObjectName(QString::fromUtf8("okButton"));

        horizontalLayout_6->addWidget(okButton);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_2);


        verticalLayout->addLayout(horizontalLayout_6);


        retranslateUi(contour);

        QMetaObject::connectSlotsByName(contour);
    } // setupUi

    void retranslateUi(QWidget *contour)
    {
        contour->setWindowTitle(QCoreApplication::translate("contour", "Contour", nullptr));
        activateCheckBox->setText(QCoreApplication::translate("contour", "Activate Contours", nullptr));
        LogLinearCheckBox->setText(QCoreApplication::translate("contour", "Log/Linear", nullptr));
        labelCheckBox->setText(QCoreApplication::translate("contour", "Value label", nullptr));
        idCheckBox->setText(QCoreApplication::translate("contour", "Id label", nullptr));
        label_contours->setText(QCoreApplication::translate("contour", "#Contours", nullptr));
        label_min->setText(QCoreApplication::translate("contour", "Lower Bound", nullptr));
        label_max->setText(QCoreApplication::translate("contour", "Upper Bound", nullptr));
        label_5->setText(QCoreApplication::translate("contour", "#Slices", nullptr));
        label_7->setText(QCoreApplication::translate("contour", "Min", nullptr));
        label_8->setText(QCoreApplication::translate("contour", "Max", nullptr));
        label_4->setText(QCoreApplication::translate("contour", "RMS", nullptr));
        label_3->setText(QCoreApplication::translate("contour", "Red", nullptr));
        label_2->setText(QCoreApplication::translate("contour", "Green", nullptr));
        label->setText(QCoreApplication::translate("contour", "Blue", nullptr));
        okButton->setText(QCoreApplication::translate("contour", "OK", nullptr));
    } // retranslateUi

};

namespace Ui {
    class contour: public Ui_contour {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CONTOUR_H
