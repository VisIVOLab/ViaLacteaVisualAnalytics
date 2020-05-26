/********************************************************************************
** Form generated from reading UI file 'vtkwindow_new.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VTKWINDOW_NEW_H
#define UI_VTKWINDOW_NEW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "QVTKWidget.h"

QT_BEGIN_NAMESPACE

class Ui_vtkwindow_new
{
public:
    QAction *actionTools;
    QAction *actionInfo;
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout_6;
    QSplitter *splitter_2;
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout_6;
    QHBoxLayout *horizontalLayout;
    QGroupBox *compactSourcesGroupBox;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *rectangularSelectionCS;
    QGroupBox *datacubeGroupBox;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *pushButton_2;
    QPushButton *pushButton_3;
    QGroupBox *filamentsGroupBox;
    QHBoxLayout *horizontalLayout_8;
    QPushButton *fil_rectPushButton;
    QGroupBox *bubbleGroupBox;
    QHBoxLayout *horizontalLayout_4;
    QPushButton *bubblePushButton;
    QGroupBox *tdGroupBox;
    QHBoxLayout *horizontalLayout_9;
    QPushButton *tdRectPushButton;
    QGroupBox *toolsGroupBox;
    QVBoxLayout *verticalLayout_12;
    QHBoxLayout *LutLayout;
    QComboBox *lutComboBox;
    QRadioButton *linearadioButton;
    QRadioButton *logRadioButton;
    QHBoxLayout *horizontalLayout_19;
    QSlider *horizontalSlider;
    QGroupBox *lut3dGroupBox;
    QHBoxLayout *horizontalLayout_16;
    QComboBox *scalarComboBox;
    QComboBox *lut3dComboBox;
    QRadioButton *linear3dRadioButton;
    QRadioButton *log3dRadioButton;
    QToolButton *toolButton;
    QCheckBox *lut3dActivateCheckBox;
    QGroupBox *glyphGroupBox;
    QHBoxLayout *horizontalLayout_18;
    QCheckBox *glyphActivateCheckBox;
    QComboBox *glyphShapeComboBox;
    QComboBox *glyphScalarComboBox;
    QLabel *label_9;
    QLineEdit *glyphScalingLineEdit;
    QGroupBox *filterGroupBox;
    QHBoxLayout *horizontalLayout_20;
    QToolButton *filterMoreButton;
    QRadioButton *PVPlot_radioButton;
    QPushButton *PVPlotPushButton;
    QLabel *label_survey;
    QLineEdit *lineEdit_survey;
    QLabel *label_species;
    QLineEdit *lineEdit_species;
    QLabel *label_transition;
    QLineEdit *lineEdit_transition;
    QGroupBox *cameraControlgroupBox;
    QHBoxLayout *horizontalLayout_5;
    QPushButton *cameraLeft;
    QPushButton *cameraRight;
    QPushButton *cameraBack;
    QPushButton *frontCamera;
    QPushButton *topCamera;
    QPushButton *bottomCamera;
    QGroupBox *ThresholdGroupBox;
    QVBoxLayout *verticalLayout_8;
    QSlider *horizontalSlider_threshold;
    QHBoxLayout *horizontalLayout_13;
    QSpacerItem *horizontalSpacer_3;
    QLineEdit *thresholdValueLineEdit;
    QSpacerItem *horizontalSpacer_4;
    QGroupBox *cuttingPlaneGroupBox;
    QVBoxLayout *verticalLayout_9;
    QSlider *cuttingPlane_Slider;
    QHBoxLayout *horizontalLayout_11;
    QSpacerItem *horizontalSpacer;
    QSpinBox *spinBox_cuttingPlane;
    QLineEdit *velocityLineEdit;
    QSpacerItem *horizontalSpacer_2;
    QGroupBox *contourGroupBox;
    QHBoxLayout *horizontalLayout_14;
    QVBoxLayout *verticalLayout_4;
    QLabel *label_4;
    QLineEdit *levelLineEdit;
    QVBoxLayout *verticalLayout_5;
    QLabel *label_5;
    QLineEdit *lowerBoundLineEdit;
    QVBoxLayout *verticalLayout_7;
    QLabel *label_6;
    QLineEdit *upperBoundLineEdit;
    QCheckBox *contourCheckBox;
    QLabel *label_channels;
    QSpinBox *spinBox_channels;
    QGroupBox *valueGroupBox;
    QHBoxLayout *horizontalLayout_12;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout_15;
    QVBoxLayout *verticalLayout_2;
    QLabel *label;
    QLineEdit *minLineEdit;
    QVBoxLayout *verticalLayout;
    QLabel *label_3;
    QLineEdit *maxLineEdit;
    QVBoxLayout *verticalLayout_3;
    QLabel *label_2;
    QLineEdit *RmsLineEdit;
    QGroupBox *groupBox_2;
    QHBoxLayout *horizontalLayout_17;
    QVBoxLayout *verticalLayout_11;
    QLabel *label_8;
    QLineEdit *minSliceLineEdit;
    QVBoxLayout *verticalLayout_10;
    QLabel *label_7;
    QLineEdit *maxSliceLineEdit;
    QGroupBox *selectionGroupBox;
    QHBoxLayout *horizontalLayout_10;
    QPushButton *rectSelection;
    QPushButton *freehandPushButton;
    QPushButton *confirmPushButton;
    QPushButton *resetPushButton;
    QHBoxLayout *horizontalLayout_7;
    QVTKWidget *qVTK1;
    QVTKWidget *isocontourVtkWin;
    QSplitter *splitter;
    QTableWidget *ElementListWidget;
    QTableWidget *tableWidget;
    QListWidget *listWidget;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuWindow;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *vtkwindow_new)
    {
        if (vtkwindow_new->objectName().isEmpty())
            vtkwindow_new->setObjectName(QString::fromUtf8("vtkwindow_new"));
        vtkwindow_new->resize(3990, 625);
        QFont font;
        font.setPointSize(11);
        vtkwindow_new->setFont(font);
        actionTools = new QAction(vtkwindow_new);
        actionTools->setObjectName(QString::fromUtf8("actionTools"));
        actionTools->setVisible(false);
        actionInfo = new QAction(vtkwindow_new);
        actionInfo->setObjectName(QString::fromUtf8("actionInfo"));
        centralwidget = new QWidget(vtkwindow_new);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        horizontalLayout_6 = new QHBoxLayout(centralwidget);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        splitter_2 = new QSplitter(centralwidget);
        splitter_2->setObjectName(QString::fromUtf8("splitter_2"));
        splitter_2->setOrientation(Qt::Horizontal);
        layoutWidget = new QWidget(splitter_2);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        verticalLayout_6 = new QVBoxLayout(layoutWidget);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        verticalLayout_6->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        compactSourcesGroupBox = new QGroupBox(layoutWidget);
        compactSourcesGroupBox->setObjectName(QString::fromUtf8("compactSourcesGroupBox"));
        horizontalLayout_2 = new QHBoxLayout(compactSourcesGroupBox);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        rectangularSelectionCS = new QPushButton(compactSourcesGroupBox);
        rectangularSelectionCS->setObjectName(QString::fromUtf8("rectangularSelectionCS"));
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(rectangularSelectionCS->sizePolicy().hasHeightForWidth());
        rectangularSelectionCS->setSizePolicy(sizePolicy);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/rect-select.png"), QSize(), QIcon::Normal, QIcon::Off);
        rectangularSelectionCS->setIcon(icon);
        rectangularSelectionCS->setIconSize(QSize(50, 20));

        horizontalLayout_2->addWidget(rectangularSelectionCS);


        horizontalLayout->addWidget(compactSourcesGroupBox);

        datacubeGroupBox = new QGroupBox(layoutWidget);
        datacubeGroupBox->setObjectName(QString::fromUtf8("datacubeGroupBox"));
        horizontalLayout_3 = new QHBoxLayout(datacubeGroupBox);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        pushButton_2 = new QPushButton(datacubeGroupBox);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));
        pushButton_2->setIcon(icon);
        pushButton_2->setIconSize(QSize(50, 20));

        horizontalLayout_3->addWidget(pushButton_2);

        pushButton_3 = new QPushButton(datacubeGroupBox);
        pushButton_3->setObjectName(QString::fromUtf8("pushButton_3"));
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/ellipse-select.png"), QSize(), QIcon::Normal, QIcon::Off);
        pushButton_3->setIcon(icon1);
        pushButton_3->setIconSize(QSize(50, 20));

        horizontalLayout_3->addWidget(pushButton_3);


        horizontalLayout->addWidget(datacubeGroupBox);

        filamentsGroupBox = new QGroupBox(layoutWidget);
        filamentsGroupBox->setObjectName(QString::fromUtf8("filamentsGroupBox"));
        horizontalLayout_8 = new QHBoxLayout(filamentsGroupBox);
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        fil_rectPushButton = new QPushButton(filamentsGroupBox);
        fil_rectPushButton->setObjectName(QString::fromUtf8("fil_rectPushButton"));
        sizePolicy.setHeightForWidth(fil_rectPushButton->sizePolicy().hasHeightForWidth());
        fil_rectPushButton->setSizePolicy(sizePolicy);
        fil_rectPushButton->setIcon(icon);
        fil_rectPushButton->setIconSize(QSize(50, 20));

        horizontalLayout_8->addWidget(fil_rectPushButton);


        horizontalLayout->addWidget(filamentsGroupBox);

        bubbleGroupBox = new QGroupBox(layoutWidget);
        bubbleGroupBox->setObjectName(QString::fromUtf8("bubbleGroupBox"));
        horizontalLayout_4 = new QHBoxLayout(bubbleGroupBox);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        bubblePushButton = new QPushButton(bubbleGroupBox);
        bubblePushButton->setObjectName(QString::fromUtf8("bubblePushButton"));
        sizePolicy.setHeightForWidth(bubblePushButton->sizePolicy().hasHeightForWidth());
        bubblePushButton->setSizePolicy(sizePolicy);
        bubblePushButton->setIcon(icon);
        bubblePushButton->setIconSize(QSize(50, 20));

        horizontalLayout_4->addWidget(bubblePushButton);


        horizontalLayout->addWidget(bubbleGroupBox);

        tdGroupBox = new QGroupBox(layoutWidget);
        tdGroupBox->setObjectName(QString::fromUtf8("tdGroupBox"));
        horizontalLayout_9 = new QHBoxLayout(tdGroupBox);
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        tdRectPushButton = new QPushButton(tdGroupBox);
        tdRectPushButton->setObjectName(QString::fromUtf8("tdRectPushButton"));
        sizePolicy.setHeightForWidth(tdRectPushButton->sizePolicy().hasHeightForWidth());
        tdRectPushButton->setSizePolicy(sizePolicy);
        tdRectPushButton->setIcon(icon);
        tdRectPushButton->setIconSize(QSize(50, 20));

        horizontalLayout_9->addWidget(tdRectPushButton);


        horizontalLayout->addWidget(tdGroupBox);

        toolsGroupBox = new QGroupBox(layoutWidget);
        toolsGroupBox->setObjectName(QString::fromUtf8("toolsGroupBox"));
        verticalLayout_12 = new QVBoxLayout(toolsGroupBox);
        verticalLayout_12->setObjectName(QString::fromUtf8("verticalLayout_12"));
        LutLayout = new QHBoxLayout();
        LutLayout->setObjectName(QString::fromUtf8("LutLayout"));
        lutComboBox = new QComboBox(toolsGroupBox);
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

        LutLayout->addWidget(lutComboBox);

        linearadioButton = new QRadioButton(toolsGroupBox);
        linearadioButton->setObjectName(QString::fromUtf8("linearadioButton"));

        LutLayout->addWidget(linearadioButton);

        logRadioButton = new QRadioButton(toolsGroupBox);
        logRadioButton->setObjectName(QString::fromUtf8("logRadioButton"));
        logRadioButton->setChecked(true);

        LutLayout->addWidget(logRadioButton);


        verticalLayout_12->addLayout(LutLayout);

        horizontalLayout_19 = new QHBoxLayout();
        horizontalLayout_19->setObjectName(QString::fromUtf8("horizontalLayout_19"));
        horizontalSlider = new QSlider(toolsGroupBox);
        horizontalSlider->setObjectName(QString::fromUtf8("horizontalSlider"));
        horizontalSlider->setValue(99);
        horizontalSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_19->addWidget(horizontalSlider);


        verticalLayout_12->addLayout(horizontalLayout_19);


        horizontalLayout->addWidget(toolsGroupBox);

        lut3dGroupBox = new QGroupBox(layoutWidget);
        lut3dGroupBox->setObjectName(QString::fromUtf8("lut3dGroupBox"));
        QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(lut3dGroupBox->sizePolicy().hasHeightForWidth());
        lut3dGroupBox->setSizePolicy(sizePolicy1);
        lut3dGroupBox->setMaximumSize(QSize(400, 16777215));
        horizontalLayout_16 = new QHBoxLayout(lut3dGroupBox);
        horizontalLayout_16->setObjectName(QString::fromUtf8("horizontalLayout_16"));
        scalarComboBox = new QComboBox(lut3dGroupBox);
        scalarComboBox->setObjectName(QString::fromUtf8("scalarComboBox"));
        scalarComboBox->setEnabled(false);

        horizontalLayout_16->addWidget(scalarComboBox);

        lut3dComboBox = new QComboBox(lut3dGroupBox);
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->addItem(QString());
        lut3dComboBox->setObjectName(QString::fromUtf8("lut3dComboBox"));
        lut3dComboBox->setEnabled(false);

        horizontalLayout_16->addWidget(lut3dComboBox);

        linear3dRadioButton = new QRadioButton(lut3dGroupBox);
        linear3dRadioButton->setObjectName(QString::fromUtf8("linear3dRadioButton"));
        linear3dRadioButton->setEnabled(false);
        linear3dRadioButton->setChecked(true);

        horizontalLayout_16->addWidget(linear3dRadioButton);

        log3dRadioButton = new QRadioButton(lut3dGroupBox);
        log3dRadioButton->setObjectName(QString::fromUtf8("log3dRadioButton"));
        log3dRadioButton->setEnabled(false);

        horizontalLayout_16->addWidget(log3dRadioButton);

        toolButton = new QToolButton(lut3dGroupBox);
        toolButton->setObjectName(QString::fromUtf8("toolButton"));
        toolButton->setEnabled(false);

        horizontalLayout_16->addWidget(toolButton);

        lut3dActivateCheckBox = new QCheckBox(lut3dGroupBox);
        lut3dActivateCheckBox->setObjectName(QString::fromUtf8("lut3dActivateCheckBox"));

        horizontalLayout_16->addWidget(lut3dActivateCheckBox);


        horizontalLayout->addWidget(lut3dGroupBox);

        glyphGroupBox = new QGroupBox(layoutWidget);
        glyphGroupBox->setObjectName(QString::fromUtf8("glyphGroupBox"));
        sizePolicy1.setHeightForWidth(glyphGroupBox->sizePolicy().hasHeightForWidth());
        glyphGroupBox->setSizePolicy(sizePolicy1);
        glyphGroupBox->setMaximumSize(QSize(400, 16777215));
        horizontalLayout_18 = new QHBoxLayout(glyphGroupBox);
        horizontalLayout_18->setObjectName(QString::fromUtf8("horizontalLayout_18"));
        glyphActivateCheckBox = new QCheckBox(glyphGroupBox);
        glyphActivateCheckBox->setObjectName(QString::fromUtf8("glyphActivateCheckBox"));
        glyphActivateCheckBox->setEnabled(true);

        horizontalLayout_18->addWidget(glyphActivateCheckBox);

        glyphShapeComboBox = new QComboBox(glyphGroupBox);
        glyphShapeComboBox->addItem(QString());
        glyphShapeComboBox->addItem(QString());
        glyphShapeComboBox->addItem(QString());
        glyphShapeComboBox->addItem(QString());
        glyphShapeComboBox->setObjectName(QString::fromUtf8("glyphShapeComboBox"));
        glyphShapeComboBox->setEnabled(false);

        horizontalLayout_18->addWidget(glyphShapeComboBox);

        glyphScalarComboBox = new QComboBox(glyphGroupBox);
        glyphScalarComboBox->setObjectName(QString::fromUtf8("glyphScalarComboBox"));
        glyphScalarComboBox->setEnabled(false);

        horizontalLayout_18->addWidget(glyphScalarComboBox);

        label_9 = new QLabel(glyphGroupBox);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        horizontalLayout_18->addWidget(label_9);

        glyphScalingLineEdit = new QLineEdit(glyphGroupBox);
        glyphScalingLineEdit->setObjectName(QString::fromUtf8("glyphScalingLineEdit"));
        glyphScalingLineEdit->setEnabled(false);

        horizontalLayout_18->addWidget(glyphScalingLineEdit);


        horizontalLayout->addWidget(glyphGroupBox);

        filterGroupBox = new QGroupBox(layoutWidget);
        filterGroupBox->setObjectName(QString::fromUtf8("filterGroupBox"));
        horizontalLayout_20 = new QHBoxLayout(filterGroupBox);
        horizontalLayout_20->setObjectName(QString::fromUtf8("horizontalLayout_20"));
        filterMoreButton = new QToolButton(filterGroupBox);
        filterMoreButton->setObjectName(QString::fromUtf8("filterMoreButton"));
        filterMoreButton->setEnabled(true);

        horizontalLayout_20->addWidget(filterMoreButton);


        horizontalLayout->addWidget(filterGroupBox);

        PVPlot_radioButton = new QRadioButton(layoutWidget);
        PVPlot_radioButton->setObjectName(QString::fromUtf8("PVPlot_radioButton"));

        horizontalLayout->addWidget(PVPlot_radioButton);

        PVPlotPushButton = new QPushButton(layoutWidget);
        PVPlotPushButton->setObjectName(QString::fromUtf8("PVPlotPushButton"));

        horizontalLayout->addWidget(PVPlotPushButton);

        label_survey = new QLabel(layoutWidget);
        label_survey->setObjectName(QString::fromUtf8("label_survey"));

        horizontalLayout->addWidget(label_survey);

        lineEdit_survey = new QLineEdit(layoutWidget);
        lineEdit_survey->setObjectName(QString::fromUtf8("lineEdit_survey"));

        horizontalLayout->addWidget(lineEdit_survey);

        label_species = new QLabel(layoutWidget);
        label_species->setObjectName(QString::fromUtf8("label_species"));

        horizontalLayout->addWidget(label_species);

        lineEdit_species = new QLineEdit(layoutWidget);
        lineEdit_species->setObjectName(QString::fromUtf8("lineEdit_species"));

        horizontalLayout->addWidget(lineEdit_species);

        label_transition = new QLabel(layoutWidget);
        label_transition->setObjectName(QString::fromUtf8("label_transition"));

        horizontalLayout->addWidget(label_transition);

        lineEdit_transition = new QLineEdit(layoutWidget);
        lineEdit_transition->setObjectName(QString::fromUtf8("lineEdit_transition"));

        horizontalLayout->addWidget(lineEdit_transition);

        cameraControlgroupBox = new QGroupBox(layoutWidget);
        cameraControlgroupBox->setObjectName(QString::fromUtf8("cameraControlgroupBox"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(cameraControlgroupBox->sizePolicy().hasHeightForWidth());
        cameraControlgroupBox->setSizePolicy(sizePolicy2);
        cameraControlgroupBox->setMaximumSize(QSize(200, 16777215));
        horizontalLayout_5 = new QHBoxLayout(cameraControlgroupBox);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        cameraLeft = new QPushButton(cameraControlgroupBox);
        cameraLeft->setObjectName(QString::fromUtf8("cameraLeft"));
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/PIC_LEFT.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        cameraLeft->setIcon(icon2);

        horizontalLayout_5->addWidget(cameraLeft);

        cameraRight = new QPushButton(cameraControlgroupBox);
        cameraRight->setObjectName(QString::fromUtf8("cameraRight"));
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/PIC_RIGHT.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        cameraRight->setIcon(icon3);

        horizontalLayout_5->addWidget(cameraRight);

        cameraBack = new QPushButton(cameraControlgroupBox);
        cameraBack->setObjectName(QString::fromUtf8("cameraBack"));
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icons/PIC_BACK.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        cameraBack->setIcon(icon4);

        horizontalLayout_5->addWidget(cameraBack);

        frontCamera = new QPushButton(cameraControlgroupBox);
        frontCamera->setObjectName(QString::fromUtf8("frontCamera"));
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/icons/PIC_FRONT.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        frontCamera->setIcon(icon5);

        horizontalLayout_5->addWidget(frontCamera);

        topCamera = new QPushButton(cameraControlgroupBox);
        topCamera->setObjectName(QString::fromUtf8("topCamera"));
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/icons/PIC_TOP.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        topCamera->setIcon(icon6);

        horizontalLayout_5->addWidget(topCamera);

        bottomCamera = new QPushButton(cameraControlgroupBox);
        bottomCamera->setObjectName(QString::fromUtf8("bottomCamera"));
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/icons/PIC_BOTTOM.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        bottomCamera->setIcon(icon7);

        horizontalLayout_5->addWidget(bottomCamera);


        horizontalLayout->addWidget(cameraControlgroupBox);

        ThresholdGroupBox = new QGroupBox(layoutWidget);
        ThresholdGroupBox->setObjectName(QString::fromUtf8("ThresholdGroupBox"));
        sizePolicy2.setHeightForWidth(ThresholdGroupBox->sizePolicy().hasHeightForWidth());
        ThresholdGroupBox->setSizePolicy(sizePolicy2);
        ThresholdGroupBox->setMaximumSize(QSize(200, 16777215));
        verticalLayout_8 = new QVBoxLayout(ThresholdGroupBox);
        verticalLayout_8->setObjectName(QString::fromUtf8("verticalLayout_8"));
        horizontalSlider_threshold = new QSlider(ThresholdGroupBox);
        horizontalSlider_threshold->setObjectName(QString::fromUtf8("horizontalSlider_threshold"));
        horizontalSlider_threshold->setOrientation(Qt::Horizontal);
        horizontalSlider_threshold->setTickPosition(QSlider::TicksAbove);
        horizontalSlider_threshold->setTickInterval(0);

        verticalLayout_8->addWidget(horizontalSlider_threshold);

        horizontalLayout_13 = new QHBoxLayout();
        horizontalLayout_13->setObjectName(QString::fromUtf8("horizontalLayout_13"));
        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_13->addItem(horizontalSpacer_3);

        thresholdValueLineEdit = new QLineEdit(ThresholdGroupBox);
        thresholdValueLineEdit->setObjectName(QString::fromUtf8("thresholdValueLineEdit"));
        thresholdValueLineEdit->setEnabled(true);
        sizePolicy.setHeightForWidth(thresholdValueLineEdit->sizePolicy().hasHeightForWidth());
        thresholdValueLineEdit->setSizePolicy(sizePolicy);
        thresholdValueLineEdit->setMaximumSize(QSize(50, 16777215));

        horizontalLayout_13->addWidget(thresholdValueLineEdit);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_13->addItem(horizontalSpacer_4);


        verticalLayout_8->addLayout(horizontalLayout_13);


        horizontalLayout->addWidget(ThresholdGroupBox);

        cuttingPlaneGroupBox = new QGroupBox(layoutWidget);
        cuttingPlaneGroupBox->setObjectName(QString::fromUtf8("cuttingPlaneGroupBox"));
        sizePolicy2.setHeightForWidth(cuttingPlaneGroupBox->sizePolicy().hasHeightForWidth());
        cuttingPlaneGroupBox->setSizePolicy(sizePolicy2);
        cuttingPlaneGroupBox->setMaximumSize(QSize(200, 16777215));
        verticalLayout_9 = new QVBoxLayout(cuttingPlaneGroupBox);
        verticalLayout_9->setObjectName(QString::fromUtf8("verticalLayout_9"));
        cuttingPlane_Slider = new QSlider(cuttingPlaneGroupBox);
        cuttingPlane_Slider->setObjectName(QString::fromUtf8("cuttingPlane_Slider"));
        cuttingPlane_Slider->setOrientation(Qt::Horizontal);
        cuttingPlane_Slider->setTickPosition(QSlider::TicksAbove);
        cuttingPlane_Slider->setTickInterval(0);

        verticalLayout_9->addWidget(cuttingPlane_Slider);

        horizontalLayout_11 = new QHBoxLayout();
        horizontalLayout_11->setObjectName(QString::fromUtf8("horizontalLayout_11"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_11->addItem(horizontalSpacer);

        spinBox_cuttingPlane = new QSpinBox(cuttingPlaneGroupBox);
        spinBox_cuttingPlane->setObjectName(QString::fromUtf8("spinBox_cuttingPlane"));
        sizePolicy.setHeightForWidth(spinBox_cuttingPlane->sizePolicy().hasHeightForWidth());
        spinBox_cuttingPlane->setSizePolicy(sizePolicy);
        spinBox_cuttingPlane->setMaximumSize(QSize(80, 16777215));

        horizontalLayout_11->addWidget(spinBox_cuttingPlane);

        velocityLineEdit = new QLineEdit(cuttingPlaneGroupBox);
        velocityLineEdit->setObjectName(QString::fromUtf8("velocityLineEdit"));
        velocityLineEdit->setEnabled(false);
        sizePolicy.setHeightForWidth(velocityLineEdit->sizePolicy().hasHeightForWidth());
        velocityLineEdit->setSizePolicy(sizePolicy);
        velocityLineEdit->setMaximumSize(QSize(90, 16777215));

        horizontalLayout_11->addWidget(velocityLineEdit);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_11->addItem(horizontalSpacer_2);


        verticalLayout_9->addLayout(horizontalLayout_11);


        horizontalLayout->addWidget(cuttingPlaneGroupBox);

        contourGroupBox = new QGroupBox(layoutWidget);
        contourGroupBox->setObjectName(QString::fromUtf8("contourGroupBox"));
        sizePolicy2.setHeightForWidth(contourGroupBox->sizePolicy().hasHeightForWidth());
        contourGroupBox->setSizePolicy(sizePolicy2);
        contourGroupBox->setMaximumSize(QSize(250, 16777215));
        contourGroupBox->setFont(font);
        horizontalLayout_14 = new QHBoxLayout(contourGroupBox);
        horizontalLayout_14->setObjectName(QString::fromUtf8("horizontalLayout_14"));
        verticalLayout_4 = new QVBoxLayout();
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        label_4 = new QLabel(contourGroupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        sizePolicy1.setHeightForWidth(label_4->sizePolicy().hasHeightForWidth());
        label_4->setSizePolicy(sizePolicy1);
        label_4->setMaximumSize(QSize(60, 16777215));
        label_4->setFont(font);

        verticalLayout_4->addWidget(label_4);

        levelLineEdit = new QLineEdit(contourGroupBox);
        levelLineEdit->setObjectName(QString::fromUtf8("levelLineEdit"));
        levelLineEdit->setMaximumSize(QSize(60, 16777215));
        levelLineEdit->setMaxLength(4);

        verticalLayout_4->addWidget(levelLineEdit);


        horizontalLayout_14->addLayout(verticalLayout_4);

        verticalLayout_5 = new QVBoxLayout();
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        label_5 = new QLabel(contourGroupBox);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        sizePolicy1.setHeightForWidth(label_5->sizePolicy().hasHeightForWidth());
        label_5->setSizePolicy(sizePolicy1);
        label_5->setMaximumSize(QSize(80, 16777215));
        label_5->setFont(font);

        verticalLayout_5->addWidget(label_5);

        lowerBoundLineEdit = new QLineEdit(contourGroupBox);
        lowerBoundLineEdit->setObjectName(QString::fromUtf8("lowerBoundLineEdit"));
        lowerBoundLineEdit->setMaximumSize(QSize(80, 16777215));

        verticalLayout_5->addWidget(lowerBoundLineEdit);


        horizontalLayout_14->addLayout(verticalLayout_5);

        verticalLayout_7 = new QVBoxLayout();
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        label_6 = new QLabel(contourGroupBox);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setMaximumSize(QSize(80, 16777215));
        label_6->setFont(font);

        verticalLayout_7->addWidget(label_6);

        upperBoundLineEdit = new QLineEdit(contourGroupBox);
        upperBoundLineEdit->setObjectName(QString::fromUtf8("upperBoundLineEdit"));
        upperBoundLineEdit->setMaximumSize(QSize(80, 16777215));

        verticalLayout_7->addWidget(upperBoundLineEdit);


        horizontalLayout_14->addLayout(verticalLayout_7);

        contourCheckBox = new QCheckBox(contourGroupBox);
        contourCheckBox->setObjectName(QString::fromUtf8("contourCheckBox"));

        horizontalLayout_14->addWidget(contourCheckBox);


        horizontalLayout->addWidget(contourGroupBox);

        label_channels = new QLabel(layoutWidget);
        label_channels->setObjectName(QString::fromUtf8("label_channels"));

        horizontalLayout->addWidget(label_channels);

        spinBox_channels = new QSpinBox(layoutWidget);
        spinBox_channels->setObjectName(QString::fromUtf8("spinBox_channels"));

        horizontalLayout->addWidget(spinBox_channels);

        valueGroupBox = new QGroupBox(layoutWidget);
        valueGroupBox->setObjectName(QString::fromUtf8("valueGroupBox"));
        horizontalLayout_12 = new QHBoxLayout(valueGroupBox);
        horizontalLayout_12->setObjectName(QString::fromUtf8("horizontalLayout_12"));
        groupBox = new QGroupBox(valueGroupBox);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        horizontalLayout_15 = new QHBoxLayout(groupBox);
        horizontalLayout_15->setObjectName(QString::fromUtf8("horizontalLayout_15"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        label->setMaximumSize(QSize(80, 16777215));

        verticalLayout_2->addWidget(label);

        minLineEdit = new QLineEdit(groupBox);
        minLineEdit->setObjectName(QString::fromUtf8("minLineEdit"));
        minLineEdit->setEnabled(false);
        minLineEdit->setMaximumSize(QSize(80, 16777215));

        verticalLayout_2->addWidget(minLineEdit);


        horizontalLayout_15->addLayout(verticalLayout_2);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setMaximumSize(QSize(80, 16777215));

        verticalLayout->addWidget(label_3);

        maxLineEdit = new QLineEdit(groupBox);
        maxLineEdit->setObjectName(QString::fromUtf8("maxLineEdit"));
        maxLineEdit->setEnabled(false);
        maxLineEdit->setMaximumSize(QSize(80, 16777215));

        verticalLayout->addWidget(maxLineEdit);


        horizontalLayout_15->addLayout(verticalLayout);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setMaximumSize(QSize(80, 16777215));

        verticalLayout_3->addWidget(label_2);

        RmsLineEdit = new QLineEdit(groupBox);
        RmsLineEdit->setObjectName(QString::fromUtf8("RmsLineEdit"));
        RmsLineEdit->setEnabled(false);
        RmsLineEdit->setMaximumSize(QSize(80, 16777215));

        verticalLayout_3->addWidget(RmsLineEdit);


        horizontalLayout_15->addLayout(verticalLayout_3);


        horizontalLayout_12->addWidget(groupBox);

        groupBox_2 = new QGroupBox(valueGroupBox);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        horizontalLayout_17 = new QHBoxLayout(groupBox_2);
        horizontalLayout_17->setObjectName(QString::fromUtf8("horizontalLayout_17"));
        verticalLayout_11 = new QVBoxLayout();
        verticalLayout_11->setObjectName(QString::fromUtf8("verticalLayout_11"));
        label_8 = new QLabel(groupBox_2);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setMaximumSize(QSize(80, 16777215));

        verticalLayout_11->addWidget(label_8);

        minSliceLineEdit = new QLineEdit(groupBox_2);
        minSliceLineEdit->setObjectName(QString::fromUtf8("minSliceLineEdit"));
        minSliceLineEdit->setEnabled(false);
        minSliceLineEdit->setMaximumSize(QSize(80, 16777215));

        verticalLayout_11->addWidget(minSliceLineEdit);


        horizontalLayout_17->addLayout(verticalLayout_11);

        verticalLayout_10 = new QVBoxLayout();
        verticalLayout_10->setObjectName(QString::fromUtf8("verticalLayout_10"));
        label_7 = new QLabel(groupBox_2);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setMaximumSize(QSize(80, 16777215));

        verticalLayout_10->addWidget(label_7);

        maxSliceLineEdit = new QLineEdit(groupBox_2);
        maxSliceLineEdit->setObjectName(QString::fromUtf8("maxSliceLineEdit"));
        maxSliceLineEdit->setEnabled(false);
        maxSliceLineEdit->setMaximumSize(QSize(80, 16777215));

        verticalLayout_10->addWidget(maxSliceLineEdit);


        horizontalLayout_17->addLayout(verticalLayout_10);


        horizontalLayout_12->addWidget(groupBox_2);


        horizontalLayout->addWidget(valueGroupBox);

        selectionGroupBox = new QGroupBox(layoutWidget);
        selectionGroupBox->setObjectName(QString::fromUtf8("selectionGroupBox"));
        horizontalLayout_10 = new QHBoxLayout(selectionGroupBox);
        horizontalLayout_10->setObjectName(QString::fromUtf8("horizontalLayout_10"));
        rectSelection = new QPushButton(selectionGroupBox);
        rectSelection->setObjectName(QString::fromUtf8("rectSelection"));

        horizontalLayout_10->addWidget(rectSelection);

        freehandPushButton = new QPushButton(selectionGroupBox);
        freehandPushButton->setObjectName(QString::fromUtf8("freehandPushButton"));

        horizontalLayout_10->addWidget(freehandPushButton);

        confirmPushButton = new QPushButton(selectionGroupBox);
        confirmPushButton->setObjectName(QString::fromUtf8("confirmPushButton"));

        horizontalLayout_10->addWidget(confirmPushButton);

        resetPushButton = new QPushButton(selectionGroupBox);
        resetPushButton->setObjectName(QString::fromUtf8("resetPushButton"));

        horizontalLayout_10->addWidget(resetPushButton);


        horizontalLayout->addWidget(selectionGroupBox);


        verticalLayout_6->addLayout(horizontalLayout);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        qVTK1 = new QVTKWidget(layoutWidget);
        qVTK1->setObjectName(QString::fromUtf8("qVTK1"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(qVTK1->sizePolicy().hasHeightForWidth());
        qVTK1->setSizePolicy(sizePolicy3);

        horizontalLayout_7->addWidget(qVTK1);

        isocontourVtkWin = new QVTKWidget(layoutWidget);
        isocontourVtkWin->setObjectName(QString::fromUtf8("isocontourVtkWin"));
        sizePolicy3.setHeightForWidth(isocontourVtkWin->sizePolicy().hasHeightForWidth());
        isocontourVtkWin->setSizePolicy(sizePolicy3);

        horizontalLayout_7->addWidget(isocontourVtkWin);


        verticalLayout_6->addLayout(horizontalLayout_7);

        splitter_2->addWidget(layoutWidget);

        horizontalLayout_6->addWidget(splitter_2);

        splitter = new QSplitter(centralwidget);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setOrientation(Qt::Vertical);
        ElementListWidget = new QTableWidget(splitter);
        if (ElementListWidget->columnCount() < 2)
            ElementListWidget->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        ElementListWidget->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        ElementListWidget->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        ElementListWidget->setObjectName(QString::fromUtf8("ElementListWidget"));
        ElementListWidget->setColumnCount(2);
        splitter->addWidget(ElementListWidget);
        ElementListWidget->horizontalHeader()->setVisible(false);
        ElementListWidget->horizontalHeader()->setStretchLastSection(false);
        ElementListWidget->verticalHeader()->setVisible(false);
        tableWidget = new QTableWidget(splitter);
        if (tableWidget->columnCount() < 2)
            tableWidget->setColumnCount(2);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        tableWidget->setHorizontalHeaderItem(1, __qtablewidgetitem3);
        tableWidget->setObjectName(QString::fromUtf8("tableWidget"));
        sizePolicy3.setHeightForWidth(tableWidget->sizePolicy().hasHeightForWidth());
        tableWidget->setSizePolicy(sizePolicy3);
        tableWidget->setMinimumSize(QSize(0, 0));
        tableWidget->setMaximumSize(QSize(16777215, 16777215));
        splitter->addWidget(tableWidget);
        tableWidget->horizontalHeader()->setVisible(false);
        tableWidget->horizontalHeader()->setStretchLastSection(false);
        tableWidget->verticalHeader()->setStretchLastSection(false);
        listWidget = new QListWidget(splitter);
        listWidget->setObjectName(QString::fromUtf8("listWidget"));
        listWidget->setAlternatingRowColors(true);
        splitter->addWidget(listWidget);

        horizontalLayout_6->addWidget(splitter);

        vtkwindow_new->setCentralWidget(centralwidget);
        menubar = new QMenuBar(vtkwindow_new);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 3990, 22));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuWindow = new QMenu(menubar);
        menuWindow->setObjectName(QString::fromUtf8("menuWindow"));
        vtkwindow_new->setMenuBar(menubar);
        statusbar = new QStatusBar(vtkwindow_new);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        vtkwindow_new->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuWindow->menuAction());
        menuWindow->addAction(actionTools);
        menuWindow->addAction(actionInfo);

        retranslateUi(vtkwindow_new);

        QMetaObject::connectSlotsByName(vtkwindow_new);
    } // setupUi

    void retranslateUi(QMainWindow *vtkwindow_new)
    {
        vtkwindow_new->setWindowTitle(QCoreApplication::translate("vtkwindow_new", "MainWindow", nullptr));
        actionTools->setText(QCoreApplication::translate("vtkwindow_new", "Tools", nullptr));
        actionInfo->setText(QCoreApplication::translate("vtkwindow_new", "Info", nullptr));
        compactSourcesGroupBox->setTitle(QCoreApplication::translate("vtkwindow_new", "Compact souces", nullptr));
        rectangularSelectionCS->setText(QString());
        datacubeGroupBox->setTitle(QCoreApplication::translate("vtkwindow_new", "Datacubes", nullptr));
        pushButton_2->setText(QString());
        pushButton_3->setText(QString());
        filamentsGroupBox->setTitle(QCoreApplication::translate("vtkwindow_new", "Filaments", nullptr));
        fil_rectPushButton->setText(QString());
        bubbleGroupBox->setTitle(QCoreApplication::translate("vtkwindow_new", "Bubble", nullptr));
        bubblePushButton->setText(QString());
        tdGroupBox->setTitle(QCoreApplication::translate("vtkwindow_new", "3D", nullptr));
        tdRectPushButton->setText(QString());
        toolsGroupBox->setTitle(QCoreApplication::translate("vtkwindow_new", "Layer setting", nullptr));
        lutComboBox->setItemText(0, QCoreApplication::translate("vtkwindow_new", "Gray", nullptr));
        lutComboBox->setItemText(1, QCoreApplication::translate("vtkwindow_new", "Default", nullptr));
        lutComboBox->setItemText(2, QCoreApplication::translate("vtkwindow_new", "Default Step", nullptr));
        lutComboBox->setItemText(3, QCoreApplication::translate("vtkwindow_new", "EField", nullptr));
        lutComboBox->setItemText(4, QCoreApplication::translate("vtkwindow_new", "Glow", nullptr));
        lutComboBox->setItemText(5, QCoreApplication::translate("vtkwindow_new", "MinMax", nullptr));
        lutComboBox->setItemText(6, QCoreApplication::translate("vtkwindow_new", "PhysicsContour", nullptr));
        lutComboBox->setItemText(7, QCoreApplication::translate("vtkwindow_new", "PureRed", nullptr));
        lutComboBox->setItemText(8, QCoreApplication::translate("vtkwindow_new", "PureGreen", nullptr));
        lutComboBox->setItemText(9, QCoreApplication::translate("vtkwindow_new", "PureBlue", nullptr));
        lutComboBox->setItemText(10, QCoreApplication::translate("vtkwindow_new", "Run1", nullptr));
        lutComboBox->setItemText(11, QCoreApplication::translate("vtkwindow_new", "Run2", nullptr));
        lutComboBox->setItemText(12, QCoreApplication::translate("vtkwindow_new", "Sar", nullptr));
        lutComboBox->setItemText(13, QCoreApplication::translate("vtkwindow_new", "Temperature", nullptr));
        lutComboBox->setItemText(14, QCoreApplication::translate("vtkwindow_new", "TenStep", nullptr));
        lutComboBox->setItemText(15, QCoreApplication::translate("vtkwindow_new", "VolRenGlow", nullptr));
        lutComboBox->setItemText(16, QCoreApplication::translate("vtkwindow_new", "VolRenGreen", nullptr));
        lutComboBox->setItemText(17, QCoreApplication::translate("vtkwindow_new", "VolRenRGB", nullptr));
        lutComboBox->setItemText(18, QCoreApplication::translate("vtkwindow_new", "VolRenTwoLev", nullptr));
        lutComboBox->setItemText(19, QCoreApplication::translate("vtkwindow_new", "AllYellow", nullptr));
        lutComboBox->setItemText(20, QCoreApplication::translate("vtkwindow_new", "AllCyan", nullptr));
        lutComboBox->setItemText(21, QCoreApplication::translate("vtkwindow_new", "AllViolet", nullptr));
        lutComboBox->setItemText(22, QCoreApplication::translate("vtkwindow_new", "AllWhite", nullptr));
        lutComboBox->setItemText(23, QCoreApplication::translate("vtkwindow_new", "AllBlack", nullptr));
        lutComboBox->setItemText(24, QCoreApplication::translate("vtkwindow_new", "AllRed", nullptr));
        lutComboBox->setItemText(25, QCoreApplication::translate("vtkwindow_new", "AllGreen", nullptr));
        lutComboBox->setItemText(26, QCoreApplication::translate("vtkwindow_new", "AllBlu", nullptr));

        linearadioButton->setText(QCoreApplication::translate("vtkwindow_new", "Linear", nullptr));
        logRadioButton->setText(QCoreApplication::translate("vtkwindow_new", "Log", nullptr));
        lut3dGroupBox->setTitle(QCoreApplication::translate("vtkwindow_new", "Lut", nullptr));
        lut3dComboBox->setItemText(0, QCoreApplication::translate("vtkwindow_new", "Gray", nullptr));
        lut3dComboBox->setItemText(1, QCoreApplication::translate("vtkwindow_new", "Default", nullptr));
        lut3dComboBox->setItemText(2, QCoreApplication::translate("vtkwindow_new", "Default Step", nullptr));
        lut3dComboBox->setItemText(3, QCoreApplication::translate("vtkwindow_new", "EField", nullptr));
        lut3dComboBox->setItemText(4, QCoreApplication::translate("vtkwindow_new", "Glow", nullptr));
        lut3dComboBox->setItemText(5, QCoreApplication::translate("vtkwindow_new", "MinMax", nullptr));
        lut3dComboBox->setItemText(6, QCoreApplication::translate("vtkwindow_new", "PhysicsContour", nullptr));
        lut3dComboBox->setItemText(7, QCoreApplication::translate("vtkwindow_new", "PureRed", nullptr));
        lut3dComboBox->setItemText(8, QCoreApplication::translate("vtkwindow_new", "PureGreen", nullptr));
        lut3dComboBox->setItemText(9, QCoreApplication::translate("vtkwindow_new", "PureBlue", nullptr));
        lut3dComboBox->setItemText(10, QCoreApplication::translate("vtkwindow_new", "Run1", nullptr));
        lut3dComboBox->setItemText(11, QCoreApplication::translate("vtkwindow_new", "Run2", nullptr));
        lut3dComboBox->setItemText(12, QCoreApplication::translate("vtkwindow_new", "Sar", nullptr));
        lut3dComboBox->setItemText(13, QCoreApplication::translate("vtkwindow_new", "Temperature", nullptr));
        lut3dComboBox->setItemText(14, QCoreApplication::translate("vtkwindow_new", "TenStep", nullptr));
        lut3dComboBox->setItemText(15, QCoreApplication::translate("vtkwindow_new", "VolRenGlow", nullptr));
        lut3dComboBox->setItemText(16, QCoreApplication::translate("vtkwindow_new", "VolRenGreen", nullptr));
        lut3dComboBox->setItemText(17, QCoreApplication::translate("vtkwindow_new", "VolRenRGB", nullptr));
        lut3dComboBox->setItemText(18, QCoreApplication::translate("vtkwindow_new", "VolRenTwoLev", nullptr));
        lut3dComboBox->setItemText(19, QCoreApplication::translate("vtkwindow_new", "AllYellow", nullptr));
        lut3dComboBox->setItemText(20, QCoreApplication::translate("vtkwindow_new", "AllCyan", nullptr));
        lut3dComboBox->setItemText(21, QCoreApplication::translate("vtkwindow_new", "AllViolet", nullptr));
        lut3dComboBox->setItemText(22, QCoreApplication::translate("vtkwindow_new", "AllWhite", nullptr));
        lut3dComboBox->setItemText(23, QCoreApplication::translate("vtkwindow_new", "AllBlack", nullptr));
        lut3dComboBox->setItemText(24, QCoreApplication::translate("vtkwindow_new", "AllRed", nullptr));
        lut3dComboBox->setItemText(25, QCoreApplication::translate("vtkwindow_new", "AllGreen", nullptr));
        lut3dComboBox->setItemText(26, QCoreApplication::translate("vtkwindow_new", "AllBlu", nullptr));

        linear3dRadioButton->setText(QCoreApplication::translate("vtkwindow_new", "Linear", nullptr));
        log3dRadioButton->setText(QCoreApplication::translate("vtkwindow_new", "Log", nullptr));
        toolButton->setText(QCoreApplication::translate("vtkwindow_new", "...", nullptr));
        lut3dActivateCheckBox->setText(QString());
        glyphGroupBox->setTitle(QCoreApplication::translate("vtkwindow_new", "Glyph", nullptr));
        glyphActivateCheckBox->setText(QString());
        glyphShapeComboBox->setItemText(0, QCoreApplication::translate("vtkwindow_new", "Sphere", nullptr));
        glyphShapeComboBox->setItemText(1, QCoreApplication::translate("vtkwindow_new", "Cone", nullptr));
        glyphShapeComboBox->setItemText(2, QCoreApplication::translate("vtkwindow_new", "Cylinder", nullptr));
        glyphShapeComboBox->setItemText(3, QCoreApplication::translate("vtkwindow_new", "Cube", nullptr));

        label_9->setText(QCoreApplication::translate("vtkwindow_new", "Scaling Factor:", nullptr));
#if QT_CONFIG(tooltip)
        glyphScalingLineEdit->setToolTip(QCoreApplication::translate("vtkwindow_new", "<html><head/><body><p>Glyph Scaling Factor</p></body></html>", nullptr));
#endif // QT_CONFIG(tooltip)
        glyphScalingLineEdit->setText(QCoreApplication::translate("vtkwindow_new", "0.5", nullptr));
        filterGroupBox->setTitle(QCoreApplication::translate("vtkwindow_new", "Filter", nullptr));
        filterMoreButton->setText(QCoreApplication::translate("vtkwindow_new", "...", nullptr));
        PVPlot_radioButton->setText(QCoreApplication::translate("vtkwindow_new", "PV plot", nullptr));
        PVPlotPushButton->setText(QCoreApplication::translate("vtkwindow_new", "PV plot", nullptr));
        label_survey->setText(QCoreApplication::translate("vtkwindow_new", "Survey", nullptr));
        label_species->setText(QCoreApplication::translate("vtkwindow_new", "Species", nullptr));
        label_transition->setText(QCoreApplication::translate("vtkwindow_new", "Transition", nullptr));
        cameraControlgroupBox->setTitle(QCoreApplication::translate("vtkwindow_new", "Camera control", nullptr));
        cameraLeft->setText(QString());
        cameraRight->setText(QString());
        cameraBack->setText(QString());
        frontCamera->setText(QString());
        topCamera->setText(QString());
        bottomCamera->setText(QString());
        ThresholdGroupBox->setTitle(QCoreApplication::translate("vtkwindow_new", "Threshold", nullptr));
        cuttingPlaneGroupBox->setTitle(QCoreApplication::translate("vtkwindow_new", "Cutting plane", nullptr));
        contourGroupBox->setTitle(QCoreApplication::translate("vtkwindow_new", "Contours", nullptr));
        label_4->setText(QCoreApplication::translate("vtkwindow_new", "Level", nullptr));
        levelLineEdit->setText(QCoreApplication::translate("vtkwindow_new", "15", nullptr));
        label_5->setText(QCoreApplication::translate("vtkwindow_new", "Lower bound", nullptr));
        label_6->setText(QCoreApplication::translate("vtkwindow_new", "Upper bound", nullptr));
        contourCheckBox->setText(QString());
        label_channels->setText(QCoreApplication::translate("vtkwindow_new", "Range", nullptr));
        valueGroupBox->setTitle(QCoreApplication::translate("vtkwindow_new", "Value", nullptr));
        groupBox->setTitle(QCoreApplication::translate("vtkwindow_new", "Datacube", nullptr));
        label->setText(QCoreApplication::translate("vtkwindow_new", "Min", nullptr));
        label_3->setText(QCoreApplication::translate("vtkwindow_new", "Max", nullptr));
        label_2->setText(QCoreApplication::translate("vtkwindow_new", "RMS", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("vtkwindow_new", "Slice", nullptr));
        label_8->setText(QCoreApplication::translate("vtkwindow_new", "Min", nullptr));
        label_7->setText(QCoreApplication::translate("vtkwindow_new", "Max", nullptr));
        selectionGroupBox->setTitle(QCoreApplication::translate("vtkwindow_new", "Selection", nullptr));
        rectSelection->setText(QCoreApplication::translate("vtkwindow_new", "Rect", nullptr));
        freehandPushButton->setText(QCoreApplication::translate("vtkwindow_new", "Free", nullptr));
        confirmPushButton->setText(QCoreApplication::translate("vtkwindow_new", "Confirm", nullptr));
        resetPushButton->setText(QCoreApplication::translate("vtkwindow_new", "Reset", nullptr));
        QTableWidgetItem *___qtablewidgetitem = ElementListWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("vtkwindow_new", "Survey", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = ElementListWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("vtkwindow_new", "Species", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = tableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("vtkwindow_new", "Selected", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = tableWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("vtkwindow_new", "Name", nullptr));
        menuFile->setTitle(QCoreApplication::translate("vtkwindow_new", "File", nullptr));
        menuWindow->setTitle(QCoreApplication::translate("vtkwindow_new", "Window", nullptr));
    } // retranslateUi

};

namespace Ui {
    class vtkwindow_new: public Ui_vtkwindow_new {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VTKWINDOW_NEW_H
