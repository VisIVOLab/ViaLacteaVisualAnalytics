/********************************************************************************
** Form generated from reading UI file 'sedvisualizerplot.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SEDVISUALIZERPLOT_H
#define UI_SEDVISUALIZERPLOT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
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
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "src/qcustomplot.h"

QT_BEGIN_NAMESPACE

class Ui_SEDVisualizerPlot
{
public:
    QAction *actionEdit;
    QAction *actionSelect;
    QAction *actionLocal;
    QAction *actionRemote;
    QAction *actionScreenshot;
    QAction *actionCollapse;
    QAction *TheoreticalLocaleFit;
    QAction *TheoreticalRemoteFit;
    QAction *ThinLocalFit;
    QAction *ThickLocalFit;
    QAction *actionSave;
    QAction *actionLoad;
    QAction *ThinRemore;
    QAction *ThickRemote;
    QWidget *widget;
    QVBoxLayout *verticalLayout_2;
    QSplitter *splitter_2;
    QSplitter *splitter;
    QTabWidget *atabWidget;
    QCustomPlot *customPlot;
    QCustomPlot *histogramPlot;
    QTextBrowser *outputTextBrowser;
    QTableWidget *resultsTableWidget;
    QWidget *widget_2;
    QVBoxLayout *verticalLayout_3;
    QGroupBox *visualizationModeBox;
    QVBoxLayout *verticalLayout;
    QCheckBox *multiSelectCheckBox;
    QGroupBox *showOutputBox;
    QVBoxLayout *verticalLayout_4;
    QRadioButton *logRadioButton;
    QRadioButton *resultsRadioButton;
    QGroupBox *generatedSedBox;
    QVBoxLayout *verticalLayout_5;
    QPushButton *clearAllButton;
    QCheckBox *collapseCheckBox;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_7;
    QPushButton *theoreticalPushButton;
    QPushButton *greyBodyPushButton;
    QGroupBox *theorGroupBox;
    QVBoxLayout *verticalLayout_6;
    QLabel *label_6;
    QLineEdit *delta_chi2_lineEdit;
    QLabel *label;
    QLineEdit *distTheoLineEdit;
    QLabel *label_2;
    QLineEdit *prefilterLineEdit;
    QLabel *label_3;
    QLineEdit *mid_irLineEdit;
    QLabel *label_4;
    QLineEdit *far_irLineEdit;
    QLabel *label_5;
    QLineEdit *submmLineEdit;
    QPushButton *theorConfirmPushButton;
    QGroupBox *greyBodyGroupBox;
    QVBoxLayout *verticalLayout_8;
    QPushButton *thinButton;
    QPushButton *Thick;
    QSpacerItem *verticalSpacer;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuMode;
    QMenu *menuAction;
    QMenu *menuFit;
    QMenu *menuTheoretical_model;
    QMenu *menuGrey_body;
    QMenu *menuThin;
    QMenu *menuThick;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *SEDVisualizerPlot)
    {
        if (SEDVisualizerPlot->objectName().isEmpty())
            SEDVisualizerPlot->setObjectName(QString::fromUtf8("SEDVisualizerPlot"));
        SEDVisualizerPlot->resize(800, 932);
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(SEDVisualizerPlot->sizePolicy().hasHeightForWidth());
        SEDVisualizerPlot->setSizePolicy(sizePolicy);
        actionEdit = new QAction(SEDVisualizerPlot);
        actionEdit->setObjectName(QString::fromUtf8("actionEdit"));
        actionSelect = new QAction(SEDVisualizerPlot);
        actionSelect->setObjectName(QString::fromUtf8("actionSelect"));
        actionLocal = new QAction(SEDVisualizerPlot);
        actionLocal->setObjectName(QString::fromUtf8("actionLocal"));
        actionRemote = new QAction(SEDVisualizerPlot);
        actionRemote->setObjectName(QString::fromUtf8("actionRemote"));
        actionScreenshot = new QAction(SEDVisualizerPlot);
        actionScreenshot->setObjectName(QString::fromUtf8("actionScreenshot"));
        actionCollapse = new QAction(SEDVisualizerPlot);
        actionCollapse->setObjectName(QString::fromUtf8("actionCollapse"));
        TheoreticalLocaleFit = new QAction(SEDVisualizerPlot);
        TheoreticalLocaleFit->setObjectName(QString::fromUtf8("TheoreticalLocaleFit"));
        TheoreticalRemoteFit = new QAction(SEDVisualizerPlot);
        TheoreticalRemoteFit->setObjectName(QString::fromUtf8("TheoreticalRemoteFit"));
        ThinLocalFit = new QAction(SEDVisualizerPlot);
        ThinLocalFit->setObjectName(QString::fromUtf8("ThinLocalFit"));
        ThickLocalFit = new QAction(SEDVisualizerPlot);
        ThickLocalFit->setObjectName(QString::fromUtf8("ThickLocalFit"));
        actionSave = new QAction(SEDVisualizerPlot);
        actionSave->setObjectName(QString::fromUtf8("actionSave"));
        actionLoad = new QAction(SEDVisualizerPlot);
        actionLoad->setObjectName(QString::fromUtf8("actionLoad"));
        ThinRemore = new QAction(SEDVisualizerPlot);
        ThinRemore->setObjectName(QString::fromUtf8("ThinRemore"));
        ThickRemote = new QAction(SEDVisualizerPlot);
        ThickRemote->setObjectName(QString::fromUtf8("ThickRemote"));
        widget = new QWidget(SEDVisualizerPlot);
        widget->setObjectName(QString::fromUtf8("widget"));
        sizePolicy.setHeightForWidth(widget->sizePolicy().hasHeightForWidth());
        widget->setSizePolicy(sizePolicy);
        verticalLayout_2 = new QVBoxLayout(widget);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        splitter_2 = new QSplitter(widget);
        splitter_2->setObjectName(QString::fromUtf8("splitter_2"));
        splitter_2->setOrientation(Qt::Horizontal);
        splitter = new QSplitter(splitter_2);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        QSizePolicy sizePolicy1(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(1);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(splitter->sizePolicy().hasHeightForWidth());
        splitter->setSizePolicy(sizePolicy1);
        splitter->setMinimumSize(QSize(590, 0));
        splitter->setOrientation(Qt::Vertical);
        atabWidget = new QTabWidget(splitter);
        atabWidget->setObjectName(QString::fromUtf8("atabWidget"));
        atabWidget->setEnabled(true);
        atabWidget->setMinimumSize(QSize(0, 600));
        atabWidget->setFocusPolicy(Qt::NoFocus);
        customPlot = new QCustomPlot();
        customPlot->setObjectName(QString::fromUtf8("customPlot"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(100);
        sizePolicy2.setHeightForWidth(customPlot->sizePolicy().hasHeightForWidth());
        customPlot->setSizePolicy(sizePolicy2);
        customPlot->setMinimumSize(QSize(0, 0));
        atabWidget->addTab(customPlot, QString());
        histogramPlot = new QCustomPlot();
        histogramPlot->setObjectName(QString::fromUtf8("histogramPlot"));
        histogramPlot->setEnabled(true);
        sizePolicy2.setHeightForWidth(histogramPlot->sizePolicy().hasHeightForWidth());
        histogramPlot->setSizePolicy(sizePolicy2);
        atabWidget->addTab(histogramPlot, QString());
        splitter->addWidget(atabWidget);
        outputTextBrowser = new QTextBrowser(splitter);
        outputTextBrowser->setObjectName(QString::fromUtf8("outputTextBrowser"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(1);
        sizePolicy3.setHeightForWidth(outputTextBrowser->sizePolicy().hasHeightForWidth());
        outputTextBrowser->setSizePolicy(sizePolicy3);
        outputTextBrowser->setMinimumSize(QSize(0, 10));
        outputTextBrowser->setMaximumSize(QSize(16777215, 16777215));
        splitter->addWidget(outputTextBrowser);
        resultsTableWidget = new QTableWidget(splitter);
        if (resultsTableWidget->columnCount() < 32)
            resultsTableWidget->setColumnCount(32);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        QTableWidgetItem *__qtablewidgetitem3 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(3, __qtablewidgetitem3);
        QTableWidgetItem *__qtablewidgetitem4 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(4, __qtablewidgetitem4);
        QTableWidgetItem *__qtablewidgetitem5 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(5, __qtablewidgetitem5);
        QTableWidgetItem *__qtablewidgetitem6 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(6, __qtablewidgetitem6);
        QTableWidgetItem *__qtablewidgetitem7 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(7, __qtablewidgetitem7);
        QTableWidgetItem *__qtablewidgetitem8 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(8, __qtablewidgetitem8);
        QTableWidgetItem *__qtablewidgetitem9 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(9, __qtablewidgetitem9);
        QTableWidgetItem *__qtablewidgetitem10 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(10, __qtablewidgetitem10);
        QTableWidgetItem *__qtablewidgetitem11 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(11, __qtablewidgetitem11);
        QTableWidgetItem *__qtablewidgetitem12 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(12, __qtablewidgetitem12);
        QTableWidgetItem *__qtablewidgetitem13 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(13, __qtablewidgetitem13);
        QTableWidgetItem *__qtablewidgetitem14 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(14, __qtablewidgetitem14);
        QTableWidgetItem *__qtablewidgetitem15 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(15, __qtablewidgetitem15);
        QTableWidgetItem *__qtablewidgetitem16 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(16, __qtablewidgetitem16);
        QTableWidgetItem *__qtablewidgetitem17 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(17, __qtablewidgetitem17);
        QTableWidgetItem *__qtablewidgetitem18 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(18, __qtablewidgetitem18);
        QTableWidgetItem *__qtablewidgetitem19 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(19, __qtablewidgetitem19);
        QTableWidgetItem *__qtablewidgetitem20 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(20, __qtablewidgetitem20);
        QTableWidgetItem *__qtablewidgetitem21 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(21, __qtablewidgetitem21);
        QTableWidgetItem *__qtablewidgetitem22 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(22, __qtablewidgetitem22);
        QTableWidgetItem *__qtablewidgetitem23 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(23, __qtablewidgetitem23);
        QTableWidgetItem *__qtablewidgetitem24 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(24, __qtablewidgetitem24);
        QTableWidgetItem *__qtablewidgetitem25 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(25, __qtablewidgetitem25);
        QTableWidgetItem *__qtablewidgetitem26 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(26, __qtablewidgetitem26);
        QTableWidgetItem *__qtablewidgetitem27 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(27, __qtablewidgetitem27);
        QTableWidgetItem *__qtablewidgetitem28 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(28, __qtablewidgetitem28);
        QTableWidgetItem *__qtablewidgetitem29 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(29, __qtablewidgetitem29);
        QTableWidgetItem *__qtablewidgetitem30 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(30, __qtablewidgetitem30);
        QTableWidgetItem *__qtablewidgetitem31 = new QTableWidgetItem();
        resultsTableWidget->setHorizontalHeaderItem(31, __qtablewidgetitem31);
        resultsTableWidget->setObjectName(QString::fromUtf8("resultsTableWidget"));
        sizePolicy3.setHeightForWidth(resultsTableWidget->sizePolicy().hasHeightForWidth());
        resultsTableWidget->setSizePolicy(sizePolicy3);
        resultsTableWidget->setMinimumSize(QSize(0, 10));
        resultsTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        resultsTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
        resultsTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        splitter->addWidget(resultsTableWidget);
        splitter_2->addWidget(splitter);
        widget_2 = new QWidget(splitter_2);
        widget_2->setObjectName(QString::fromUtf8("widget_2"));
        QSizePolicy sizePolicy4(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(widget_2->sizePolicy().hasHeightForWidth());
        widget_2->setSizePolicy(sizePolicy4);
        widget_2->setMinimumSize(QSize(10, 0));
        verticalLayout_3 = new QVBoxLayout(widget_2);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        visualizationModeBox = new QGroupBox(widget_2);
        visualizationModeBox->setObjectName(QString::fromUtf8("visualizationModeBox"));
        sizePolicy4.setHeightForWidth(visualizationModeBox->sizePolicy().hasHeightForWidth());
        visualizationModeBox->setSizePolicy(sizePolicy4);
        visualizationModeBox->setMinimumSize(QSize(10, 0));
        verticalLayout = new QVBoxLayout(visualizationModeBox);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        multiSelectCheckBox = new QCheckBox(visualizationModeBox);
        multiSelectCheckBox->setObjectName(QString::fromUtf8("multiSelectCheckBox"));

        verticalLayout->addWidget(multiSelectCheckBox);


        verticalLayout_3->addWidget(visualizationModeBox);

        showOutputBox = new QGroupBox(widget_2);
        showOutputBox->setObjectName(QString::fromUtf8("showOutputBox"));
        sizePolicy4.setHeightForWidth(showOutputBox->sizePolicy().hasHeightForWidth());
        showOutputBox->setSizePolicy(sizePolicy4);
        showOutputBox->setMinimumSize(QSize(10, 0));
        verticalLayout_4 = new QVBoxLayout(showOutputBox);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        logRadioButton = new QRadioButton(showOutputBox);
        logRadioButton->setObjectName(QString::fromUtf8("logRadioButton"));
        QSizePolicy sizePolicy5(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
        sizePolicy5.setHorizontalStretch(0);
        sizePolicy5.setVerticalStretch(0);
        sizePolicy5.setHeightForWidth(logRadioButton->sizePolicy().hasHeightForWidth());
        logRadioButton->setSizePolicy(sizePolicy5);
        logRadioButton->setMinimumSize(QSize(10, 0));
        logRadioButton->setChecked(true);

        verticalLayout_4->addWidget(logRadioButton);

        resultsRadioButton = new QRadioButton(showOutputBox);
        resultsRadioButton->setObjectName(QString::fromUtf8("resultsRadioButton"));
        sizePolicy5.setHeightForWidth(resultsRadioButton->sizePolicy().hasHeightForWidth());
        resultsRadioButton->setSizePolicy(sizePolicy5);
        resultsRadioButton->setMinimumSize(QSize(10, 0));

        verticalLayout_4->addWidget(resultsRadioButton);


        verticalLayout_3->addWidget(showOutputBox);

        generatedSedBox = new QGroupBox(widget_2);
        generatedSedBox->setObjectName(QString::fromUtf8("generatedSedBox"));
        generatedSedBox->setEnabled(true);
        sizePolicy4.setHeightForWidth(generatedSedBox->sizePolicy().hasHeightForWidth());
        generatedSedBox->setSizePolicy(sizePolicy4);
        generatedSedBox->setMinimumSize(QSize(10, 0));
        verticalLayout_5 = new QVBoxLayout(generatedSedBox);
        verticalLayout_5->setObjectName(QString::fromUtf8("verticalLayout_5"));
        clearAllButton = new QPushButton(generatedSedBox);
        clearAllButton->setObjectName(QString::fromUtf8("clearAllButton"));

        verticalLayout_5->addWidget(clearAllButton);


        verticalLayout_3->addWidget(generatedSedBox);

        collapseCheckBox = new QCheckBox(widget_2);
        collapseCheckBox->setObjectName(QString::fromUtf8("collapseCheckBox"));

        verticalLayout_3->addWidget(collapseCheckBox);

        groupBox_2 = new QGroupBox(widget_2);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        verticalLayout_7 = new QVBoxLayout(groupBox_2);
        verticalLayout_7->setObjectName(QString::fromUtf8("verticalLayout_7"));
        theoreticalPushButton = new QPushButton(groupBox_2);
        theoreticalPushButton->setObjectName(QString::fromUtf8("theoreticalPushButton"));

        verticalLayout_7->addWidget(theoreticalPushButton);

        greyBodyPushButton = new QPushButton(groupBox_2);
        greyBodyPushButton->setObjectName(QString::fromUtf8("greyBodyPushButton"));

        verticalLayout_7->addWidget(greyBodyPushButton);


        verticalLayout_3->addWidget(groupBox_2);

        theorGroupBox = new QGroupBox(widget_2);
        theorGroupBox->setObjectName(QString::fromUtf8("theorGroupBox"));
        theorGroupBox->setEnabled(true);
        verticalLayout_6 = new QVBoxLayout(theorGroupBox);
        verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
        label_6 = new QLabel(theorGroupBox);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        verticalLayout_6->addWidget(label_6);

        delta_chi2_lineEdit = new QLineEdit(theorGroupBox);
        delta_chi2_lineEdit->setObjectName(QString::fromUtf8("delta_chi2_lineEdit"));

        verticalLayout_6->addWidget(delta_chi2_lineEdit);

        label = new QLabel(theorGroupBox);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout_6->addWidget(label);

        distTheoLineEdit = new QLineEdit(theorGroupBox);
        distTheoLineEdit->setObjectName(QString::fromUtf8("distTheoLineEdit"));

        verticalLayout_6->addWidget(distTheoLineEdit);

        label_2 = new QLabel(theorGroupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        verticalLayout_6->addWidget(label_2);

        prefilterLineEdit = new QLineEdit(theorGroupBox);
        prefilterLineEdit->setObjectName(QString::fromUtf8("prefilterLineEdit"));

        verticalLayout_6->addWidget(prefilterLineEdit);

        label_3 = new QLabel(theorGroupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        verticalLayout_6->addWidget(label_3);

        mid_irLineEdit = new QLineEdit(theorGroupBox);
        mid_irLineEdit->setObjectName(QString::fromUtf8("mid_irLineEdit"));

        verticalLayout_6->addWidget(mid_irLineEdit);

        label_4 = new QLabel(theorGroupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        verticalLayout_6->addWidget(label_4);

        far_irLineEdit = new QLineEdit(theorGroupBox);
        far_irLineEdit->setObjectName(QString::fromUtf8("far_irLineEdit"));

        verticalLayout_6->addWidget(far_irLineEdit);

        label_5 = new QLabel(theorGroupBox);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        verticalLayout_6->addWidget(label_5);

        submmLineEdit = new QLineEdit(theorGroupBox);
        submmLineEdit->setObjectName(QString::fromUtf8("submmLineEdit"));

        verticalLayout_6->addWidget(submmLineEdit);

        theorConfirmPushButton = new QPushButton(theorGroupBox);
        theorConfirmPushButton->setObjectName(QString::fromUtf8("theorConfirmPushButton"));

        verticalLayout_6->addWidget(theorConfirmPushButton);


        verticalLayout_3->addWidget(theorGroupBox);

        greyBodyGroupBox = new QGroupBox(widget_2);
        greyBodyGroupBox->setObjectName(QString::fromUtf8("greyBodyGroupBox"));
        verticalLayout_8 = new QVBoxLayout(greyBodyGroupBox);
        verticalLayout_8->setObjectName(QString::fromUtf8("verticalLayout_8"));
        thinButton = new QPushButton(greyBodyGroupBox);
        thinButton->setObjectName(QString::fromUtf8("thinButton"));

        verticalLayout_8->addWidget(thinButton);

        Thick = new QPushButton(greyBodyGroupBox);
        Thick->setObjectName(QString::fromUtf8("Thick"));

        verticalLayout_8->addWidget(Thick);


        verticalLayout_3->addWidget(greyBodyGroupBox);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_3->addItem(verticalSpacer);

        splitter_2->addWidget(widget_2);

        verticalLayout_2->addWidget(splitter_2);

        SEDVisualizerPlot->setCentralWidget(widget);
        menubar = new QMenuBar(SEDVisualizerPlot);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 22));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuMode = new QMenu(menubar);
        menuMode->setObjectName(QString::fromUtf8("menuMode"));
        menuAction = new QMenu(menubar);
        menuAction->setObjectName(QString::fromUtf8("menuAction"));
        menuFit = new QMenu(menuAction);
        menuFit->setObjectName(QString::fromUtf8("menuFit"));
        menuTheoretical_model = new QMenu(menuFit);
        menuTheoretical_model->setObjectName(QString::fromUtf8("menuTheoretical_model"));
        menuGrey_body = new QMenu(menuFit);
        menuGrey_body->setObjectName(QString::fromUtf8("menuGrey_body"));
        menuThin = new QMenu(menuGrey_body);
        menuThin->setObjectName(QString::fromUtf8("menuThin"));
        menuThick = new QMenu(menuGrey_body);
        menuThick->setObjectName(QString::fromUtf8("menuThick"));
        SEDVisualizerPlot->setMenuBar(menubar);
        statusbar = new QStatusBar(SEDVisualizerPlot);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        SEDVisualizerPlot->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuMode->menuAction());
        menubar->addAction(menuAction->menuAction());
        menuFile->addAction(actionScreenshot);
        menuFile->addAction(actionSave);
        menuFile->addAction(actionLoad);
        menuMode->addAction(actionEdit);
        menuMode->addAction(actionSelect);
        menuAction->addAction(menuFit->menuAction());
        menuAction->addAction(actionCollapse);
        menuFit->addAction(menuTheoretical_model->menuAction());
        menuFit->addAction(menuGrey_body->menuAction());
        menuTheoretical_model->addAction(TheoreticalLocaleFit);
        menuTheoretical_model->addAction(TheoreticalRemoteFit);
        menuGrey_body->addAction(menuThin->menuAction());
        menuGrey_body->addAction(menuThick->menuAction());
        menuThin->addAction(ThinLocalFit);
        menuThin->addAction(ThinRemore);
        menuThick->addAction(ThickLocalFit);
        menuThick->addAction(ThickRemote);

        retranslateUi(SEDVisualizerPlot);

        atabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(SEDVisualizerPlot);
    } // setupUi

    void retranslateUi(QMainWindow *SEDVisualizerPlot)
    {
        SEDVisualizerPlot->setWindowTitle(QCoreApplication::translate("SEDVisualizerPlot", "MainWindow", nullptr));
        actionEdit->setText(QCoreApplication::translate("SEDVisualizerPlot", "Edit", nullptr));
#if QT_CONFIG(shortcut)
        actionEdit->setShortcut(QCoreApplication::translate("SEDVisualizerPlot", "Ctrl+E", nullptr));
#endif // QT_CONFIG(shortcut)
        actionSelect->setText(QCoreApplication::translate("SEDVisualizerPlot", "Select", nullptr));
#if QT_CONFIG(shortcut)
        actionSelect->setShortcut(QCoreApplication::translate("SEDVisualizerPlot", "Ctrl+S", nullptr));
#endif // QT_CONFIG(shortcut)
        actionLocal->setText(QCoreApplication::translate("SEDVisualizerPlot", "Local", nullptr));
        actionRemote->setText(QCoreApplication::translate("SEDVisualizerPlot", "Remote", nullptr));
        actionScreenshot->setText(QCoreApplication::translate("SEDVisualizerPlot", "Screenshot", nullptr));
        actionCollapse->setText(QCoreApplication::translate("SEDVisualizerPlot", "Collapse", nullptr));
        TheoreticalLocaleFit->setText(QCoreApplication::translate("SEDVisualizerPlot", "Local", nullptr));
        TheoreticalRemoteFit->setText(QCoreApplication::translate("SEDVisualizerPlot", "Remote", nullptr));
        ThinLocalFit->setText(QCoreApplication::translate("SEDVisualizerPlot", "Local", nullptr));
        ThickLocalFit->setText(QCoreApplication::translate("SEDVisualizerPlot", "Local", nullptr));
        actionSave->setText(QCoreApplication::translate("SEDVisualizerPlot", "Save", nullptr));
        actionLoad->setText(QCoreApplication::translate("SEDVisualizerPlot", "Load", nullptr));
        ThinRemore->setText(QCoreApplication::translate("SEDVisualizerPlot", "Remote", nullptr));
        ThickRemote->setText(QCoreApplication::translate("SEDVisualizerPlot", "Remote", nullptr));
        atabWidget->setTabText(atabWidget->indexOf(customPlot), QCoreApplication::translate("SEDVisualizerPlot", "SED", nullptr));
        atabWidget->setTabText(atabWidget->indexOf(histogramPlot), QCoreApplication::translate("SEDVisualizerPlot", "Histogram", nullptr));
        QTableWidgetItem *___qtablewidgetitem = resultsTableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("SEDVisualizerPlot", "id", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = resultsTableWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("SEDVisualizerPlot", "clump_mass", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = resultsTableWidget->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("SEDVisualizerPlot", "compact_mass_fraction", nullptr));
        QTableWidgetItem *___qtablewidgetitem3 = resultsTableWidget->horizontalHeaderItem(3);
        ___qtablewidgetitem3->setText(QCoreApplication::translate("SEDVisualizerPlot", "clump_upper_age", nullptr));
        QTableWidgetItem *___qtablewidgetitem4 = resultsTableWidget->horizontalHeaderItem(4);
        ___qtablewidgetitem4->setText(QCoreApplication::translate("SEDVisualizerPlot", "dust_temp", nullptr));
        QTableWidgetItem *___qtablewidgetitem5 = resultsTableWidget->horizontalHeaderItem(5);
        ___qtablewidgetitem5->setText(QCoreApplication::translate("SEDVisualizerPlot", "bolometric_luminosity", nullptr));
        QTableWidgetItem *___qtablewidgetitem6 = resultsTableWidget->horizontalHeaderItem(6);
        ___qtablewidgetitem6->setText(QCoreApplication::translate("SEDVisualizerPlot", "n_start_tot", nullptr));
        QTableWidgetItem *___qtablewidgetitem7 = resultsTableWidget->horizontalHeaderItem(7);
        ___qtablewidgetitem7->setText(QCoreApplication::translate("SEDVisualizerPlot", "m_star_tot", nullptr));
        QTableWidgetItem *___qtablewidgetitem8 = resultsTableWidget->horizontalHeaderItem(8);
        ___qtablewidgetitem8->setText(QCoreApplication::translate("SEDVisualizerPlot", "n_star_zams", nullptr));
        QTableWidgetItem *___qtablewidgetitem9 = resultsTableWidget->horizontalHeaderItem(9);
        ___qtablewidgetitem9->setText(QCoreApplication::translate("SEDVisualizerPlot", "m_star_zams", nullptr));
        QTableWidgetItem *___qtablewidgetitem10 = resultsTableWidget->horizontalHeaderItem(10);
        ___qtablewidgetitem10->setText(QCoreApplication::translate("SEDVisualizerPlot", "l_star_tot", nullptr));
        QTableWidgetItem *___qtablewidgetitem11 = resultsTableWidget->horizontalHeaderItem(11);
        ___qtablewidgetitem11->setText(QCoreApplication::translate("SEDVisualizerPlot", "l_star_zams", nullptr));
        QTableWidgetItem *___qtablewidgetitem12 = resultsTableWidget->horizontalHeaderItem(12);
        ___qtablewidgetitem12->setText(QCoreApplication::translate("SEDVisualizerPlot", "zams_luminosity_1", nullptr));
        QTableWidgetItem *___qtablewidgetitem13 = resultsTableWidget->horizontalHeaderItem(13);
        ___qtablewidgetitem13->setText(QCoreApplication::translate("SEDVisualizerPlot", "zams_mass_1", nullptr));
        QTableWidgetItem *___qtablewidgetitem14 = resultsTableWidget->horizontalHeaderItem(14);
        ___qtablewidgetitem14->setText(QCoreApplication::translate("SEDVisualizerPlot", "zams_temperature_1", nullptr));
        QTableWidgetItem *___qtablewidgetitem15 = resultsTableWidget->horizontalHeaderItem(15);
        ___qtablewidgetitem15->setText(QCoreApplication::translate("SEDVisualizerPlot", "zams_luminosity_2", nullptr));
        QTableWidgetItem *___qtablewidgetitem16 = resultsTableWidget->horizontalHeaderItem(16);
        ___qtablewidgetitem16->setText(QCoreApplication::translate("SEDVisualizerPlot", "zams_mass_2", nullptr));
        QTableWidgetItem *___qtablewidgetitem17 = resultsTableWidget->horizontalHeaderItem(17);
        ___qtablewidgetitem17->setText(QCoreApplication::translate("SEDVisualizerPlot", "zams_temperature_2", nullptr));
        QTableWidgetItem *___qtablewidgetitem18 = resultsTableWidget->horizontalHeaderItem(18);
        ___qtablewidgetitem18->setText(QCoreApplication::translate("SEDVisualizerPlot", "zams_luminosity_3", nullptr));
        QTableWidgetItem *___qtablewidgetitem19 = resultsTableWidget->horizontalHeaderItem(19);
        ___qtablewidgetitem19->setText(QCoreApplication::translate("SEDVisualizerPlot", "zams_mass_3", nullptr));
        QTableWidgetItem *___qtablewidgetitem20 = resultsTableWidget->horizontalHeaderItem(20);
        ___qtablewidgetitem20->setText(QCoreApplication::translate("SEDVisualizerPlot", "zams_temperature_3", nullptr));
        QTableWidgetItem *___qtablewidgetitem21 = resultsTableWidget->horizontalHeaderItem(21);
        ___qtablewidgetitem21->setText(QCoreApplication::translate("SEDVisualizerPlot", "m1", nullptr));
        QTableWidgetItem *___qtablewidgetitem22 = resultsTableWidget->horizontalHeaderItem(22);
        ___qtablewidgetitem22->setText(QCoreApplication::translate("SEDVisualizerPlot", "pacs1", nullptr));
        QTableWidgetItem *___qtablewidgetitem23 = resultsTableWidget->horizontalHeaderItem(23);
        ___qtablewidgetitem23->setText(QCoreApplication::translate("SEDVisualizerPlot", "pacs2", nullptr));
        QTableWidgetItem *___qtablewidgetitem24 = resultsTableWidget->horizontalHeaderItem(24);
        ___qtablewidgetitem24->setText(QCoreApplication::translate("SEDVisualizerPlot", "pacs3", nullptr));
        QTableWidgetItem *___qtablewidgetitem25 = resultsTableWidget->horizontalHeaderItem(25);
        ___qtablewidgetitem25->setText(QCoreApplication::translate("SEDVisualizerPlot", "spir1", nullptr));
        QTableWidgetItem *___qtablewidgetitem26 = resultsTableWidget->horizontalHeaderItem(26);
        ___qtablewidgetitem26->setText(QCoreApplication::translate("SEDVisualizerPlot", "spir2", nullptr));
        QTableWidgetItem *___qtablewidgetitem27 = resultsTableWidget->horizontalHeaderItem(27);
        ___qtablewidgetitem27->setText(QCoreApplication::translate("SEDVisualizerPlot", "spir3", nullptr));
        QTableWidgetItem *___qtablewidgetitem28 = resultsTableWidget->horizontalHeaderItem(28);
        ___qtablewidgetitem28->setText(QCoreApplication::translate("SEDVisualizerPlot", "laboc", nullptr));
        QTableWidgetItem *___qtablewidgetitem29 = resultsTableWidget->horizontalHeaderItem(29);
        ___qtablewidgetitem29->setText(QCoreApplication::translate("SEDVisualizerPlot", "bol11", nullptr));
        QTableWidgetItem *___qtablewidgetitem30 = resultsTableWidget->horizontalHeaderItem(30);
        ___qtablewidgetitem30->setText(QCoreApplication::translate("SEDVisualizerPlot", "CHI2", nullptr));
        QTableWidgetItem *___qtablewidgetitem31 = resultsTableWidget->horizontalHeaderItem(31);
        ___qtablewidgetitem31->setText(QCoreApplication::translate("SEDVisualizerPlot", "DIST", nullptr));
        visualizationModeBox->setTitle(QCoreApplication::translate("SEDVisualizerPlot", "Visualization mode:", nullptr));
        multiSelectCheckBox->setText(QCoreApplication::translate("SEDVisualizerPlot", "Multi Select", nullptr));
        showOutputBox->setTitle(QCoreApplication::translate("SEDVisualizerPlot", "Select SED output to view:", nullptr));
        logRadioButton->setText(QCoreApplication::translate("SEDVisualizerPlot", "Log", nullptr));
        resultsRadioButton->setText(QCoreApplication::translate("SEDVisualizerPlot", "Results", nullptr));
        generatedSedBox->setTitle(QCoreApplication::translate("SEDVisualizerPlot", "Generated Fit:", nullptr));
        clearAllButton->setText(QCoreApplication::translate("SEDVisualizerPlot", "Clear All", nullptr));
        collapseCheckBox->setText(QCoreApplication::translate("SEDVisualizerPlot", "Collapse All", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("SEDVisualizerPlot", "Fit", nullptr));
        theoreticalPushButton->setText(QCoreApplication::translate("SEDVisualizerPlot", "Theoretical", nullptr));
        greyBodyPushButton->setText(QCoreApplication::translate("SEDVisualizerPlot", "Grey-body", nullptr));
        theorGroupBox->setTitle(QCoreApplication::translate("SEDVisualizerPlot", "Theoretical", nullptr));
        label_6->setText(QCoreApplication::translate("SEDVisualizerPlot", "Delta chi2", nullptr));
        delta_chi2_lineEdit->setText(QCoreApplication::translate("SEDVisualizerPlot", "3", nullptr));
        label->setText(QCoreApplication::translate("SEDVisualizerPlot", "Dist", nullptr));
        distTheoLineEdit->setText(QCoreApplication::translate("SEDVisualizerPlot", "2000", nullptr));
        label_2->setText(QCoreApplication::translate("SEDVisualizerPlot", "prefilter threshold", nullptr));
        prefilterLineEdit->setText(QCoreApplication::translate("SEDVisualizerPlot", "0.8", nullptr));
        label_3->setText(QCoreApplication::translate("SEDVisualizerPlot", "mid_ir", nullptr));
        mid_irLineEdit->setText(QCoreApplication::translate("SEDVisualizerPlot", "1", nullptr));
        label_4->setText(QCoreApplication::translate("SEDVisualizerPlot", "far_ir", nullptr));
        far_irLineEdit->setText(QCoreApplication::translate("SEDVisualizerPlot", "1", nullptr));
        label_5->setText(QCoreApplication::translate("SEDVisualizerPlot", "submm", nullptr));
        submmLineEdit->setText(QCoreApplication::translate("SEDVisualizerPlot", "1", nullptr));
        theorConfirmPushButton->setText(QCoreApplication::translate("SEDVisualizerPlot", "Confirm", nullptr));
        greyBodyGroupBox->setTitle(QCoreApplication::translate("SEDVisualizerPlot", "Grey-Body", nullptr));
        thinButton->setText(QCoreApplication::translate("SEDVisualizerPlot", "Thin", nullptr));
        Thick->setText(QCoreApplication::translate("SEDVisualizerPlot", "Thick", nullptr));
        menuFile->setTitle(QCoreApplication::translate("SEDVisualizerPlot", "File", nullptr));
        menuMode->setTitle(QCoreApplication::translate("SEDVisualizerPlot", "Mode", nullptr));
        menuAction->setTitle(QCoreApplication::translate("SEDVisualizerPlot", "Action", nullptr));
        menuFit->setTitle(QCoreApplication::translate("SEDVisualizerPlot", "Fit", nullptr));
        menuTheoretical_model->setTitle(QCoreApplication::translate("SEDVisualizerPlot", "Theoretical model", nullptr));
        menuGrey_body->setTitle(QCoreApplication::translate("SEDVisualizerPlot", "Grey-body", nullptr));
        menuThin->setTitle(QCoreApplication::translate("SEDVisualizerPlot", "Thin", nullptr));
        menuThick->setTitle(QCoreApplication::translate("SEDVisualizerPlot", "Thick", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SEDVisualizerPlot: public Ui_SEDVisualizerPlot {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SEDVISUALIZERPLOT_H
