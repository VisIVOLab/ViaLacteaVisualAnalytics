/********************************************************************************
** Form generated from reading UI file 'vialactea.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VIALACTEA_H
#define UI_VIALACTEA_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWebKitWidgets/QWebView>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ViaLactea
{
public:
    QAction *actionSettings;
    QAction *actionExit;
    QAction *actionAbout;
    QAction *actionLoad_SED;
    QAction *actionLoad_SED_2;
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QSplitter *splitter;
    QWebView *webView;
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_5;
    QGroupBox *groupBox_2;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *openLocalImagePushButton;
    QGroupBox *groupBox_3;
    QHBoxLayout *horizontalLayout_6;
    QPushButton *localDCPushButton;
    QGroupBox *groupBox_4;
    QVBoxLayout *verticalLayout_5;
    QPushButton *select3dPushButton;
    QGroupBox *surveySelectorGroupBox;
    QVBoxLayout *verticalLayout_11;
    QGridLayout *gridLayout_2;
    QGroupBox *groupBox_5;
    QVBoxLayout *verticalLayout_6;
    QCheckBox *glimpse_24_checkBox;
    QCheckBox *glimpse_8_checkBox;
    QCheckBox *glimpse_58_checkBox;
    QCheckBox *glimpse_45_checkBox;
    QCheckBox *glimpse_36checkBox;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QCheckBox *PLW_checkBox_2;
    QCheckBox *PMW_checkBox;
    QCheckBox *PSW_checkBox_12;
    QCheckBox *blue_checkBox_2;
    QCheckBox *red_checkBox;
    QGroupBox *groupBox_6;
    QVBoxLayout *verticalLayout_7;
    QCheckBox *wise_22_checkBox;
    QCheckBox *wise_12_checkBox;
    QCheckBox *wise_46_checkBox;
    QCheckBox *wise_34_checkBox;
    QGridLayout *gridLayout_3;
    QGroupBox *groupBox_7;
    QVBoxLayout *verticalLayout_8;
    QCheckBox *atlasgal_870_checkBox;
    QGroupBox *groupBox_8;
    QVBoxLayout *verticalLayout_9;
    QCheckBox *bolocam_11_checkBox;
    QGroupBox *groupBox_9;
    QVBoxLayout *verticalLayout_10;
    QCheckBox *cornish_6_checkBox;
    QGroupBox *selectionGroupBox;
    QVBoxLayout *verticalLayout_3;
    QRadioButton *noneRadioButton;
    QRadioButton *pointRadioButton;
    QRadioButton *rectRadioButton;
    QGroupBox *coordinateGroupBox;
    QVBoxLayout *verticalLayout_4;
    QLabel *label;
    QLineEdit *glonLineEdit;
    QLabel *label_3;
    QLineEdit *glatLineEdit;
    QGridLayout *gridLayout;
    QLabel *dlLabel;
    QLabel *label_2;
    QLineEdit *radiumLineEdit;
    QLabel *dbLabel;
    QLineEdit *dlLineEdit;
    QLineEdit *dbLineEdit;
    QCheckBox *saveToDiskCheckBox;
    QHBoxLayout *horizontalLayout_2;
    QLineEdit *fileNameLineEdit;
    QPushButton *selectFsPushButton;
    QSpacerItem *verticalSpacer;
    QPushButton *queryPushButton;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuAiuto;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *ViaLactea)
    {
        if (ViaLactea->objectName().isEmpty())
            ViaLactea->setObjectName(QString::fromUtf8("ViaLactea"));
        ViaLactea->resize(1266, 835);
        actionSettings = new QAction(ViaLactea);
        actionSettings->setObjectName(QString::fromUtf8("actionSettings"));
        actionExit = new QAction(ViaLactea);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        actionAbout = new QAction(ViaLactea);
        actionAbout->setObjectName(QString::fromUtf8("actionAbout"));
        actionLoad_SED = new QAction(ViaLactea);
        actionLoad_SED->setObjectName(QString::fromUtf8("actionLoad_SED"));
        actionLoad_SED_2 = new QAction(ViaLactea);
        actionLoad_SED_2->setObjectName(QString::fromUtf8("actionLoad_SED_2"));
        centralwidget = new QWidget(ViaLactea);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        splitter = new QSplitter(centralwidget);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(splitter->sizePolicy().hasHeightForWidth());
        splitter->setSizePolicy(sizePolicy);
        splitter->setMinimumSize(QSize(0, 0));
        splitter->setOrientation(Qt::Horizontal);
        webView = new QWebView(splitter);
        webView->setObjectName(QString::fromUtf8("webView"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(webView->sizePolicy().hasHeightForWidth());
        webView->setSizePolicy(sizePolicy1);
        webView->setMinimumSize(QSize(800, 0));
        webView->setProperty("url", QVariant(QUrl(QString::fromUtf8("about:blank"))));
        splitter->addWidget(webView);
        layoutWidget = new QWidget(splitter);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        verticalLayout = new QVBoxLayout(layoutWidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        groupBox_2 = new QGroupBox(layoutWidget);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        horizontalLayout_3 = new QHBoxLayout(groupBox_2);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        openLocalImagePushButton = new QPushButton(groupBox_2);
        openLocalImagePushButton->setObjectName(QString::fromUtf8("openLocalImagePushButton"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/FILE_OPEN.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        openLocalImagePushButton->setIcon(icon);

        horizontalLayout_3->addWidget(openLocalImagePushButton);


        horizontalLayout_5->addWidget(groupBox_2);

        groupBox_3 = new QGroupBox(layoutWidget);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        horizontalLayout_6 = new QHBoxLayout(groupBox_3);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        localDCPushButton = new QPushButton(groupBox_3);
        localDCPushButton->setObjectName(QString::fromUtf8("localDCPushButton"));
        localDCPushButton->setIcon(icon);

        horizontalLayout_6->addWidget(localDCPushButton);


        horizontalLayout_5->addWidget(groupBox_3);

        groupBox_4 = new QGroupBox(layoutWidget);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        verticalLayout_5 = new QVBoxLayout(groupBox_4);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        select3dPushButton = new QPushButton(groupBox_4);
        select3dPushButton->setObjectName(QString::fromUtf8("select3dPushButton"));

        verticalLayout_5->addWidget(select3dPushButton);


        horizontalLayout_5->addWidget(groupBox_4);


        verticalLayout->addLayout(horizontalLayout_5);

        surveySelectorGroupBox = new QGroupBox(layoutWidget);
        surveySelectorGroupBox->setObjectName(QString::fromUtf8("surveySelectorGroupBox"));
        verticalLayout_11 = new QVBoxLayout(surveySelectorGroupBox);
        verticalLayout_11->setObjectName(QString::fromUtf8("verticalLayout_11"));
        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        groupBox_5 = new QGroupBox(surveySelectorGroupBox);
        groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
        verticalLayout_6 = new QVBoxLayout(groupBox_5);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        glimpse_24_checkBox = new QCheckBox(groupBox_5);
        glimpse_24_checkBox->setObjectName(QString::fromUtf8("glimpse_24_checkBox"));

        verticalLayout_6->addWidget(glimpse_24_checkBox);

        glimpse_8_checkBox = new QCheckBox(groupBox_5);
        glimpse_8_checkBox->setObjectName(QString::fromUtf8("glimpse_8_checkBox"));

        verticalLayout_6->addWidget(glimpse_8_checkBox);

        glimpse_58_checkBox = new QCheckBox(groupBox_5);
        glimpse_58_checkBox->setObjectName(QString::fromUtf8("glimpse_58_checkBox"));

        verticalLayout_6->addWidget(glimpse_58_checkBox);

        glimpse_45_checkBox = new QCheckBox(groupBox_5);
        glimpse_45_checkBox->setObjectName(QString::fromUtf8("glimpse_45_checkBox"));

        verticalLayout_6->addWidget(glimpse_45_checkBox);

        glimpse_36checkBox = new QCheckBox(groupBox_5);
        glimpse_36checkBox->setObjectName(QString::fromUtf8("glimpse_36checkBox"));

        verticalLayout_6->addWidget(glimpse_36checkBox);


        gridLayout_2->addWidget(groupBox_5, 1, 2, 1, 1);

        groupBox = new QGroupBox(surveySelectorGroupBox);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setEnabled(true);
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        PLW_checkBox_2 = new QCheckBox(groupBox);
        PLW_checkBox_2->setObjectName(QString::fromUtf8("PLW_checkBox_2"));

        verticalLayout_2->addWidget(PLW_checkBox_2);

        PMW_checkBox = new QCheckBox(groupBox);
        PMW_checkBox->setObjectName(QString::fromUtf8("PMW_checkBox"));

        verticalLayout_2->addWidget(PMW_checkBox);

        PSW_checkBox_12 = new QCheckBox(groupBox);
        PSW_checkBox_12->setObjectName(QString::fromUtf8("PSW_checkBox_12"));
        PSW_checkBox_12->setChecked(true);

        verticalLayout_2->addWidget(PSW_checkBox_12);

        blue_checkBox_2 = new QCheckBox(groupBox);
        blue_checkBox_2->setObjectName(QString::fromUtf8("blue_checkBox_2"));

        verticalLayout_2->addWidget(blue_checkBox_2);

        red_checkBox = new QCheckBox(groupBox);
        red_checkBox->setObjectName(QString::fromUtf8("red_checkBox"));

        verticalLayout_2->addWidget(red_checkBox);


        gridLayout_2->addWidget(groupBox, 1, 0, 1, 1);

        groupBox_6 = new QGroupBox(surveySelectorGroupBox);
        groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
        verticalLayout_7 = new QVBoxLayout(groupBox_6);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        wise_22_checkBox = new QCheckBox(groupBox_6);
        wise_22_checkBox->setObjectName(QString::fromUtf8("wise_22_checkBox"));

        verticalLayout_7->addWidget(wise_22_checkBox);

        wise_12_checkBox = new QCheckBox(groupBox_6);
        wise_12_checkBox->setObjectName(QString::fromUtf8("wise_12_checkBox"));

        verticalLayout_7->addWidget(wise_12_checkBox);

        wise_46_checkBox = new QCheckBox(groupBox_6);
        wise_46_checkBox->setObjectName(QString::fromUtf8("wise_46_checkBox"));

        verticalLayout_7->addWidget(wise_46_checkBox);

        wise_34_checkBox = new QCheckBox(groupBox_6);
        wise_34_checkBox->setObjectName(QString::fromUtf8("wise_34_checkBox"));

        verticalLayout_7->addWidget(wise_34_checkBox);


        gridLayout_2->addWidget(groupBox_6, 1, 3, 1, 1);


        verticalLayout_11->addLayout(gridLayout_2);

        gridLayout_3 = new QGridLayout();
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        groupBox_7 = new QGroupBox(surveySelectorGroupBox);
        groupBox_7->setObjectName(QString::fromUtf8("groupBox_7"));
        verticalLayout_8 = new QVBoxLayout(groupBox_7);
        verticalLayout_8->setObjectName(QString::fromUtf8("verticalLayout_8"));
        atlasgal_870_checkBox = new QCheckBox(groupBox_7);
        atlasgal_870_checkBox->setObjectName(QString::fromUtf8("atlasgal_870_checkBox"));

        verticalLayout_8->addWidget(atlasgal_870_checkBox);


        gridLayout_3->addWidget(groupBox_7, 0, 0, 1, 1);

        groupBox_8 = new QGroupBox(surveySelectorGroupBox);
        groupBox_8->setObjectName(QString::fromUtf8("groupBox_8"));
        verticalLayout_9 = new QVBoxLayout(groupBox_8);
        verticalLayout_9->setObjectName(QString::fromUtf8("verticalLayout_9"));
        bolocam_11_checkBox = new QCheckBox(groupBox_8);
        bolocam_11_checkBox->setObjectName(QString::fromUtf8("bolocam_11_checkBox"));

        verticalLayout_9->addWidget(bolocam_11_checkBox);


        gridLayout_3->addWidget(groupBox_8, 0, 1, 1, 1);

        groupBox_9 = new QGroupBox(surveySelectorGroupBox);
        groupBox_9->setObjectName(QString::fromUtf8("groupBox_9"));
        verticalLayout_10 = new QVBoxLayout(groupBox_9);
        verticalLayout_10->setObjectName(QString::fromUtf8("verticalLayout_10"));
        cornish_6_checkBox = new QCheckBox(groupBox_9);
        cornish_6_checkBox->setObjectName(QString::fromUtf8("cornish_6_checkBox"));

        verticalLayout_10->addWidget(cornish_6_checkBox);


        gridLayout_3->addWidget(groupBox_9, 0, 2, 1, 1);


        verticalLayout_11->addLayout(gridLayout_3);


        verticalLayout->addWidget(surveySelectorGroupBox);

        selectionGroupBox = new QGroupBox(layoutWidget);
        selectionGroupBox->setObjectName(QString::fromUtf8("selectionGroupBox"));
        verticalLayout_3 = new QVBoxLayout(selectionGroupBox);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        noneRadioButton = new QRadioButton(selectionGroupBox);
        noneRadioButton->setObjectName(QString::fromUtf8("noneRadioButton"));
        noneRadioButton->setChecked(true);

        verticalLayout_3->addWidget(noneRadioButton);

        pointRadioButton = new QRadioButton(selectionGroupBox);
        pointRadioButton->setObjectName(QString::fromUtf8("pointRadioButton"));

        verticalLayout_3->addWidget(pointRadioButton);

        rectRadioButton = new QRadioButton(selectionGroupBox);
        rectRadioButton->setObjectName(QString::fromUtf8("rectRadioButton"));

        verticalLayout_3->addWidget(rectRadioButton);


        verticalLayout->addWidget(selectionGroupBox);

        coordinateGroupBox = new QGroupBox(layoutWidget);
        coordinateGroupBox->setObjectName(QString::fromUtf8("coordinateGroupBox"));
        verticalLayout_4 = new QVBoxLayout(coordinateGroupBox);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        label = new QLabel(coordinateGroupBox);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout_4->addWidget(label);

        glonLineEdit = new QLineEdit(coordinateGroupBox);
        glonLineEdit->setObjectName(QString::fromUtf8("glonLineEdit"));
        QSizePolicy sizePolicy2(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(glonLineEdit->sizePolicy().hasHeightForWidth());
        glonLineEdit->setSizePolicy(sizePolicy2);
        glonLineEdit->setMinimumSize(QSize(30, 0));

        verticalLayout_4->addWidget(glonLineEdit);

        label_3 = new QLabel(coordinateGroupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        verticalLayout_4->addWidget(label_3);

        glatLineEdit = new QLineEdit(coordinateGroupBox);
        glatLineEdit->setObjectName(QString::fromUtf8("glatLineEdit"));
        sizePolicy2.setHeightForWidth(glatLineEdit->sizePolicy().hasHeightForWidth());
        glatLineEdit->setSizePolicy(sizePolicy2);
        glatLineEdit->setMinimumSize(QSize(30, 0));

        verticalLayout_4->addWidget(glatLineEdit);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        dlLabel = new QLabel(coordinateGroupBox);
        dlLabel->setObjectName(QString::fromUtf8("dlLabel"));

        gridLayout->addWidget(dlLabel, 0, 1, 1, 1);

        label_2 = new QLabel(coordinateGroupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 0, 0, 1, 1);

        radiumLineEdit = new QLineEdit(coordinateGroupBox);
        radiumLineEdit->setObjectName(QString::fromUtf8("radiumLineEdit"));
        sizePolicy2.setHeightForWidth(radiumLineEdit->sizePolicy().hasHeightForWidth());
        radiumLineEdit->setSizePolicy(sizePolicy2);
        radiumLineEdit->setMinimumSize(QSize(10, 0));
        radiumLineEdit->setMaximumSize(QSize(300, 16777215));

        gridLayout->addWidget(radiumLineEdit, 1, 0, 1, 1);

        dbLabel = new QLabel(coordinateGroupBox);
        dbLabel->setObjectName(QString::fromUtf8("dbLabel"));

        gridLayout->addWidget(dbLabel, 0, 2, 1, 1);

        dlLineEdit = new QLineEdit(coordinateGroupBox);
        dlLineEdit->setObjectName(QString::fromUtf8("dlLineEdit"));
        sizePolicy2.setHeightForWidth(dlLineEdit->sizePolicy().hasHeightForWidth());
        dlLineEdit->setSizePolicy(sizePolicy2);
        dlLineEdit->setMinimumSize(QSize(10, 0));
        dlLineEdit->setMaximumSize(QSize(300, 16777215));

        gridLayout->addWidget(dlLineEdit, 1, 1, 1, 1);

        dbLineEdit = new QLineEdit(coordinateGroupBox);
        dbLineEdit->setObjectName(QString::fromUtf8("dbLineEdit"));
        sizePolicy2.setHeightForWidth(dbLineEdit->sizePolicy().hasHeightForWidth());
        dbLineEdit->setSizePolicy(sizePolicy2);
        dbLineEdit->setMinimumSize(QSize(10, 0));
        dbLineEdit->setMaximumSize(QSize(300, 16777215));

        gridLayout->addWidget(dbLineEdit, 1, 2, 1, 1);


        verticalLayout_4->addLayout(gridLayout);


        verticalLayout->addWidget(coordinateGroupBox);

        saveToDiskCheckBox = new QCheckBox(layoutWidget);
        saveToDiskCheckBox->setObjectName(QString::fromUtf8("saveToDiskCheckBox"));
        saveToDiskCheckBox->setEnabled(true);

        verticalLayout->addWidget(saveToDiskCheckBox);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        fileNameLineEdit = new QLineEdit(layoutWidget);
        fileNameLineEdit->setObjectName(QString::fromUtf8("fileNameLineEdit"));
        fileNameLineEdit->setEnabled(false);
        sizePolicy2.setHeightForWidth(fileNameLineEdit->sizePolicy().hasHeightForWidth());
        fileNameLineEdit->setSizePolicy(sizePolicy2);
        fileNameLineEdit->setMinimumSize(QSize(30, 0));

        horizontalLayout_2->addWidget(fileNameLineEdit);

        selectFsPushButton = new QPushButton(layoutWidget);
        selectFsPushButton->setObjectName(QString::fromUtf8("selectFsPushButton"));
        selectFsPushButton->setEnabled(false);
        selectFsPushButton->setIcon(icon);

        horizontalLayout_2->addWidget(selectFsPushButton);


        verticalLayout->addLayout(horizontalLayout_2);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);

        queryPushButton = new QPushButton(layoutWidget);
        queryPushButton->setObjectName(QString::fromUtf8("queryPushButton"));
        queryPushButton->setEnabled(false);

        verticalLayout->addWidget(queryPushButton);

        splitter->addWidget(layoutWidget);

        horizontalLayout->addWidget(splitter);

        ViaLactea->setCentralWidget(centralwidget);
        menubar = new QMenuBar(ViaLactea);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1266, 22));
        menubar->setNativeMenuBar(false);
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuAiuto = new QMenu(menubar);
        menuAiuto->setObjectName(QString::fromUtf8("menuAiuto"));
        ViaLactea->setMenuBar(menubar);
        statusbar = new QStatusBar(ViaLactea);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        ViaLactea->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuAiuto->menuAction());
        menuFile->addAction(actionSettings);
        menuFile->addAction(actionExit);
        menuFile->addAction(actionLoad_SED_2);
        menuAiuto->addAction(actionAbout);

        retranslateUi(ViaLactea);

        QMetaObject::connectSlotsByName(ViaLactea);
    } // setupUi

    void retranslateUi(QMainWindow *ViaLactea)
    {
        ViaLactea->setWindowTitle(QCoreApplication::translate("ViaLactea", "Vialactea - Visual Analytics client", nullptr));
        actionSettings->setText(QCoreApplication::translate("ViaLactea", "Settings", nullptr));
        actionExit->setText(QCoreApplication::translate("ViaLactea", "Exit", nullptr));
        actionAbout->setText(QCoreApplication::translate("ViaLactea", "About", nullptr));
        actionLoad_SED->setText(QCoreApplication::translate("ViaLactea", "Load SED", nullptr));
        actionLoad_SED_2->setText(QCoreApplication::translate("ViaLactea", "Load SED", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("ViaLactea", "Local image", nullptr));
        openLocalImagePushButton->setText(QString());
        groupBox_3->setTitle(QCoreApplication::translate("ViaLactea", "Local DC", nullptr));
        localDCPushButton->setText(QString());
        groupBox_4->setTitle(QCoreApplication::translate("ViaLactea", "3D", nullptr));
        select3dPushButton->setText(QCoreApplication::translate("ViaLactea", "Select", nullptr));
        surveySelectorGroupBox->setTitle(QCoreApplication::translate("ViaLactea", "Survey Selector", nullptr));
        groupBox_5->setTitle(QCoreApplication::translate("ViaLactea", "GLIMPSE/MIPSGAL", nullptr));
        glimpse_24_checkBox->setText(QCoreApplication::translate("ViaLactea", "24 \316\274m", nullptr));
        glimpse_8_checkBox->setText(QCoreApplication::translate("ViaLactea", "8.0 \316\274m", nullptr));
        glimpse_58_checkBox->setText(QCoreApplication::translate("ViaLactea", "5.8 \316\274m", nullptr));
        glimpse_45_checkBox->setText(QCoreApplication::translate("ViaLactea", "4.5 \316\274m", nullptr));
        glimpse_36checkBox->setText(QCoreApplication::translate("ViaLactea", "3.6 \316\274m", nullptr));
        groupBox->setTitle(QCoreApplication::translate("ViaLactea", "Hi-GAL", nullptr));
        PLW_checkBox_2->setText(QCoreApplication::translate("ViaLactea", "500 \316\274m", nullptr));
        PMW_checkBox->setText(QCoreApplication::translate("ViaLactea", "350 \316\274m ", nullptr));
        PSW_checkBox_12->setText(QCoreApplication::translate("ViaLactea", "250 \316\274m ", nullptr));
        blue_checkBox_2->setText(QCoreApplication::translate("ViaLactea", "70 \316\274m ", nullptr));
        red_checkBox->setText(QCoreApplication::translate("ViaLactea", "160 \316\274m ", nullptr));
        groupBox_6->setTitle(QCoreApplication::translate("ViaLactea", "WISE", nullptr));
        wise_22_checkBox->setText(QCoreApplication::translate("ViaLactea", "22 \316\274m", nullptr));
        wise_12_checkBox->setText(QCoreApplication::translate("ViaLactea", "12 \316\274m", nullptr));
        wise_46_checkBox->setText(QCoreApplication::translate("ViaLactea", "4.6 \316\274m", nullptr));
        wise_34_checkBox->setText(QCoreApplication::translate("ViaLactea", "3.4 \316\274m", nullptr));
        groupBox_7->setTitle(QCoreApplication::translate("ViaLactea", "ATLASGAL", nullptr));
        atlasgal_870_checkBox->setText(QCoreApplication::translate("ViaLactea", "870 \316\274m", nullptr));
        groupBox_8->setTitle(QCoreApplication::translate("ViaLactea", "BOLOCAM GPS", nullptr));
        bolocam_11_checkBox->setText(QCoreApplication::translate("ViaLactea", "1.1 mm", nullptr));
        groupBox_9->setTitle(QCoreApplication::translate("ViaLactea", "CORNISH", nullptr));
        cornish_6_checkBox->setText(QCoreApplication::translate("ViaLactea", "5 GHz", nullptr));
        selectionGroupBox->setTitle(QCoreApplication::translate("ViaLactea", "Selection", nullptr));
        noneRadioButton->setText(QCoreApplication::translate("ViaLactea", "None", nullptr));
        pointRadioButton->setText(QCoreApplication::translate("ViaLactea", "Point", nullptr));
        rectRadioButton->setText(QCoreApplication::translate("ViaLactea", "Rectangular", nullptr));
        coordinateGroupBox->setTitle(QCoreApplication::translate("ViaLactea", "Coordinates (center of selection)", nullptr));
        label->setText(QCoreApplication::translate("ViaLactea", "glon", nullptr));
        label_3->setText(QCoreApplication::translate("ViaLactea", "glat", nullptr));
        dlLabel->setText(QCoreApplication::translate("ViaLactea", "dl", nullptr));
        label_2->setText(QCoreApplication::translate("ViaLactea", "radius", nullptr));
        dbLabel->setText(QCoreApplication::translate("ViaLactea", "db", nullptr));
        saveToDiskCheckBox->setText(QCoreApplication::translate("ViaLactea", "Save to disk", nullptr));
        selectFsPushButton->setText(QString());
        queryPushButton->setText(QCoreApplication::translate("ViaLactea", "Query", nullptr));
        menuFile->setTitle(QCoreApplication::translate("ViaLactea", "File", nullptr));
        menuAiuto->setTitle(QCoreApplication::translate("ViaLactea", "Help", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ViaLactea: public Ui_ViaLactea {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VIALACTEA_H
