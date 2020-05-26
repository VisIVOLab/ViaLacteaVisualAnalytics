/********************************************************************************
** Form generated from reading UI file 'vtkwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VTKWINDOW_H
#define UI_VTKWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "QVTKWidget.h"

QT_BEGIN_NAMESPACE

class Ui_vtkwindow
{
public:
    QAction *actionTools;
    QAction *actionClose;
    QAction *actionInfo;
    QWidget *win;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QRadioButton *PVPlot_radioButton;
    QPushButton *PVPlotPushButton;
    QPushButton *generatePushButton;
    QPushButton *cameraLeft;
    QLabel *label_survey;
    QLineEdit *lineEdit_survey;
    QPushButton *cameraBack;
    QLabel *label_species;
    QLineEdit *lineEdit_species;
    QPushButton *cameraRight;
    QPushButton *frontCamera;
    QLabel *label_transition;
    QLineEdit *lineEdit_transition;
    QPushButton *topCamera;
    QPushButton *contour_pushButton;
    QPushButton *bottomCamera;
    QLabel *label_threshold;
    QSlider *horizontalSlider_threshold;
    QLabel *label_2;
    QSlider *cuttingPlane_Slider;
    QSpinBox *spinBox_cuttingPlane;
    QLabel *label_channel;
    QSpinBox *spinBox_contour;
    QLabel *label_channels;
    QSpinBox *spinBox_channels;
    QLabel *selectionLabel;
    QPushButton *rectSelection;
    QPushButton *freehandPushButton;
    QSpacerItem *horizontalSpacer_2;
    QPushButton *resetPushButton;
    QPushButton *confirmPushButton;
    QVTKWidget *qVTK1;
    QMenuBar *menubar;
    QMenu *menuWindow;
    QMenu *menuFile;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *vtkwindow)
    {
        if (vtkwindow->objectName().isEmpty())
            vtkwindow->setObjectName(QString::fromUtf8("vtkwindow"));
        vtkwindow->resize(1893, 767);
        actionTools = new QAction(vtkwindow);
        actionTools->setObjectName(QString::fromUtf8("actionTools"));
        actionClose = new QAction(vtkwindow);
        actionClose->setObjectName(QString::fromUtf8("actionClose"));
        actionInfo = new QAction(vtkwindow);
        actionInfo->setObjectName(QString::fromUtf8("actionInfo"));
        win = new QWidget(vtkwindow);
        win->setObjectName(QString::fromUtf8("win"));
        verticalLayout_2 = new QVBoxLayout(win);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        PVPlot_radioButton = new QRadioButton(win);
        PVPlot_radioButton->setObjectName(QString::fromUtf8("PVPlot_radioButton"));

        horizontalLayout->addWidget(PVPlot_radioButton);

        PVPlotPushButton = new QPushButton(win);
        PVPlotPushButton->setObjectName(QString::fromUtf8("PVPlotPushButton"));

        horizontalLayout->addWidget(PVPlotPushButton);

        generatePushButton = new QPushButton(win);
        generatePushButton->setObjectName(QString::fromUtf8("generatePushButton"));

        horizontalLayout->addWidget(generatePushButton);

        cameraLeft = new QPushButton(win);
        cameraLeft->setObjectName(QString::fromUtf8("cameraLeft"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/PIC_LEFT.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        cameraLeft->setIcon(icon);

        horizontalLayout->addWidget(cameraLeft);

        label_survey = new QLabel(win);
        label_survey->setObjectName(QString::fromUtf8("label_survey"));

        horizontalLayout->addWidget(label_survey);

        lineEdit_survey = new QLineEdit(win);
        lineEdit_survey->setObjectName(QString::fromUtf8("lineEdit_survey"));

        horizontalLayout->addWidget(lineEdit_survey);

        cameraBack = new QPushButton(win);
        cameraBack->setObjectName(QString::fromUtf8("cameraBack"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/PIC_BACK.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        cameraBack->setIcon(icon1);

        horizontalLayout->addWidget(cameraBack);

        label_species = new QLabel(win);
        label_species->setObjectName(QString::fromUtf8("label_species"));

        horizontalLayout->addWidget(label_species);

        lineEdit_species = new QLineEdit(win);
        lineEdit_species->setObjectName(QString::fromUtf8("lineEdit_species"));

        horizontalLayout->addWidget(lineEdit_species);

        cameraRight = new QPushButton(win);
        cameraRight->setObjectName(QString::fromUtf8("cameraRight"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/PIC_RIGHT.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        cameraRight->setIcon(icon2);

        horizontalLayout->addWidget(cameraRight);

        frontCamera = new QPushButton(win);
        frontCamera->setObjectName(QString::fromUtf8("frontCamera"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/PIC_FRONT.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        frontCamera->setIcon(icon3);

        horizontalLayout->addWidget(frontCamera);

        label_transition = new QLabel(win);
        label_transition->setObjectName(QString::fromUtf8("label_transition"));

        horizontalLayout->addWidget(label_transition);

        lineEdit_transition = new QLineEdit(win);
        lineEdit_transition->setObjectName(QString::fromUtf8("lineEdit_transition"));

        horizontalLayout->addWidget(lineEdit_transition);

        topCamera = new QPushButton(win);
        topCamera->setObjectName(QString::fromUtf8("topCamera"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icons/PIC_TOP.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        topCamera->setIcon(icon4);

        horizontalLayout->addWidget(topCamera);

        contour_pushButton = new QPushButton(win);
        contour_pushButton->setObjectName(QString::fromUtf8("contour_pushButton"));

        horizontalLayout->addWidget(contour_pushButton);

        bottomCamera = new QPushButton(win);
        bottomCamera->setObjectName(QString::fromUtf8("bottomCamera"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/icons/PIC_BOTTOM.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        bottomCamera->setIcon(icon5);

        horizontalLayout->addWidget(bottomCamera);

        label_threshold = new QLabel(win);
        label_threshold->setObjectName(QString::fromUtf8("label_threshold"));

        horizontalLayout->addWidget(label_threshold);

        horizontalSlider_threshold = new QSlider(win);
        horizontalSlider_threshold->setObjectName(QString::fromUtf8("horizontalSlider_threshold"));
        horizontalSlider_threshold->setOrientation(Qt::Horizontal);
        horizontalSlider_threshold->setTickPosition(QSlider::TicksAbove);
        horizontalSlider_threshold->setTickInterval(1);

        horizontalLayout->addWidget(horizontalSlider_threshold);

        label_2 = new QLabel(win);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout->addWidget(label_2);

        cuttingPlane_Slider = new QSlider(win);
        cuttingPlane_Slider->setObjectName(QString::fromUtf8("cuttingPlane_Slider"));
        cuttingPlane_Slider->setOrientation(Qt::Horizontal);
        cuttingPlane_Slider->setTickPosition(QSlider::TicksAbove);

        horizontalLayout->addWidget(cuttingPlane_Slider);

        spinBox_cuttingPlane = new QSpinBox(win);
        spinBox_cuttingPlane->setObjectName(QString::fromUtf8("spinBox_cuttingPlane"));

        horizontalLayout->addWidget(spinBox_cuttingPlane);

        label_channel = new QLabel(win);
        label_channel->setObjectName(QString::fromUtf8("label_channel"));

        horizontalLayout->addWidget(label_channel);

        spinBox_contour = new QSpinBox(win);
        spinBox_contour->setObjectName(QString::fromUtf8("spinBox_contour"));

        horizontalLayout->addWidget(spinBox_contour);

        label_channels = new QLabel(win);
        label_channels->setObjectName(QString::fromUtf8("label_channels"));

        horizontalLayout->addWidget(label_channels);

        spinBox_channels = new QSpinBox(win);
        spinBox_channels->setObjectName(QString::fromUtf8("spinBox_channels"));

        horizontalLayout->addWidget(spinBox_channels);

        selectionLabel = new QLabel(win);
        selectionLabel->setObjectName(QString::fromUtf8("selectionLabel"));

        horizontalLayout->addWidget(selectionLabel);

        rectSelection = new QPushButton(win);
        rectSelection->setObjectName(QString::fromUtf8("rectSelection"));

        horizontalLayout->addWidget(rectSelection);

        freehandPushButton = new QPushButton(win);
        freehandPushButton->setObjectName(QString::fromUtf8("freehandPushButton"));

        horizontalLayout->addWidget(freehandPushButton);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        resetPushButton = new QPushButton(win);
        resetPushButton->setObjectName(QString::fromUtf8("resetPushButton"));

        horizontalLayout->addWidget(resetPushButton);

        confirmPushButton = new QPushButton(win);
        confirmPushButton->setObjectName(QString::fromUtf8("confirmPushButton"));

        horizontalLayout->addWidget(confirmPushButton);


        verticalLayout_2->addLayout(horizontalLayout);

        qVTK1 = new QVTKWidget(win);
        qVTK1->setObjectName(QString::fromUtf8("qVTK1"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(qVTK1->sizePolicy().hasHeightForWidth());
        qVTK1->setSizePolicy(sizePolicy);
        qVTK1->setMouseTracking(false);

        verticalLayout_2->addWidget(qVTK1);

        vtkwindow->setCentralWidget(win);
        menubar = new QMenuBar(vtkwindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1893, 22));
        menuWindow = new QMenu(menubar);
        menuWindow->setObjectName(QString::fromUtf8("menuWindow"));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        vtkwindow->setMenuBar(menubar);
        statusbar = new QStatusBar(vtkwindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        vtkwindow->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuWindow->menuAction());
        menuWindow->addAction(actionTools);
        menuWindow->addAction(actionInfo);

        retranslateUi(vtkwindow);

        QMetaObject::connectSlotsByName(vtkwindow);
    } // setupUi

    void retranslateUi(QMainWindow *vtkwindow)
    {
        vtkwindow->setWindowTitle(QCoreApplication::translate("vtkwindow", "MainWindow", nullptr));
        actionTools->setText(QCoreApplication::translate("vtkwindow", "Tools", nullptr));
#if QT_CONFIG(shortcut)
        actionTools->setShortcut(QCoreApplication::translate("vtkwindow", "Ctrl+T", nullptr));
#endif // QT_CONFIG(shortcut)
        actionClose->setText(QCoreApplication::translate("vtkwindow", "Close", nullptr));
        actionInfo->setText(QCoreApplication::translate("vtkwindow", "Info", nullptr));
        PVPlot_radioButton->setText(QCoreApplication::translate("vtkwindow", "PV plot", nullptr));
        PVPlotPushButton->setText(QCoreApplication::translate("vtkwindow", "PV plot", nullptr));
        generatePushButton->setText(QCoreApplication::translate("vtkwindow", "Gen", nullptr));
        cameraLeft->setText(QString());
        label_survey->setText(QCoreApplication::translate("vtkwindow", "Survey", nullptr));
        cameraBack->setText(QString());
        label_species->setText(QCoreApplication::translate("vtkwindow", "Species", nullptr));
        cameraRight->setText(QString());
        frontCamera->setText(QString());
        label_transition->setText(QCoreApplication::translate("vtkwindow", "Transition", nullptr));
        topCamera->setText(QString());
        contour_pushButton->setText(QCoreApplication::translate("vtkwindow", "Contour", nullptr));
        bottomCamera->setText(QString());
        label_threshold->setText(QCoreApplication::translate("vtkwindow", "Threshold", nullptr));
        label_2->setText(QCoreApplication::translate("vtkwindow", "Cutting plane", nullptr));
        label_channel->setText(QCoreApplication::translate("vtkwindow", "Channel", nullptr));
        label_channels->setText(QCoreApplication::translate("vtkwindow", "Range", nullptr));
        selectionLabel->setText(QCoreApplication::translate("vtkwindow", "Selection:", nullptr));
        rectSelection->setText(QCoreApplication::translate("vtkwindow", "Rect", nullptr));
        freehandPushButton->setText(QCoreApplication::translate("vtkwindow", "Free", nullptr));
        resetPushButton->setText(QCoreApplication::translate("vtkwindow", "Reset", nullptr));
        confirmPushButton->setText(QCoreApplication::translate("vtkwindow", "Confirm", nullptr));
        menuWindow->setTitle(QCoreApplication::translate("vtkwindow", "Window", nullptr));
        menuFile->setTitle(QCoreApplication::translate("vtkwindow", "File", nullptr));
    } // retranslateUi

};

namespace Ui {
    class vtkwindow: public Ui_vtkwindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VTKWINDOW_H
