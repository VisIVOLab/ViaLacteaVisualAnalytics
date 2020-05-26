/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
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
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionNew;
    QAction *actionOpen;
    QAction *actionSave;
    QAction *actionSaveAs;
    QAction *actionPrint;
    QAction *actionPrintPreview;
    QAction *actionPage_Setup;
    QAction *actionRecent_Files;
    QAction *actionImportAscii;
    QAction *actionImportBinary;
    QAction *actionImportRawGrid;
    QAction *actionImportRawPoints;
    QAction *actionImportFlyOutput;
    QAction *actionImportHDF5;
    QAction *actionImportGadget;
    QAction *actionImportFits;
    QAction *actionImportVoTable;
    QAction *actionExportAscii;
    QAction *actionExportBinary;
    QAction *actionExportVoTable;
    QAction *actionUndo;
    QAction *actionDelete;
    QAction *action2DView;
    QAction *action3DView;
    QAction *actionOrthoSlices;
    QAction *actionPlotVIew;
    QAction *action2DFunctions;
    QAction *actionSendVOTable;
    QAction *actionShowObjects;
    QAction *actionPointDistribute;
    QAction *actionSubsampler;
    QAction *actionPicker;
    QAction *actionExtractCluster;
    QAction *actionSmoothSurface;
    QAction *actionRandomize;
    QAction *actionCreateVisualObject;
    QAction *actionCreateGridObject;
    QAction *actionCreateImageObject;
    QAction *actionAddVector;
    QAction *actionMathOp;
    QAction *actionCreatePlot;
    QAction *actionExtractIsoSurfaces;
    QAction *actionAboutVisIVO;
    QAction *actionVTK_ImageData;
    QAction *actionVTK_PolyData;
    QAction *actionBye;
    QAction *actionCiao;
    QAction *actionExit;
    QAction *actionClose;
    QAction *actionHi_Gal;
    QAction *actionOperation_queue;
    QAction *actionImportFitsImage;
    QAction *actionVialactea;
    QAction *actionDatacube;
    QAction *actionCsv;
    QAction *actionTEST_DC3D;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout_2;
    QFrame *frame;
    QHBoxLayout *horizontalLayout;
    QToolButton *buttonUndo;
    QToolButton *buttonResetCameraAll;
    QToolButton *buttonResetCameraObject;
    QToolButton *buttonCreateDO;
    QToolButton *buttonFilter;
    QToolButton *buttonCreateGrid;
    QToolButton *buttonDefineVector;
    QToolButton *buttonMathOp;
    QToolButton *buttonPlot;
    QToolButton *buttonMergeDO;
    QTabWidget *tabWidget;
    QWidget *tabObjectTree;
    QHBoxLayout *horizontalLayout_14;
    QSplitter *splitter;
    QTreeView *treeView;
    QTabWidget *dataEntity;
    QWidget *tabObjectInfo;
    QWidget *layoutWidget;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_13;
    QLabel *typeLabel;
    QLabel *valueTypeLabel;
    QHBoxLayout *horizontalLayout_10;
    QLabel *nameLabel_2;
    QLabel *valueNameLabel;
    QHBoxLayout *horizontalLayout_11;
    QLabel *elementLabel;
    QLabel *valueElementLabel;
    QHBoxLayout *horizontalLayout_12;
    QLabel *fieldLabel;
    QLabel *valueFieldLabel;
    QWidget *tabRendering;
    QWidget *tabObjectFields;
    QHBoxLayout *horizontalLayout_9;
    QTableWidget *fieldTable;
    QWidget *tabViewSettings;
    QVBoxLayout *verticalLayout_7;
    QScrollArea *viewSettingArea;
    QWidget *scrollAreaWidgetContents;
    QVBoxLayout *verticalLayout_6;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_2;
    QLabel *nameLabel;
    QLineEdit *namelineEdit;
    QHBoxLayout *horizontalLayout_3;
    QLabel *Xlabel;
    QComboBox *XcomboBox;
    QCheckBox *XLogCheckBox;
    QHBoxLayout *horizontalLayout_4;
    QLabel *Ylabel;
    QComboBox *YcomboBox;
    QCheckBox *YLogCheckBox;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label;
    QComboBox *ZcomboBox;
    QCheckBox *ZLogCheckBox;
    QCheckBox *ShowPointsCheckBox;
    QPushButton *ViewSettingspushButton;
    QVBoxLayout *verticalLayout_5;
    QCheckBox *ScaleCheckBox;
    QCheckBox *ShowAxesCheckBox;
    QCheckBox *ShowBoxCheckBox;
    QVBoxLayout *VectorLayout;
    QHBoxLayout *horizontalLayout_8;
    QLabel *XVectorsLabel;
    QComboBox *XVectorsComboBox;
    QHBoxLayout *horizontalLayout_7;
    QLabel *YVectorsLabel;
    QComboBox *YVectorsComboBox;
    QHBoxLayout *horizontalLayout_6;
    QLabel *ZVectorsLabel;
    QComboBox *ZVectorsComboBox;
    QCheckBox *ShowVectorsCheckBox;
    QWidget *tabOpParameters;
    QVBoxLayout *verticalLayout_999;
    QGroupBox *importerGroupBox;
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout_15;
    QLabel *label_2;
    QSpacerItem *horizontalSpacer_2;
    QLineEdit *missingValueLineEdit;
    QHBoxLayout *horizontalLayout_16;
    QLabel *label_3;
    QSpacerItem *horizontalSpacer;
    QLineEdit *textValueLineEdit;
    QHBoxLayout *horizontalLayout_17;
    QRadioButton *tableRadioButton;
    QRadioButton *volumeRadioButton;
    QGroupBox *volumeGroupBox;
    QVBoxLayout *verticalLayout_8;
    QLabel *label_4;
    QGridLayout *gridLayout;
    QLabel *label_6;
    QLineEdit *xComputationaCellLineEdit;
    QLabel *label_5;
    QLineEdit *yComputationaCellLineEdit;
    QLabel *label_7;
    QLineEdit *zComputationaCellLineEdit;
    QLabel *label_8;
    QGridLayout *gridLayout_2;
    QLabel *label_10;
    QLabel *label_11;
    QLabel *label_9;
    QLineEdit *xCellSizeLineEdit;
    QLineEdit *yCellSizeLineEdit;
    QLineEdit *zCellSizeLineEdit;
    QPushButton *importPushButton;
    QGroupBox *filterGroupBox;
    QVBoxLayout *verticalLayout_9;
    QComboBox *selectFilterComboBox;
    QTextBrowser *filterDescriptionTextBrowser;
    QGroupBox *addIdentifierParameterGroupBox;
    QGridLayout *gridLayout_3;
    QLineEdit *addIdNewColName;
    QLabel *label_12;
    QLabel *label_13;
    QLineEdit *addIdStartNumber;
    QGroupBox *appendParameterGroupBox;
    QVBoxLayout *verticalLayout_11;
    QHBoxLayout *horizontalLayout_19;
    QLabel *selectedVBT;
    QHBoxLayout *horizontalLayout_18;
    QListWidget *availableVBTlistWidget;
    QVBoxLayout *verticalLayout_10;
    QPushButton *addVBTpushButton;
    QPushButton *removeVBTpushButton;
    QListWidget *selectedVBTlistWidget;
    QPushButton *runFilterPushButton;
    QSpacerItem *verticalSpacer_2;
    QSpacerItem *verticalSpacer;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuImport;
    QMenu *menuExport;
    QMenu *menuEdit;
    QMenu *menuView;
    QMenu *menuAddView;
    QMenu *menuTools;
    QMenu *menuWindow;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->setEnabled(true);
        MainWindow->resize(755, 1000);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        actionNew = new QAction(MainWindow);
        actionNew->setObjectName(QString::fromUtf8("actionNew"));
        actionNew->setEnabled(false);
        actionOpen = new QAction(MainWindow);
        actionOpen->setObjectName(QString::fromUtf8("actionOpen"));
        actionOpen->setEnabled(false);
        actionSave = new QAction(MainWindow);
        actionSave->setObjectName(QString::fromUtf8("actionSave"));
        actionSave->setEnabled(false);
        actionSaveAs = new QAction(MainWindow);
        actionSaveAs->setObjectName(QString::fromUtf8("actionSaveAs"));
        actionSaveAs->setEnabled(false);
        actionPrint = new QAction(MainWindow);
        actionPrint->setObjectName(QString::fromUtf8("actionPrint"));
        actionPrint->setEnabled(false);
        actionPrintPreview = new QAction(MainWindow);
        actionPrintPreview->setObjectName(QString::fromUtf8("actionPrintPreview"));
        actionPrintPreview->setEnabled(false);
        actionPage_Setup = new QAction(MainWindow);
        actionPage_Setup->setObjectName(QString::fromUtf8("actionPage_Setup"));
        actionRecent_Files = new QAction(MainWindow);
        actionRecent_Files->setObjectName(QString::fromUtf8("actionRecent_Files"));
        actionImportAscii = new QAction(MainWindow);
        actionImportAscii->setObjectName(QString::fromUtf8("actionImportAscii"));
        actionImportBinary = new QAction(MainWindow);
        actionImportBinary->setObjectName(QString::fromUtf8("actionImportBinary"));
        actionImportRawGrid = new QAction(MainWindow);
        actionImportRawGrid->setObjectName(QString::fromUtf8("actionImportRawGrid"));
        actionImportRawGrid->setEnabled(false);
        actionImportRawPoints = new QAction(MainWindow);
        actionImportRawPoints->setObjectName(QString::fromUtf8("actionImportRawPoints"));
        actionImportRawPoints->setEnabled(false);
        actionImportFlyOutput = new QAction(MainWindow);
        actionImportFlyOutput->setObjectName(QString::fromUtf8("actionImportFlyOutput"));
        actionImportFlyOutput->setEnabled(false);
        actionImportHDF5 = new QAction(MainWindow);
        actionImportHDF5->setObjectName(QString::fromUtf8("actionImportHDF5"));
        actionImportHDF5->setEnabled(false);
        actionImportGadget = new QAction(MainWindow);
        actionImportGadget->setObjectName(QString::fromUtf8("actionImportGadget"));
        actionImportGadget->setEnabled(false);
        actionImportFits = new QAction(MainWindow);
        actionImportFits->setObjectName(QString::fromUtf8("actionImportFits"));
        actionImportFits->setEnabled(false);
        actionImportVoTable = new QAction(MainWindow);
        actionImportVoTable->setObjectName(QString::fromUtf8("actionImportVoTable"));
        actionExportAscii = new QAction(MainWindow);
        actionExportAscii->setObjectName(QString::fromUtf8("actionExportAscii"));
        actionExportBinary = new QAction(MainWindow);
        actionExportBinary->setObjectName(QString::fromUtf8("actionExportBinary"));
        actionExportVoTable = new QAction(MainWindow);
        actionExportVoTable->setObjectName(QString::fromUtf8("actionExportVoTable"));
        actionUndo = new QAction(MainWindow);
        actionUndo->setObjectName(QString::fromUtf8("actionUndo"));
        actionUndo->setEnabled(false);
        actionDelete = new QAction(MainWindow);
        actionDelete->setObjectName(QString::fromUtf8("actionDelete"));
        actionDelete->setEnabled(false);
        action2DView = new QAction(MainWindow);
        action2DView->setObjectName(QString::fromUtf8("action2DView"));
        action2DView->setEnabled(false);
        action3DView = new QAction(MainWindow);
        action3DView->setObjectName(QString::fromUtf8("action3DView"));
        actionOrthoSlices = new QAction(MainWindow);
        actionOrthoSlices->setObjectName(QString::fromUtf8("actionOrthoSlices"));
        actionPlotVIew = new QAction(MainWindow);
        actionPlotVIew->setObjectName(QString::fromUtf8("actionPlotVIew"));
        action2DFunctions = new QAction(MainWindow);
        action2DFunctions->setObjectName(QString::fromUtf8("action2DFunctions"));
        actionSendVOTable = new QAction(MainWindow);
        actionSendVOTable->setObjectName(QString::fromUtf8("actionSendVOTable"));
        actionShowObjects = new QAction(MainWindow);
        actionShowObjects->setObjectName(QString::fromUtf8("actionShowObjects"));
        actionPointDistribute = new QAction(MainWindow);
        actionPointDistribute->setObjectName(QString::fromUtf8("actionPointDistribute"));
        actionSubsampler = new QAction(MainWindow);
        actionSubsampler->setObjectName(QString::fromUtf8("actionSubsampler"));
        actionPicker = new QAction(MainWindow);
        actionPicker->setObjectName(QString::fromUtf8("actionPicker"));
        actionExtractCluster = new QAction(MainWindow);
        actionExtractCluster->setObjectName(QString::fromUtf8("actionExtractCluster"));
        actionSmoothSurface = new QAction(MainWindow);
        actionSmoothSurface->setObjectName(QString::fromUtf8("actionSmoothSurface"));
        actionRandomize = new QAction(MainWindow);
        actionRandomize->setObjectName(QString::fromUtf8("actionRandomize"));
        actionCreateVisualObject = new QAction(MainWindow);
        actionCreateVisualObject->setObjectName(QString::fromUtf8("actionCreateVisualObject"));
        actionCreateGridObject = new QAction(MainWindow);
        actionCreateGridObject->setObjectName(QString::fromUtf8("actionCreateGridObject"));
        actionCreateImageObject = new QAction(MainWindow);
        actionCreateImageObject->setObjectName(QString::fromUtf8("actionCreateImageObject"));
        actionAddVector = new QAction(MainWindow);
        actionAddVector->setObjectName(QString::fromUtf8("actionAddVector"));
        actionMathOp = new QAction(MainWindow);
        actionMathOp->setObjectName(QString::fromUtf8("actionMathOp"));
        actionCreatePlot = new QAction(MainWindow);
        actionCreatePlot->setObjectName(QString::fromUtf8("actionCreatePlot"));
        actionExtractIsoSurfaces = new QAction(MainWindow);
        actionExtractIsoSurfaces->setObjectName(QString::fromUtf8("actionExtractIsoSurfaces"));
        actionAboutVisIVO = new QAction(MainWindow);
        actionAboutVisIVO->setObjectName(QString::fromUtf8("actionAboutVisIVO"));
        actionVTK_ImageData = new QAction(MainWindow);
        actionVTK_ImageData->setObjectName(QString::fromUtf8("actionVTK_ImageData"));
        actionVTK_PolyData = new QAction(MainWindow);
        actionVTK_PolyData->setObjectName(QString::fromUtf8("actionVTK_PolyData"));
        actionBye = new QAction(MainWindow);
        actionBye->setObjectName(QString::fromUtf8("actionBye"));
        actionCiao = new QAction(MainWindow);
        actionCiao->setObjectName(QString::fromUtf8("actionCiao"));
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName(QString::fromUtf8("actionExit"));
        actionClose = new QAction(MainWindow);
        actionClose->setObjectName(QString::fromUtf8("actionClose"));
        actionHi_Gal = new QAction(MainWindow);
        actionHi_Gal->setObjectName(QString::fromUtf8("actionHi_Gal"));
        actionOperation_queue = new QAction(MainWindow);
        actionOperation_queue->setObjectName(QString::fromUtf8("actionOperation_queue"));
        actionImportFitsImage = new QAction(MainWindow);
        actionImportFitsImage->setObjectName(QString::fromUtf8("actionImportFitsImage"));
        actionImportFitsImage->setEnabled(false);
        actionVialactea = new QAction(MainWindow);
        actionVialactea->setObjectName(QString::fromUtf8("actionVialactea"));
        actionDatacube = new QAction(MainWindow);
        actionDatacube->setObjectName(QString::fromUtf8("actionDatacube"));
        actionDatacube->setEnabled(false);
        actionCsv = new QAction(MainWindow);
        actionCsv->setObjectName(QString::fromUtf8("actionCsv"));
        actionTEST_DC3D = new QAction(MainWindow);
        actionTEST_DC3D->setObjectName(QString::fromUtf8("actionTEST_DC3D"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        verticalLayout_2 = new QVBoxLayout(centralwidget);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        frame = new QFrame(centralwidget);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setMaximumSize(QSize(326, 16777215));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        horizontalLayout = new QHBoxLayout(frame);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        buttonUndo = new QToolButton(frame);
        buttonUndo->setObjectName(QString::fromUtf8("buttonUndo"));
        buttonUndo->setEnabled(false);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/OP_UNDO.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        buttonUndo->setIcon(icon);

        horizontalLayout->addWidget(buttonUndo);

        buttonResetCameraAll = new QToolButton(frame);
        buttonResetCameraAll->setObjectName(QString::fromUtf8("buttonResetCameraAll"));
        buttonResetCameraAll->setEnabled(false);
        QIcon icon1;
        icon1.addFile(QString::fromUtf8(":/icons/ZOOM_ALL.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        buttonResetCameraAll->setIcon(icon1);

        horizontalLayout->addWidget(buttonResetCameraAll);

        buttonResetCameraObject = new QToolButton(frame);
        buttonResetCameraObject->setObjectName(QString::fromUtf8("buttonResetCameraObject"));
        buttonResetCameraObject->setEnabled(false);
        QIcon icon2;
        icon2.addFile(QString::fromUtf8(":/icons/ZOOM.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        buttonResetCameraObject->setIcon(icon2);

        horizontalLayout->addWidget(buttonResetCameraObject);

        buttonCreateDO = new QToolButton(frame);
        buttonCreateDO->setObjectName(QString::fromUtf8("buttonCreateDO"));
        buttonCreateDO->setEnabled(false);
        QIcon icon3;
        icon3.addFile(QString::fromUtf8(":/icons/VBT_CLOUD.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        buttonCreateDO->setIcon(icon3);

        horizontalLayout->addWidget(buttonCreateDO);

        buttonFilter = new QToolButton(frame);
        buttonFilter->setObjectName(QString::fromUtf8("buttonFilter"));
        buttonFilter->setEnabled(false);
        QIcon icon4;
        icon4.addFile(QString::fromUtf8(":/icons/filter.png"), QSize(), QIcon::Normal, QIcon::Off);
        buttonFilter->setIcon(icon4);

        horizontalLayout->addWidget(buttonFilter);

        buttonCreateGrid = new QToolButton(frame);
        buttonCreateGrid->setObjectName(QString::fromUtf8("buttonCreateGrid"));
        buttonCreateGrid->setEnabled(false);
        QIcon icon5;
        icon5.addFile(QString::fromUtf8(":/icons/VBT_Volume.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        buttonCreateGrid->setIcon(icon5);

        horizontalLayout->addWidget(buttonCreateGrid);

        buttonDefineVector = new QToolButton(frame);
        buttonDefineVector->setObjectName(QString::fromUtf8("buttonDefineVector"));
        buttonDefineVector->setEnabled(false);
        QIcon icon6;
        icon6.addFile(QString::fromUtf8(":/icons/OPAddVector.png"), QSize(), QIcon::Normal, QIcon::Off);
        buttonDefineVector->setIcon(icon6);

        horizontalLayout->addWidget(buttonDefineVector);

        buttonMathOp = new QToolButton(frame);
        buttonMathOp->setObjectName(QString::fromUtf8("buttonMathOp"));
        buttonMathOp->setEnabled(false);
        QIcon icon7;
        icon7.addFile(QString::fromUtf8(":/icons/OP_Parser.png"), QSize(), QIcon::Normal, QIcon::Off);
        buttonMathOp->setIcon(icon7);

        horizontalLayout->addWidget(buttonMathOp);

        buttonPlot = new QToolButton(frame);
        buttonPlot->setObjectName(QString::fromUtf8("buttonPlot"));
        buttonPlot->setEnabled(false);
        QIcon icon8;
        icon8.addFile(QString::fromUtf8(":/icons/OP_MakePlot.png"), QSize(), QIcon::Normal, QIcon::Off);
        buttonPlot->setIcon(icon8);

        horizontalLayout->addWidget(buttonPlot);

        buttonMergeDO = new QToolButton(frame);
        buttonMergeDO->setObjectName(QString::fromUtf8("buttonMergeDO"));
        buttonMergeDO->setEnabled(false);
        QIcon icon9;
        icon9.addFile(QString::fromUtf8(":/icons/OP_Merger.png"), QSize(), QIcon::Normal, QIcon::Off);
        buttonMergeDO->setIcon(icon9);

        horizontalLayout->addWidget(buttonMergeDO);


        verticalLayout_2->addWidget(frame);

        tabWidget = new QTabWidget(centralwidget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabObjectTree = new QWidget();
        tabObjectTree->setObjectName(QString::fromUtf8("tabObjectTree"));
        tabObjectTree->setFocusPolicy(Qt::TabFocus);
        horizontalLayout_14 = new QHBoxLayout(tabObjectTree);
        horizontalLayout_14->setObjectName(QString::fromUtf8("horizontalLayout_14"));
        splitter = new QSplitter(tabObjectTree);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setOrientation(Qt::Vertical);
        treeView = new QTreeView(splitter);
        treeView->setObjectName(QString::fromUtf8("treeView"));
        splitter->addWidget(treeView);
        dataEntity = new QTabWidget(splitter);
        dataEntity->setObjectName(QString::fromUtf8("dataEntity"));
        tabObjectInfo = new QWidget();
        tabObjectInfo->setObjectName(QString::fromUtf8("tabObjectInfo"));
#if QT_CONFIG(tooltip)
        tabObjectInfo->setToolTip(QString::fromUtf8(""));
#endif // QT_CONFIG(tooltip)
        layoutWidget = new QWidget(tabObjectInfo);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(21, 20, 271, 131));
        verticalLayout_3 = new QVBoxLayout(layoutWidget);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_13 = new QHBoxLayout();
        horizontalLayout_13->setObjectName(QString::fromUtf8("horizontalLayout_13"));
        typeLabel = new QLabel(layoutWidget);
        typeLabel->setObjectName(QString::fromUtf8("typeLabel"));

        horizontalLayout_13->addWidget(typeLabel);

        valueTypeLabel = new QLabel(layoutWidget);
        valueTypeLabel->setObjectName(QString::fromUtf8("valueTypeLabel"));

        horizontalLayout_13->addWidget(valueTypeLabel);


        verticalLayout_3->addLayout(horizontalLayout_13);

        horizontalLayout_10 = new QHBoxLayout();
        horizontalLayout_10->setObjectName(QString::fromUtf8("horizontalLayout_10"));
        nameLabel_2 = new QLabel(layoutWidget);
        nameLabel_2->setObjectName(QString::fromUtf8("nameLabel_2"));

        horizontalLayout_10->addWidget(nameLabel_2);

        valueNameLabel = new QLabel(layoutWidget);
        valueNameLabel->setObjectName(QString::fromUtf8("valueNameLabel"));

        horizontalLayout_10->addWidget(valueNameLabel);


        verticalLayout_3->addLayout(horizontalLayout_10);

        horizontalLayout_11 = new QHBoxLayout();
        horizontalLayout_11->setObjectName(QString::fromUtf8("horizontalLayout_11"));
        elementLabel = new QLabel(layoutWidget);
        elementLabel->setObjectName(QString::fromUtf8("elementLabel"));

        horizontalLayout_11->addWidget(elementLabel);

        valueElementLabel = new QLabel(layoutWidget);
        valueElementLabel->setObjectName(QString::fromUtf8("valueElementLabel"));

        horizontalLayout_11->addWidget(valueElementLabel);


        verticalLayout_3->addLayout(horizontalLayout_11);

        horizontalLayout_12 = new QHBoxLayout();
        horizontalLayout_12->setObjectName(QString::fromUtf8("horizontalLayout_12"));
        fieldLabel = new QLabel(layoutWidget);
        fieldLabel->setObjectName(QString::fromUtf8("fieldLabel"));

        horizontalLayout_12->addWidget(fieldLabel);

        valueFieldLabel = new QLabel(layoutWidget);
        valueFieldLabel->setObjectName(QString::fromUtf8("valueFieldLabel"));

        horizontalLayout_12->addWidget(valueFieldLabel);


        verticalLayout_3->addLayout(horizontalLayout_12);

        dataEntity->addTab(tabObjectInfo, QString());
        tabRendering = new QWidget();
        tabRendering->setObjectName(QString::fromUtf8("tabRendering"));
        dataEntity->addTab(tabRendering, QString());
        tabObjectFields = new QWidget();
        tabObjectFields->setObjectName(QString::fromUtf8("tabObjectFields"));
        horizontalLayout_9 = new QHBoxLayout(tabObjectFields);
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        fieldTable = new QTableWidget(tabObjectFields);
        fieldTable->setObjectName(QString::fromUtf8("fieldTable"));

        horizontalLayout_9->addWidget(fieldTable);

        dataEntity->addTab(tabObjectFields, QString());
        splitter->addWidget(dataEntity);

        horizontalLayout_14->addWidget(splitter);

        tabWidget->addTab(tabObjectTree, QString());
        tabViewSettings = new QWidget();
        tabViewSettings->setObjectName(QString::fromUtf8("tabViewSettings"));
        tabViewSettings->setFocusPolicy(Qt::TabFocus);
        verticalLayout_7 = new QVBoxLayout(tabViewSettings);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        viewSettingArea = new QScrollArea(tabViewSettings);
        viewSettingArea->setObjectName(QString::fromUtf8("viewSettingArea"));
        viewSettingArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 683, 808));
        verticalLayout_6 = new QVBoxLayout(scrollAreaWidgetContents);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        nameLabel = new QLabel(scrollAreaWidgetContents);
        nameLabel->setObjectName(QString::fromUtf8("nameLabel"));

        horizontalLayout_2->addWidget(nameLabel);

        namelineEdit = new QLineEdit(scrollAreaWidgetContents);
        namelineEdit->setObjectName(QString::fromUtf8("namelineEdit"));
        namelineEdit->setReadOnly(true);

        horizontalLayout_2->addWidget(namelineEdit);


        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        Xlabel = new QLabel(scrollAreaWidgetContents);
        Xlabel->setObjectName(QString::fromUtf8("Xlabel"));

        horizontalLayout_3->addWidget(Xlabel);

        XcomboBox = new QComboBox(scrollAreaWidgetContents);
        XcomboBox->setObjectName(QString::fromUtf8("XcomboBox"));

        horizontalLayout_3->addWidget(XcomboBox);

        XLogCheckBox = new QCheckBox(scrollAreaWidgetContents);
        XLogCheckBox->setObjectName(QString::fromUtf8("XLogCheckBox"));
        XLogCheckBox->setChecked(false);

        horizontalLayout_3->addWidget(XLogCheckBox);


        verticalLayout->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        Ylabel = new QLabel(scrollAreaWidgetContents);
        Ylabel->setObjectName(QString::fromUtf8("Ylabel"));

        horizontalLayout_4->addWidget(Ylabel);

        YcomboBox = new QComboBox(scrollAreaWidgetContents);
        YcomboBox->setObjectName(QString::fromUtf8("YcomboBox"));

        horizontalLayout_4->addWidget(YcomboBox);

        YLogCheckBox = new QCheckBox(scrollAreaWidgetContents);
        YLogCheckBox->setObjectName(QString::fromUtf8("YLogCheckBox"));
        YLogCheckBox->setChecked(false);

        horizontalLayout_4->addWidget(YLogCheckBox);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label = new QLabel(scrollAreaWidgetContents);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_5->addWidget(label);

        ZcomboBox = new QComboBox(scrollAreaWidgetContents);
        ZcomboBox->setObjectName(QString::fromUtf8("ZcomboBox"));

        horizontalLayout_5->addWidget(ZcomboBox);

        ZLogCheckBox = new QCheckBox(scrollAreaWidgetContents);
        ZLogCheckBox->setObjectName(QString::fromUtf8("ZLogCheckBox"));
        ZLogCheckBox->setCheckable(true);
        ZLogCheckBox->setChecked(false);

        horizontalLayout_5->addWidget(ZLogCheckBox);


        verticalLayout->addLayout(horizontalLayout_5);

        ShowPointsCheckBox = new QCheckBox(scrollAreaWidgetContents);
        ShowPointsCheckBox->setObjectName(QString::fromUtf8("ShowPointsCheckBox"));
        ShowPointsCheckBox->setChecked(false);

        verticalLayout->addWidget(ShowPointsCheckBox);


        verticalLayout_6->addLayout(verticalLayout);

        ViewSettingspushButton = new QPushButton(scrollAreaWidgetContents);
        ViewSettingspushButton->setObjectName(QString::fromUtf8("ViewSettingspushButton"));

        verticalLayout_6->addWidget(ViewSettingspushButton);

        verticalLayout_5 = new QVBoxLayout();
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        ScaleCheckBox = new QCheckBox(scrollAreaWidgetContents);
        ScaleCheckBox->setObjectName(QString::fromUtf8("ScaleCheckBox"));
        ScaleCheckBox->setChecked(false);

        verticalLayout_5->addWidget(ScaleCheckBox);

        ShowAxesCheckBox = new QCheckBox(scrollAreaWidgetContents);
        ShowAxesCheckBox->setObjectName(QString::fromUtf8("ShowAxesCheckBox"));
        ShowAxesCheckBox->setChecked(false);

        verticalLayout_5->addWidget(ShowAxesCheckBox);

        ShowBoxCheckBox = new QCheckBox(scrollAreaWidgetContents);
        ShowBoxCheckBox->setObjectName(QString::fromUtf8("ShowBoxCheckBox"));
        ShowBoxCheckBox->setChecked(false);

        verticalLayout_5->addWidget(ShowBoxCheckBox);


        verticalLayout_6->addLayout(verticalLayout_5);

        VectorLayout = new QVBoxLayout();
        VectorLayout->setObjectName(QString::fromUtf8("VectorLayout"));
        horizontalLayout_8 = new QHBoxLayout();
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        XVectorsLabel = new QLabel(scrollAreaWidgetContents);
        XVectorsLabel->setObjectName(QString::fromUtf8("XVectorsLabel"));

        horizontalLayout_8->addWidget(XVectorsLabel);

        XVectorsComboBox = new QComboBox(scrollAreaWidgetContents);
        XVectorsComboBox->setObjectName(QString::fromUtf8("XVectorsComboBox"));

        horizontalLayout_8->addWidget(XVectorsComboBox);


        VectorLayout->addLayout(horizontalLayout_8);

        horizontalLayout_7 = new QHBoxLayout();
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        YVectorsLabel = new QLabel(scrollAreaWidgetContents);
        YVectorsLabel->setObjectName(QString::fromUtf8("YVectorsLabel"));

        horizontalLayout_7->addWidget(YVectorsLabel);

        YVectorsComboBox = new QComboBox(scrollAreaWidgetContents);
        YVectorsComboBox->setObjectName(QString::fromUtf8("YVectorsComboBox"));

        horizontalLayout_7->addWidget(YVectorsComboBox);


        VectorLayout->addLayout(horizontalLayout_7);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        ZVectorsLabel = new QLabel(scrollAreaWidgetContents);
        ZVectorsLabel->setObjectName(QString::fromUtf8("ZVectorsLabel"));

        horizontalLayout_6->addWidget(ZVectorsLabel);

        ZVectorsComboBox = new QComboBox(scrollAreaWidgetContents);
        ZVectorsComboBox->setObjectName(QString::fromUtf8("ZVectorsComboBox"));

        horizontalLayout_6->addWidget(ZVectorsComboBox);


        VectorLayout->addLayout(horizontalLayout_6);

        ShowVectorsCheckBox = new QCheckBox(scrollAreaWidgetContents);
        ShowVectorsCheckBox->setObjectName(QString::fromUtf8("ShowVectorsCheckBox"));
        ShowVectorsCheckBox->setChecked(false);

        VectorLayout->addWidget(ShowVectorsCheckBox);


        verticalLayout_6->addLayout(VectorLayout);

        viewSettingArea->setWidget(scrollAreaWidgetContents);

        verticalLayout_7->addWidget(viewSettingArea);

        tabWidget->addTab(tabViewSettings, QString());
        tabOpParameters = new QWidget();
        tabOpParameters->setObjectName(QString::fromUtf8("tabOpParameters"));
        tabOpParameters->setEnabled(true);
        tabOpParameters->setMinimumSize(QSize(412, 0));
        tabOpParameters->setFocusPolicy(Qt::TabFocus);
        verticalLayout_999 = new QVBoxLayout(tabOpParameters);
        verticalLayout_999->setObjectName(QString::fromUtf8("verticalLayout_999"));
        importerGroupBox = new QGroupBox(tabOpParameters);
        importerGroupBox->setObjectName(QString::fromUtf8("importerGroupBox"));
        importerGroupBox->setEnabled(true);
        verticalLayout_4 = new QVBoxLayout(importerGroupBox);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        horizontalLayout_15 = new QHBoxLayout();
        horizontalLayout_15->setObjectName(QString::fromUtf8("horizontalLayout_15"));
        label_2 = new QLabel(importerGroupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_15->addWidget(label_2);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_15->addItem(horizontalSpacer_2);

        missingValueLineEdit = new QLineEdit(importerGroupBox);
        missingValueLineEdit->setObjectName(QString::fromUtf8("missingValueLineEdit"));

        horizontalLayout_15->addWidget(missingValueLineEdit);


        verticalLayout_4->addLayout(horizontalLayout_15);

        horizontalLayout_16 = new QHBoxLayout();
        horizontalLayout_16->setObjectName(QString::fromUtf8("horizontalLayout_16"));
        label_3 = new QLabel(importerGroupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_16->addWidget(label_3);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_16->addItem(horizontalSpacer);

        textValueLineEdit = new QLineEdit(importerGroupBox);
        textValueLineEdit->setObjectName(QString::fromUtf8("textValueLineEdit"));

        horizontalLayout_16->addWidget(textValueLineEdit);


        verticalLayout_4->addLayout(horizontalLayout_16);

        horizontalLayout_17 = new QHBoxLayout();
        horizontalLayout_17->setObjectName(QString::fromUtf8("horizontalLayout_17"));
        tableRadioButton = new QRadioButton(importerGroupBox);
        tableRadioButton->setObjectName(QString::fromUtf8("tableRadioButton"));
        tableRadioButton->setChecked(true);

        horizontalLayout_17->addWidget(tableRadioButton);

        volumeRadioButton = new QRadioButton(importerGroupBox);
        volumeRadioButton->setObjectName(QString::fromUtf8("volumeRadioButton"));

        horizontalLayout_17->addWidget(volumeRadioButton);


        verticalLayout_4->addLayout(horizontalLayout_17);

        volumeGroupBox = new QGroupBox(importerGroupBox);
        volumeGroupBox->setObjectName(QString::fromUtf8("volumeGroupBox"));
        verticalLayout_8 = new QVBoxLayout(volumeGroupBox);
        verticalLayout_8->setObjectName(QString::fromUtf8("verticalLayout_8"));
        label_4 = new QLabel(volumeGroupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        verticalLayout_8->addWidget(label_4);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_6 = new QLabel(volumeGroupBox);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout->addWidget(label_6, 0, 1, 1, 1);

        xComputationaCellLineEdit = new QLineEdit(volumeGroupBox);
        xComputationaCellLineEdit->setObjectName(QString::fromUtf8("xComputationaCellLineEdit"));

        gridLayout->addWidget(xComputationaCellLineEdit, 1, 0, 1, 1);

        label_5 = new QLabel(volumeGroupBox);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 0, 0, 1, 1);

        yComputationaCellLineEdit = new QLineEdit(volumeGroupBox);
        yComputationaCellLineEdit->setObjectName(QString::fromUtf8("yComputationaCellLineEdit"));

        gridLayout->addWidget(yComputationaCellLineEdit, 1, 1, 1, 1);

        label_7 = new QLabel(volumeGroupBox);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        gridLayout->addWidget(label_7, 0, 2, 1, 1);

        zComputationaCellLineEdit = new QLineEdit(volumeGroupBox);
        zComputationaCellLineEdit->setObjectName(QString::fromUtf8("zComputationaCellLineEdit"));

        gridLayout->addWidget(zComputationaCellLineEdit, 1, 2, 1, 1);


        verticalLayout_8->addLayout(gridLayout);

        label_8 = new QLabel(volumeGroupBox);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        verticalLayout_8->addWidget(label_8);

        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        label_10 = new QLabel(volumeGroupBox);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        gridLayout_2->addWidget(label_10, 0, 1, 1, 1);

        label_11 = new QLabel(volumeGroupBox);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        gridLayout_2->addWidget(label_11, 0, 2, 1, 1);

        label_9 = new QLabel(volumeGroupBox);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        gridLayout_2->addWidget(label_9, 0, 0, 1, 1);

        xCellSizeLineEdit = new QLineEdit(volumeGroupBox);
        xCellSizeLineEdit->setObjectName(QString::fromUtf8("xCellSizeLineEdit"));

        gridLayout_2->addWidget(xCellSizeLineEdit, 1, 0, 1, 1);

        yCellSizeLineEdit = new QLineEdit(volumeGroupBox);
        yCellSizeLineEdit->setObjectName(QString::fromUtf8("yCellSizeLineEdit"));

        gridLayout_2->addWidget(yCellSizeLineEdit, 1, 1, 1, 1);

        zCellSizeLineEdit = new QLineEdit(volumeGroupBox);
        zCellSizeLineEdit->setObjectName(QString::fromUtf8("zCellSizeLineEdit"));

        gridLayout_2->addWidget(zCellSizeLineEdit, 1, 2, 1, 1);


        verticalLayout_8->addLayout(gridLayout_2);


        verticalLayout_4->addWidget(volumeGroupBox);

        importPushButton = new QPushButton(importerGroupBox);
        importPushButton->setObjectName(QString::fromUtf8("importPushButton"));

        verticalLayout_4->addWidget(importPushButton);


        verticalLayout_999->addWidget(importerGroupBox);

        filterGroupBox = new QGroupBox(tabOpParameters);
        filterGroupBox->setObjectName(QString::fromUtf8("filterGroupBox"));
        filterGroupBox->setEnabled(true);
        verticalLayout_9 = new QVBoxLayout(filterGroupBox);
        verticalLayout_9->setObjectName(QString::fromUtf8("verticalLayout_9"));
        selectFilterComboBox = new QComboBox(filterGroupBox);
        selectFilterComboBox->addItem(QString());
        selectFilterComboBox->addItem(QString());
        selectFilterComboBox->addItem(QString());
        selectFilterComboBox->addItem(QString());
        selectFilterComboBox->addItem(QString());
        selectFilterComboBox->addItem(QString());
        selectFilterComboBox->addItem(QString());
        selectFilterComboBox->addItem(QString());
        selectFilterComboBox->addItem(QString());
        selectFilterComboBox->addItem(QString());
        selectFilterComboBox->addItem(QString());
        selectFilterComboBox->addItem(QString());
        selectFilterComboBox->addItem(QString());
        selectFilterComboBox->addItem(QString());
        selectFilterComboBox->addItem(QString());
        selectFilterComboBox->addItem(QString());
        selectFilterComboBox->addItem(QString());
        selectFilterComboBox->addItem(QString());
        selectFilterComboBox->addItem(QString());
        selectFilterComboBox->addItem(QString());
        selectFilterComboBox->addItem(QString());
        selectFilterComboBox->addItem(QString());
        selectFilterComboBox->setObjectName(QString::fromUtf8("selectFilterComboBox"));

        verticalLayout_9->addWidget(selectFilterComboBox);

        filterDescriptionTextBrowser = new QTextBrowser(filterGroupBox);
        filterDescriptionTextBrowser->setObjectName(QString::fromUtf8("filterDescriptionTextBrowser"));
        filterDescriptionTextBrowser->setEnabled(false);

        verticalLayout_9->addWidget(filterDescriptionTextBrowser);

        addIdentifierParameterGroupBox = new QGroupBox(filterGroupBox);
        addIdentifierParameterGroupBox->setObjectName(QString::fromUtf8("addIdentifierParameterGroupBox"));
        gridLayout_3 = new QGridLayout(addIdentifierParameterGroupBox);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        addIdNewColName = new QLineEdit(addIdentifierParameterGroupBox);
        addIdNewColName->setObjectName(QString::fromUtf8("addIdNewColName"));

        gridLayout_3->addWidget(addIdNewColName, 0, 1, 1, 1);

        label_12 = new QLabel(addIdentifierParameterGroupBox);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        gridLayout_3->addWidget(label_12, 1, 0, 1, 1);

        label_13 = new QLabel(addIdentifierParameterGroupBox);
        label_13->setObjectName(QString::fromUtf8("label_13"));

        gridLayout_3->addWidget(label_13, 0, 0, 1, 1);

        addIdStartNumber = new QLineEdit(addIdentifierParameterGroupBox);
        addIdStartNumber->setObjectName(QString::fromUtf8("addIdStartNumber"));

        gridLayout_3->addWidget(addIdStartNumber, 1, 1, 1, 1);


        verticalLayout_9->addWidget(addIdentifierParameterGroupBox);

        appendParameterGroupBox = new QGroupBox(filterGroupBox);
        appendParameterGroupBox->setObjectName(QString::fromUtf8("appendParameterGroupBox"));
        verticalLayout_11 = new QVBoxLayout(appendParameterGroupBox);
        verticalLayout_11->setObjectName(QString::fromUtf8("verticalLayout_11"));
        horizontalLayout_19 = new QHBoxLayout();
        horizontalLayout_19->setObjectName(QString::fromUtf8("horizontalLayout_19"));
        selectedVBT = new QLabel(appendParameterGroupBox);
        selectedVBT->setObjectName(QString::fromUtf8("selectedVBT"));

        horizontalLayout_19->addWidget(selectedVBT);


        verticalLayout_11->addLayout(horizontalLayout_19);

        horizontalLayout_18 = new QHBoxLayout();
        horizontalLayout_18->setObjectName(QString::fromUtf8("horizontalLayout_18"));
        availableVBTlistWidget = new QListWidget(appendParameterGroupBox);
        availableVBTlistWidget->setObjectName(QString::fromUtf8("availableVBTlistWidget"));

        horizontalLayout_18->addWidget(availableVBTlistWidget);

        verticalLayout_10 = new QVBoxLayout();
        verticalLayout_10->setObjectName(QString::fromUtf8("verticalLayout_10"));
        addVBTpushButton = new QPushButton(appendParameterGroupBox);
        addVBTpushButton->setObjectName(QString::fromUtf8("addVBTpushButton"));

        verticalLayout_10->addWidget(addVBTpushButton);

        removeVBTpushButton = new QPushButton(appendParameterGroupBox);
        removeVBTpushButton->setObjectName(QString::fromUtf8("removeVBTpushButton"));

        verticalLayout_10->addWidget(removeVBTpushButton);


        horizontalLayout_18->addLayout(verticalLayout_10);

        selectedVBTlistWidget = new QListWidget(appendParameterGroupBox);
        selectedVBTlistWidget->setObjectName(QString::fromUtf8("selectedVBTlistWidget"));

        horizontalLayout_18->addWidget(selectedVBTlistWidget);


        verticalLayout_11->addLayout(horizontalLayout_18);


        verticalLayout_9->addWidget(appendParameterGroupBox);

        runFilterPushButton = new QPushButton(filterGroupBox);
        runFilterPushButton->setObjectName(QString::fromUtf8("runFilterPushButton"));

        verticalLayout_9->addWidget(runFilterPushButton);

        verticalSpacer_2 = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_9->addItem(verticalSpacer_2);


        verticalLayout_999->addWidget(filterGroupBox);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_999->addItem(verticalSpacer);

        tabWidget->addTab(tabOpParameters, QString());

        verticalLayout_2->addWidget(tabWidget);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 755, 22));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuImport = new QMenu(menuFile);
        menuImport->setObjectName(QString::fromUtf8("menuImport"));
        menuExport = new QMenu(menuFile);
        menuExport->setObjectName(QString::fromUtf8("menuExport"));
        menuExport->setEnabled(false);
        menuEdit = new QMenu(menubar);
        menuEdit->setObjectName(QString::fromUtf8("menuEdit"));
        menuEdit->setEnabled(true);
        menuView = new QMenu(menubar);
        menuView->setObjectName(QString::fromUtf8("menuView"));
        menuView->setEnabled(false);
        menuAddView = new QMenu(menuView);
        menuAddView->setObjectName(QString::fromUtf8("menuAddView"));
        menuAddView->setEnabled(false);
        menuTools = new QMenu(menubar);
        menuTools->setObjectName(QString::fromUtf8("menuTools"));
        menuTools->setEnabled(true);
        menuWindow = new QMenu(menubar);
        menuWindow->setObjectName(QString::fromUtf8("menuWindow"));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuEdit->menuAction());
        menubar->addAction(menuView->menuAction());
        menubar->addAction(menuTools->menuAction());
        menubar->addAction(menuWindow->menuAction());
        menuFile->addAction(actionNew);
        menuFile->addSeparator();
        menuFile->addAction(actionOpen);
        menuFile->addAction(actionSave);
        menuFile->addAction(actionSaveAs);
        menuFile->addSeparator();
        menuFile->addAction(menuImport->menuAction());
        menuFile->addAction(menuExport->menuAction());
        menuFile->addSeparator();
        menuFile->addAction(actionPrint);
        menuFile->addAction(actionPrintPreview);
        menuFile->addAction(actionClose);
        menuImport->addAction(actionImportAscii);
        menuImport->addAction(actionCsv);
        menuImport->addAction(actionImportBinary);
        menuImport->addAction(actionImportVoTable);
        menuImport->addAction(actionImportRawGrid);
        menuImport->addAction(actionImportRawPoints);
        menuImport->addAction(actionImportFlyOutput);
        menuImport->addAction(actionImportHDF5);
        menuImport->addAction(actionImportGadget);
        menuImport->addAction(actionImportFits);
        menuImport->addAction(actionImportFitsImage);
        menuImport->addAction(actionDatacube);
        menuImport->addSeparator();
        menuImport->addAction(actionVTK_ImageData);
        menuImport->addAction(actionVTK_PolyData);
        menuImport->addAction(actionTEST_DC3D);
        menuExport->addAction(actionExportAscii);
        menuExport->addAction(actionExportBinary);
        menuExport->addAction(actionExportVoTable);
        menuEdit->addAction(actionUndo);
        menuEdit->addAction(actionDelete);
        menuView->addAction(menuAddView->menuAction());
        menuAddView->addAction(action2DView);
        menuAddView->addAction(action3DView);
        menuAddView->addAction(actionOrthoSlices);
        menuAddView->addAction(actionPlotVIew);
        menuTools->addAction(actionVialactea);
        menuWindow->addAction(actionOperation_queue);

        retranslateUi(MainWindow);
        QObject::connect(buttonCreateDO, SIGNAL(clicked(bool)), MainWindow, SLOT(viewSetting()));
        QObject::connect(treeView, SIGNAL(doubleClicked(QModelIndex)), treeView, SLOT(expand(QModelIndex)));
        QObject::connect(ViewSettingspushButton, SIGNAL(clicked()), MainWindow, SLOT(viewSettingOk()));
        QObject::connect(tabWidget, SIGNAL(currentChanged(int)), MainWindow, SLOT(switchTab(int)));
        QObject::connect(treeView, SIGNAL(clicked(QModelIndex)), MainWindow, SLOT(itemSelected()));

        tabWidget->setCurrentIndex(2);
        dataEntity->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "VisIVO Desktop", nullptr));
        actionNew->setText(QCoreApplication::translate("MainWindow", "New", nullptr));
        actionOpen->setText(QCoreApplication::translate("MainWindow", "Open", nullptr));
        actionSave->setText(QCoreApplication::translate("MainWindow", "Save", nullptr));
        actionSaveAs->setText(QCoreApplication::translate("MainWindow", "Save as", nullptr));
        actionPrint->setText(QCoreApplication::translate("MainWindow", "Print", nullptr));
        actionPrintPreview->setText(QCoreApplication::translate("MainWindow", "Print Preview", nullptr));
        actionPage_Setup->setText(QCoreApplication::translate("MainWindow", "Page Setup", nullptr));
        actionRecent_Files->setText(QCoreApplication::translate("MainWindow", "Recent Files", nullptr));
        actionImportAscii->setText(QCoreApplication::translate("MainWindow", "Ascii", nullptr));
        actionImportBinary->setText(QCoreApplication::translate("MainWindow", "Binary", nullptr));
        actionImportRawGrid->setText(QCoreApplication::translate("MainWindow", "Raw Grid", nullptr));
        actionImportRawPoints->setText(QCoreApplication::translate("MainWindow", "Raw Points", nullptr));
        actionImportFlyOutput->setText(QCoreApplication::translate("MainWindow", "Fly Output", nullptr));
        actionImportHDF5->setText(QCoreApplication::translate("MainWindow", "HDF5", nullptr));
        actionImportGadget->setText(QCoreApplication::translate("MainWindow", "Gadget", nullptr));
        actionImportFits->setText(QCoreApplication::translate("MainWindow", "Fits", nullptr));
        actionImportVoTable->setText(QCoreApplication::translate("MainWindow", "VoTable", nullptr));
        actionExportAscii->setText(QCoreApplication::translate("MainWindow", "Ascii", nullptr));
        actionExportBinary->setText(QCoreApplication::translate("MainWindow", "Binary", nullptr));
        actionExportVoTable->setText(QCoreApplication::translate("MainWindow", "VoTable", nullptr));
        actionUndo->setText(QCoreApplication::translate("MainWindow", "Undo", nullptr));
        actionDelete->setText(QCoreApplication::translate("MainWindow", "Delete", nullptr));
        action2DView->setText(QCoreApplication::translate("MainWindow", "2D View", nullptr));
        action3DView->setText(QCoreApplication::translate("MainWindow", "3D View", nullptr));
        actionOrthoSlices->setText(QCoreApplication::translate("MainWindow", "Ortho Slices", nullptr));
        actionPlotVIew->setText(QCoreApplication::translate("MainWindow", "Plot VIew", nullptr));
        action2DFunctions->setText(QCoreApplication::translate("MainWindow", "2D Functions", nullptr));
        actionSendVOTable->setText(QCoreApplication::translate("MainWindow", "Send VOTable", nullptr));
        actionShowObjects->setText(QCoreApplication::translate("MainWindow", "Show Objects", nullptr));
        actionPointDistribute->setText(QCoreApplication::translate("MainWindow", "Point Distribute", nullptr));
        actionSubsampler->setText(QCoreApplication::translate("MainWindow", "Subsampler", nullptr));
        actionPicker->setText(QCoreApplication::translate("MainWindow", "Picker", nullptr));
        actionExtractCluster->setText(QCoreApplication::translate("MainWindow", "Extract Cluster", nullptr));
        actionSmoothSurface->setText(QCoreApplication::translate("MainWindow", "Smooth Surface", nullptr));
        actionRandomize->setText(QCoreApplication::translate("MainWindow", "Randomize", nullptr));
        actionCreateVisualObject->setText(QCoreApplication::translate("MainWindow", "Create Visual Object", nullptr));
        actionCreateGridObject->setText(QCoreApplication::translate("MainWindow", "Create Grid Object", nullptr));
        actionCreateImageObject->setText(QCoreApplication::translate("MainWindow", "Create Image Object", nullptr));
        actionAddVector->setText(QCoreApplication::translate("MainWindow", "Add Vector", nullptr));
        actionMathOp->setText(QCoreApplication::translate("MainWindow", "Math Op.", nullptr));
        actionCreatePlot->setText(QCoreApplication::translate("MainWindow", "Create Plot", nullptr));
        actionExtractIsoSurfaces->setText(QCoreApplication::translate("MainWindow", "Extract IsoSurfaces", nullptr));
        actionAboutVisIVO->setText(QCoreApplication::translate("MainWindow", "about VisIVO", nullptr));
        actionVTK_ImageData->setText(QCoreApplication::translate("MainWindow", "VTK ImageData ", nullptr));
        actionVTK_PolyData->setText(QCoreApplication::translate("MainWindow", "VTK PolyData", nullptr));
        actionBye->setText(QCoreApplication::translate("MainWindow", "Quit", nullptr));
        actionCiao->setText(QCoreApplication::translate("MainWindow", "Quit", nullptr));
        actionExit->setText(QCoreApplication::translate("MainWindow", "Close", nullptr));
        actionClose->setText(QCoreApplication::translate("MainWindow", "Close", nullptr));
        actionHi_Gal->setText(QCoreApplication::translate("MainWindow", "Hi-Gal", nullptr));
        actionOperation_queue->setText(QCoreApplication::translate("MainWindow", "Operation queue", nullptr));
        actionImportFitsImage->setText(QCoreApplication::translate("MainWindow", "Fits Image", nullptr));
        actionVialactea->setText(QCoreApplication::translate("MainWindow", "Vialactea", nullptr));
        actionDatacube->setText(QCoreApplication::translate("MainWindow", "Datacube", nullptr));
        actionCsv->setText(QCoreApplication::translate("MainWindow", "Csv", nullptr));
        actionTEST_DC3D->setText(QCoreApplication::translate("MainWindow", "TEST DC3D", nullptr));
#if QT_CONFIG(shortcut)
        actionTEST_DC3D->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+D", nullptr));
#endif // QT_CONFIG(shortcut)
#if QT_CONFIG(tooltip)
        buttonUndo->setToolTip(QCoreApplication::translate("MainWindow", "Undo", nullptr));
#endif // QT_CONFIG(tooltip)
        buttonUndo->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
#if QT_CONFIG(tooltip)
        buttonResetCameraAll->setToolTip(QCoreApplication::translate("MainWindow", "Reset camera to fit all", nullptr));
#endif // QT_CONFIG(tooltip)
        buttonResetCameraAll->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
#if QT_CONFIG(tooltip)
        buttonResetCameraObject->setToolTip(QCoreApplication::translate("MainWindow", "Reset camera to fit selected object", nullptr));
#endif // QT_CONFIG(tooltip)
        buttonResetCameraObject->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
#if QT_CONFIG(tooltip)
        buttonCreateDO->setToolTip(QCoreApplication::translate("MainWindow", "Create a visual object from the selected Data Object", nullptr));
#endif // QT_CONFIG(tooltip)
        buttonCreateDO->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
        buttonFilter->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
#if QT_CONFIG(tooltip)
        buttonCreateGrid->setToolTip(QCoreApplication::translate("MainWindow", "Create a grid object from the selected Data Object", nullptr));
#endif // QT_CONFIG(tooltip)
        buttonCreateGrid->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
#if QT_CONFIG(tooltip)
        buttonDefineVector->setToolTip(QCoreApplication::translate("MainWindow", "Define a vector in the Data Object", nullptr));
#endif // QT_CONFIG(tooltip)
        buttonDefineVector->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
#if QT_CONFIG(tooltip)
        buttonMathOp->setToolTip(QCoreApplication::translate("MainWindow", "Perform a mathematical operation on a Data Object", nullptr));
#endif // QT_CONFIG(tooltip)
        buttonMathOp->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
#if QT_CONFIG(tooltip)
        buttonPlot->setToolTip(QCoreApplication::translate("MainWindow", "Create a plot from the Data Object", nullptr));
#endif // QT_CONFIG(tooltip)
        buttonPlot->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
#if QT_CONFIG(tooltip)
        buttonMergeDO->setToolTip(QCoreApplication::translate("MainWindow", "Merge two Data Objects to a new one", nullptr));
#endif // QT_CONFIG(tooltip)
        buttonMergeDO->setText(QCoreApplication::translate("MainWindow", "...", nullptr));
#if QT_CONFIG(tooltip)
        dataEntity->setToolTip(QString());
#endif // QT_CONFIG(tooltip)
        typeLabel->setText(QString());
        valueTypeLabel->setText(QString());
        nameLabel_2->setText(QString());
        valueNameLabel->setText(QString());
        elementLabel->setText(QString());
        valueElementLabel->setText(QString());
        fieldLabel->setText(QString());
        valueFieldLabel->setText(QString());
        dataEntity->setTabText(dataEntity->indexOf(tabObjectInfo), QCoreApplication::translate("MainWindow", "Object Info", nullptr));
        dataEntity->setTabText(dataEntity->indexOf(tabRendering), QCoreApplication::translate("MainWindow", "Rendering", nullptr));
        dataEntity->setTabText(dataEntity->indexOf(tabObjectFields), QCoreApplication::translate("MainWindow", "Object Fields", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabObjectTree), QCoreApplication::translate("MainWindow", "Object Tree", nullptr));
#if QT_CONFIG(accessibility)
        tabViewSettings->setAccessibleDescription(QString());
#endif // QT_CONFIG(accessibility)
        nameLabel->setText(QCoreApplication::translate("MainWindow", "Name", nullptr));
#if QT_CONFIG(tooltip)
        namelineEdit->setToolTip(QCoreApplication::translate("MainWindow", "Choose a name for the Visual Object", nullptr));
#endif // QT_CONFIG(tooltip)
        namelineEdit->setText(QCoreApplication::translate("MainWindow", "Visual Object", nullptr));
        Xlabel->setText(QCoreApplication::translate("MainWindow", "X", nullptr));
        XLogCheckBox->setText(QCoreApplication::translate("MainWindow", "Log", nullptr));
        Ylabel->setText(QCoreApplication::translate("MainWindow", "Y", nullptr));
        YLogCheckBox->setText(QCoreApplication::translate("MainWindow", "Log", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Z", nullptr));
        ZLogCheckBox->setText(QCoreApplication::translate("MainWindow", "Log", nullptr));
        ShowPointsCheckBox->setText(QCoreApplication::translate("MainWindow", "Show Points", nullptr));
        ViewSettingspushButton->setText(QCoreApplication::translate("MainWindow", "OK", nullptr));
        ScaleCheckBox->setText(QCoreApplication::translate("MainWindow", "Scale", nullptr));
        ShowAxesCheckBox->setText(QCoreApplication::translate("MainWindow", "Show Axes", nullptr));
        ShowBoxCheckBox->setText(QCoreApplication::translate("MainWindow", "Show Box", nullptr));
        XVectorsLabel->setText(QCoreApplication::translate("MainWindow", "Vector X", nullptr));
        YVectorsLabel->setText(QCoreApplication::translate("MainWindow", "Vector Y", nullptr));
        ZVectorsLabel->setText(QCoreApplication::translate("MainWindow", "Vector Z", nullptr));
        ShowVectorsCheckBox->setText(QCoreApplication::translate("MainWindow", "Show Vectors", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabViewSettings), QCoreApplication::translate("MainWindow", "View Settings", nullptr));
        importerGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Importer", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "Missing Value", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "Text Value      ", nullptr));
        tableRadioButton->setText(QCoreApplication::translate("MainWindow", "Table", nullptr));
        volumeRadioButton->setText(QCoreApplication::translate("MainWindow", "Volume", nullptr));
        volumeGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Volume", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "Number of computational cells:", nullptr));
        label_6->setText(QCoreApplication::translate("MainWindow", "Y", nullptr));
        label_5->setText(QCoreApplication::translate("MainWindow", "X", nullptr));
        label_7->setText(QCoreApplication::translate("MainWindow", "Z", nullptr));
        label_8->setText(QCoreApplication::translate("MainWindow", "Cell sizes:", nullptr));
        label_10->setText(QCoreApplication::translate("MainWindow", "Y dim", nullptr));
        label_11->setText(QCoreApplication::translate("MainWindow", "Z dim", nullptr));
        label_9->setText(QCoreApplication::translate("MainWindow", "X dim", nullptr));
        importPushButton->setText(QCoreApplication::translate("MainWindow", "Import", nullptr));
        filterGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Filter", nullptr));
        selectFilterComboBox->setItemText(0, QCoreApplication::translate("MainWindow", "Add Identifier", nullptr));
        selectFilterComboBox->setItemText(1, QCoreApplication::translate("MainWindow", "Append", nullptr));
        selectFilterComboBox->setItemText(2, QCoreApplication::translate("MainWindow", "Cartesian 2 Polar", nullptr));
        selectFilterComboBox->setItemText(3, QCoreApplication::translate("MainWindow", "Cut", nullptr));
        selectFilterComboBox->setItemText(4, QCoreApplication::translate("MainWindow", "Decimator", nullptr));
        selectFilterComboBox->setItemText(5, QCoreApplication::translate("MainWindow", "Extract List", nullptr));
        selectFilterComboBox->setItemText(6, QCoreApplication::translate("MainWindow", "Extraction", nullptr));
        selectFilterComboBox->setItemText(7, QCoreApplication::translate("MainWindow", "Grid 2 Point", nullptr));
        selectFilterComboBox->setItemText(8, QCoreApplication::translate("MainWindow", "Include", nullptr));
        selectFilterComboBox->setItemText(9, QCoreApplication::translate("MainWindow", "Math Op", nullptr));
        selectFilterComboBox->setItemText(10, QCoreApplication::translate("MainWindow", "Merge", nullptr));
        selectFilterComboBox->setItemText(11, QCoreApplication::translate("MainWindow", "Module", nullptr));
        selectFilterComboBox->setItemText(12, QCoreApplication::translate("MainWindow", "Multi Resolution", nullptr));
        selectFilterComboBox->setItemText(13, QCoreApplication::translate("MainWindow", "Point distribute", nullptr));
        selectFilterComboBox->setItemText(14, QCoreApplication::translate("MainWindow", "Point Property", nullptr));
        selectFilterComboBox->setItemText(15, QCoreApplication::translate("MainWindow", "Randomizer", nullptr));
        selectFilterComboBox->setItemText(16, QCoreApplication::translate("MainWindow", "Scalar Distribution", nullptr));
        selectFilterComboBox->setItemText(17, QCoreApplication::translate("MainWindow", "Select Columns", nullptr));
        selectFilterComboBox->setItemText(18, QCoreApplication::translate("MainWindow", "Select Rows", nullptr));
        selectFilterComboBox->setItemText(19, QCoreApplication::translate("MainWindow", "Sigma Contours", nullptr));
        selectFilterComboBox->setItemText(20, QCoreApplication::translate("MainWindow", "Split Table", nullptr));
        selectFilterComboBox->setItemText(21, QCoreApplication::translate("MainWindow", "Write VO Table", nullptr));

        filterDescriptionTextBrowser->setHtml(QCoreApplication::translate("MainWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'.SF NS Text'; font-size:13pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-family:'Arial'; color:#232323;\">This filter adds a new column to the input table. The column name is given in Column Name field, with a sequence of value starting from a Start Number value (Default value is 0).</span> </p></body></html>", nullptr));
        addIdentifierParameterGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Parameter", nullptr));
        addIdNewColName->setText(QCoreApplication::translate("MainWindow", "NewColumnName", nullptr));
        label_12->setText(QCoreApplication::translate("MainWindow", "Start Number:", nullptr));
        label_13->setText(QCoreApplication::translate("MainWindow", "New Column:", nullptr));
        addIdStartNumber->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        appendParameterGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Parameter", nullptr));
        selectedVBT->setText(QString());
        addVBTpushButton->setText(QCoreApplication::translate("MainWindow", ">>", nullptr));
        removeVBTpushButton->setText(QCoreApplication::translate("MainWindow", "<<", nullptr));
        runFilterPushButton->setText(QCoreApplication::translate("MainWindow", "Run filter", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabOpParameters), QCoreApplication::translate("MainWindow", "Op. Parameters", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
        menuImport->setTitle(QCoreApplication::translate("MainWindow", "Import", nullptr));
        menuExport->setTitle(QCoreApplication::translate("MainWindow", "Export", nullptr));
        menuEdit->setTitle(QCoreApplication::translate("MainWindow", "Edit", nullptr));
        menuView->setTitle(QCoreApplication::translate("MainWindow", "View", nullptr));
        menuAddView->setTitle(QCoreApplication::translate("MainWindow", "Add View", nullptr));
        menuTools->setTitle(QCoreApplication::translate("MainWindow", "Tools", nullptr));
        menuWindow->setTitle(QCoreApplication::translate("MainWindow", "Window", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
