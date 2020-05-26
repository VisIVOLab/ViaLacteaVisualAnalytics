/********************************************************************************
** Form generated from reading UI file 'vtktoolswidget.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VTKTOOLSWIDGET_H
#define UI_VTKTOOLSWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
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

class Ui_vtktoolswidget
{
public:
    QVBoxLayout *verticalLayout_2;
    QLabel *label_3;
    QHBoxLayout *horizontalLayout;
    QPushButton *cameraLeft;
    QPushButton *cameraBack;
    QPushButton *cameraRight;
    QPushButton *frontCamera;
    QPushButton *topCamera;
    QPushButton *bottomCamera;
    QFrame *line;
    QHBoxLayout *horizontalLayout_2;
    QCheckBox *ShowBoxCheckBox;
    QSpacerItem *horizontalSpacer_2;
    QCheckBox *ShowAxesCheckBox;
    QSpacerItem *horizontalSpacer;
    QCheckBox *ShowColorbarCheckBox;
    QFrame *line_2;
    QHBoxLayout *horizontalLayout_6;
    QCheckBox *scaleCheckBox;
    QCheckBox *gridCheckBox;
    QFrame *line_4;
    QLabel *label_5;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *activateLutCheckBox;
    QSpacerItem *horizontalSpacer_4;
    QLabel *scalarLabel;
    QComboBox *scalarComboBox;
    QSpacerItem *horizontalSpacer_3;
    QLabel *lutLabel;
    QComboBox *lutComboBox;
    QVBoxLayout *verticalLayout;
    QCustomPlot *histogramWidget;
    QLabel *label_4;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label;
    QSlider *fromSlider;
    QLineEdit *fromValue;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_2;
    QSlider *toSlider;
    QLineEdit *toValue;
    QFrame *line_3;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *vtktoolswidget)
    {
        if (vtktoolswidget->objectName().isEmpty())
            vtktoolswidget->setObjectName(QString::fromUtf8("vtktoolswidget"));
        vtktoolswidget->resize(496, 628);
        verticalLayout_2 = new QVBoxLayout(vtktoolswidget);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        label_3 = new QLabel(vtktoolswidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        verticalLayout_2->addWidget(label_3);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        cameraLeft = new QPushButton(vtktoolswidget);
        cameraLeft->setObjectName(QString::fromUtf8("cameraLeft"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/PIC_LEFT.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        cameraLeft->setIcon(icon);

        horizontalLayout->addWidget(cameraLeft);

        cameraBack = new QPushButton(vtktoolswidget);
        cameraBack->setObjectName(QString::fromUtf8("cameraBack"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/PIC_BACK.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        cameraBack->setIcon(icon1);

        horizontalLayout->addWidget(cameraBack);

        cameraRight = new QPushButton(vtktoolswidget);
        cameraRight->setObjectName(QString::fromUtf8("cameraRight"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/PIC_RIGHT.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        cameraRight->setIcon(icon2);

        horizontalLayout->addWidget(cameraRight);

        frontCamera = new QPushButton(vtktoolswidget);
        frontCamera->setObjectName(QString::fromUtf8("frontCamera"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/PIC_FRONT.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        frontCamera->setIcon(icon3);

        horizontalLayout->addWidget(frontCamera);

        topCamera = new QPushButton(vtktoolswidget);
        topCamera->setObjectName(QString::fromUtf8("topCamera"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icons/PIC_TOP.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        topCamera->setIcon(icon4);

        horizontalLayout->addWidget(topCamera);

        bottomCamera = new QPushButton(vtktoolswidget);
        bottomCamera->setObjectName(QString::fromUtf8("bottomCamera"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/icons/PIC_BOTTOM.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        bottomCamera->setIcon(icon5);

        horizontalLayout->addWidget(bottomCamera);


        verticalLayout_2->addLayout(horizontalLayout);

        line = new QFrame(vtktoolswidget);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout_2->addWidget(line);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        ShowBoxCheckBox = new QCheckBox(vtktoolswidget);
        ShowBoxCheckBox->setObjectName(QString::fromUtf8("ShowBoxCheckBox"));
        ShowBoxCheckBox->setChecked(true);

        horizontalLayout_2->addWidget(ShowBoxCheckBox);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);

        ShowAxesCheckBox = new QCheckBox(vtktoolswidget);
        ShowAxesCheckBox->setObjectName(QString::fromUtf8("ShowAxesCheckBox"));
        ShowAxesCheckBox->setChecked(true);

        horizontalLayout_2->addWidget(ShowAxesCheckBox);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        ShowColorbarCheckBox = new QCheckBox(vtktoolswidget);
        ShowColorbarCheckBox->setObjectName(QString::fromUtf8("ShowColorbarCheckBox"));

        horizontalLayout_2->addWidget(ShowColorbarCheckBox);


        verticalLayout_2->addLayout(horizontalLayout_2);

        line_2 = new QFrame(vtktoolswidget);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        verticalLayout_2->addWidget(line_2);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        scaleCheckBox = new QCheckBox(vtktoolswidget);
        scaleCheckBox->setObjectName(QString::fromUtf8("scaleCheckBox"));

        horizontalLayout_6->addWidget(scaleCheckBox);

        gridCheckBox = new QCheckBox(vtktoolswidget);
        gridCheckBox->setObjectName(QString::fromUtf8("gridCheckBox"));

        horizontalLayout_6->addWidget(gridCheckBox);


        verticalLayout_2->addLayout(horizontalLayout_6);

        line_4 = new QFrame(vtktoolswidget);
        line_4->setObjectName(QString::fromUtf8("line_4"));
        line_4->setFrameShape(QFrame::HLine);
        line_4->setFrameShadow(QFrame::Sunken);

        verticalLayout_2->addWidget(line_4);

        label_5 = new QLabel(vtktoolswidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        verticalLayout_2->addWidget(label_5);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        activateLutCheckBox = new QCheckBox(vtktoolswidget);
        activateLutCheckBox->setObjectName(QString::fromUtf8("activateLutCheckBox"));

        horizontalLayout_3->addWidget(activateLutCheckBox);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_4);

        scalarLabel = new QLabel(vtktoolswidget);
        scalarLabel->setObjectName(QString::fromUtf8("scalarLabel"));
        scalarLabel->setEnabled(false);

        horizontalLayout_3->addWidget(scalarLabel);

        scalarComboBox = new QComboBox(vtktoolswidget);
        scalarComboBox->setObjectName(QString::fromUtf8("scalarComboBox"));
        scalarComboBox->setEnabled(false);

        horizontalLayout_3->addWidget(scalarComboBox);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_3);

        lutLabel = new QLabel(vtktoolswidget);
        lutLabel->setObjectName(QString::fromUtf8("lutLabel"));
        lutLabel->setEnabled(false);

        horizontalLayout_3->addWidget(lutLabel);

        lutComboBox = new QComboBox(vtktoolswidget);
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->setObjectName(QString::fromUtf8("lutComboBox"));
        lutComboBox->setEnabled(false);

        horizontalLayout_3->addWidget(lutComboBox);


        verticalLayout_2->addLayout(horizontalLayout_3);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        histogramWidget = new QCustomPlot(vtktoolswidget);
        histogramWidget->setObjectName(QString::fromUtf8("histogramWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(histogramWidget->sizePolicy().hasHeightForWidth());
        histogramWidget->setSizePolicy(sizePolicy);

        verticalLayout->addWidget(histogramWidget);


        verticalLayout_2->addLayout(verticalLayout);

        label_4 = new QLabel(vtktoolswidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        verticalLayout_2->addWidget(label_4);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label = new QLabel(vtktoolswidget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_4->addWidget(label);

        fromSlider = new QSlider(vtktoolswidget);
        fromSlider->setObjectName(QString::fromUtf8("fromSlider"));
        fromSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_4->addWidget(fromSlider);

        fromValue = new QLineEdit(vtktoolswidget);
        fromValue->setObjectName(QString::fromUtf8("fromValue"));

        horizontalLayout_4->addWidget(fromValue);


        verticalLayout_3->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_2 = new QLabel(vtktoolswidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_5->addWidget(label_2);

        toSlider = new QSlider(vtktoolswidget);
        toSlider->setObjectName(QString::fromUtf8("toSlider"));
        toSlider->setSliderPosition(99);
        toSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_5->addWidget(toSlider);

        toValue = new QLineEdit(vtktoolswidget);
        toValue->setObjectName(QString::fromUtf8("toValue"));

        horizontalLayout_5->addWidget(toValue);


        verticalLayout_3->addLayout(horizontalLayout_5);


        verticalLayout_2->addLayout(verticalLayout_3);

        line_3 = new QFrame(vtktoolswidget);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);

        verticalLayout_2->addWidget(line_3);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);


        retranslateUi(vtktoolswidget);

        QMetaObject::connectSlotsByName(vtktoolswidget);
    } // setupUi

    void retranslateUi(QWidget *vtktoolswidget)
    {
        vtktoolswidget->setWindowTitle(QCoreApplication::translate("vtktoolswidget", "View Setting", nullptr));
        label_3->setText(QCoreApplication::translate("vtktoolswidget", "Camera", nullptr));
        cameraLeft->setText(QString());
        cameraBack->setText(QString());
        cameraRight->setText(QString());
        frontCamera->setText(QString());
        topCamera->setText(QString());
        bottomCamera->setText(QString());
        ShowBoxCheckBox->setText(QCoreApplication::translate("vtktoolswidget", "Show Box", nullptr));
        ShowAxesCheckBox->setText(QCoreApplication::translate("vtktoolswidget", "Show Axes", nullptr));
        ShowColorbarCheckBox->setText(QCoreApplication::translate("vtktoolswidget", "Show Colorbar", nullptr));
        scaleCheckBox->setText(QCoreApplication::translate("vtktoolswidget", "Scale", nullptr));
        gridCheckBox->setText(QCoreApplication::translate("vtktoolswidget", "Grid", nullptr));
        label_5->setText(QCoreApplication::translate("vtktoolswidget", "Look Up Table", nullptr));
        activateLutCheckBox->setText(QCoreApplication::translate("vtktoolswidget", "Activate LUT", nullptr));
        scalarLabel->setText(QCoreApplication::translate("vtktoolswidget", "Scalar", nullptr));
        lutLabel->setText(QCoreApplication::translate("vtktoolswidget", "LUT", nullptr));
        lutComboBox->setItemText(0, QCoreApplication::translate("vtktoolswidget", "Default", nullptr));
        lutComboBox->setItemText(1, QCoreApplication::translate("vtktoolswidget", "Default Step", nullptr));
        lutComboBox->setItemText(2, QCoreApplication::translate("vtktoolswidget", "EField", nullptr));
        lutComboBox->setItemText(3, QCoreApplication::translate("vtktoolswidget", "Glow", nullptr));
        lutComboBox->setItemText(4, QCoreApplication::translate("vtktoolswidget", "Gray", nullptr));
        lutComboBox->setItemText(5, QCoreApplication::translate("vtktoolswidget", "MinMax", nullptr));
        lutComboBox->setItemText(6, QCoreApplication::translate("vtktoolswidget", "PhysicsContour", nullptr));
        lutComboBox->setItemText(7, QCoreApplication::translate("vtktoolswidget", "PureRed", nullptr));
        lutComboBox->setItemText(8, QCoreApplication::translate("vtktoolswidget", "PureGreen", nullptr));
        lutComboBox->setItemText(9, QCoreApplication::translate("vtktoolswidget", "Blue", nullptr));
        lutComboBox->setItemText(10, QCoreApplication::translate("vtktoolswidget", "Run1", nullptr));
        lutComboBox->setItemText(11, QCoreApplication::translate("vtktoolswidget", "Run2", nullptr));
        lutComboBox->setItemText(12, QCoreApplication::translate("vtktoolswidget", "Sar", nullptr));
        lutComboBox->setItemText(13, QCoreApplication::translate("vtktoolswidget", "Temperature", nullptr));
        lutComboBox->setItemText(14, QCoreApplication::translate("vtktoolswidget", "TenStep", nullptr));
        lutComboBox->setItemText(15, QCoreApplication::translate("vtktoolswidget", "VolRenGlow", nullptr));
        lutComboBox->setItemText(16, QCoreApplication::translate("vtktoolswidget", "VolRenGreen", nullptr));
        lutComboBox->setItemText(17, QCoreApplication::translate("vtktoolswidget", "VolRenRGB", nullptr));
        lutComboBox->setItemText(18, QCoreApplication::translate("vtktoolswidget", "VolRenTwoLev", nullptr));
        lutComboBox->setItemText(19, QCoreApplication::translate("vtktoolswidget", "AllYellow", nullptr));
        lutComboBox->setItemText(20, QCoreApplication::translate("vtktoolswidget", "AllCyan", nullptr));
        lutComboBox->setItemText(21, QCoreApplication::translate("vtktoolswidget", "AllViolet", nullptr));
        lutComboBox->setItemText(22, QCoreApplication::translate("vtktoolswidget", "AllWhite", nullptr));
        lutComboBox->setItemText(23, QCoreApplication::translate("vtktoolswidget", "AllBlack", nullptr));
        lutComboBox->setItemText(24, QCoreApplication::translate("vtktoolswidget", "AllRed", nullptr));
        lutComboBox->setItemText(25, QCoreApplication::translate("vtktoolswidget", "AllGreen", nullptr));
        lutComboBox->setItemText(26, QCoreApplication::translate("vtktoolswidget", "AllBlue", nullptr));

        label_4->setText(QCoreApplication::translate("vtktoolswidget", "Range", nullptr));
        label->setText(QCoreApplication::translate("vtktoolswidget", "Min:", nullptr));
        label_2->setText(QCoreApplication::translate("vtktoolswidget", "Max:", nullptr));
    } // retranslateUi

};

namespace Ui {
    class vtktoolswidget: public Ui_vtktoolswidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VTKTOOLSWIDGET_H
