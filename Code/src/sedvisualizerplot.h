#ifndef SEDVISUALIZERPLOT_H
#define SEDVISUALIZERPLOT_H

#include "sed.h"
#include <QMainWindow>
#include "qcustomplot.h"
#include "sednode.h"
#include "sedplotpointcustom.h"
#include "vtkwindow_new.h"
#include "sedfitgrid_thin.h"
#include "sedfitgrid_thick.h"

namespace Ui {
class SEDVisualizerPlot;
}

class SedFitGrid_thin;

class SedFitgrid_thick;

class SEDVisualizerPlot : public QMainWindow
{
    Q_OBJECT

public:
    explicit SEDVisualizerPlot(QList<SED *> s, vtkwindow_new *v,QWidget *parent = 0);
    ~SEDVisualizerPlot();
    void setModelFitValue(QVector<QStringList> headerAndValueList, Qt::GlobalColor color);
    void loadSavedSED(QStringList dirList);
    void setDistances(double dist);
    void setTitle(QString t);
private:
    Ui::SEDVisualizerPlot *ui;
    SED *sed;
    QHash <QString, SEDPlotPointCustom*> visualnode_hash;
    double minFlux,maxFlux, minWavelen, maxWavelen;
    vtkwindow_new *vtkwin;
    int sedCount;
    QList<SED *> sed_list;
    QList<SEDNode *> selected_sed_list;
    bool prepareInputForSedFit( SEDNode *node);
    bool prepareSelectedInputForSedFit();
    QMap<double,double> sedFitInput;
    QMap<int,QVector<double> > sedFitValues;
    QString sedFitInputF;
    QString sedFitInputErrF;
    QString sedFitInputW;
    QString sedFitInputFflag;
    QString sedFitInputUlimitString;
    double dist;
    QString idlPath;
    QString outputSedLog;
    QString outputSedResults;

    QStringList plottedSedLabels;

    void readSedFitResultsHeader(QString header);
    void readSedFitOutput(QString filename);
    void loadSedFitOutput(QString filename);
    void loadSedFitThin(QString filename);
    void loadSedFitThick(QString filename);
    bool readThinFit(QString resultPath);
    bool readThickFit(QString resultPath);

    QMap <QString, double> modelFitBands;

    QStringList columnNames;
    QMap <QString, int> resultsOutputColumn;
    QMap <double, double> collapsed_flux;
    QMultiMap <double ,SEDNode*> all_sed_node;
    SedFitGrid_thin *sd_thin;
    SedFitgrid_thick *sd_thick;
    QVector<QString > sedFitInputUlimit;
    QFutureWatcher<void> watcher;
    LoadingWidget *loading;
    int nSED;
    void addNewSEDCheckBox(QString label, int newSED);
    void addNewSEDCheckBox(QString label);
    QList<QCPGraph *> sedGraphs;
    QList<QCPGraph *> originalGraphs;
    QCPGraph* collapsedGraph;
    QList<SEDPlotPointCustom *> collapsedGraphPoints;
    QMap <int, QList<SEDPlotPointCustom *> > sedGraphPoints;
    bool multiSelectMOD;
    bool temporaryMOD;
    int temporaryRow;
    QCPGraph *temporaryGraph;
    QList<SEDPlotPointCustom *> temporaryGraphPoints;
    bool doubleClicked;

    QProcess *process;

private slots:
    void titleDoubleClick(QMouseEvent* event, QCPPlotTitle* title);
    void axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part);
    void legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item);
    void selectionChanged();
    void mousePress();
    void mouseWheel();
    void mouseRelease();
    void checkboxClicked(int cb);

    void removeSelectedGraph();
    void removeAllGraphs();
    void contextMenuRequest(QPoint pos);
    void graphClicked(QCPAbstractPlottable *plottable);
    void drawNode( SEDNode *node);
    void doThinLocalFit();
    void doThickLocalFit();

    void doThinRemoteFit();
    void doThickRemoteFit();


    void on_actionEdit_triggered();
    void on_actionFit_triggered();
    void on_actionLocal_triggered();
    void on_actionScreenshot_triggered();
    void on_actionCollapse_triggered();
    void on_TheoreticalLocaleFit_triggered();
    void on_ThinLocalFit_triggered();
    void on_ThickLocalFit_triggered();
    void on_TheoreticalRemoteFit_triggered();

    void on_logRadioButton_toggled(bool checked);

    static bool checkIDRemoteCall(QString id);
    static void executeRemoteCall(QString sedFitInputW, QString sedFitInputF,QString sedFitInputErrF, QString sedFitInputFflag, QMap<double,double> sedFitInput);
    void handleFinished();
    void on_clearAllButton_clicked();
    void on_SEDCheckboxClicked(int n);
    void on_normalRadioButton_toggled(bool checked);
    void on_actionSave_triggered();
    void on_actionLoad_triggered();
    void on_collapseCheckBox_toggled(bool checked);
    void on_multiSelectCheckBox_toggled(bool checked);
    void on_resultsTableWidget_clicked(const QModelIndex &index);
    void addNewTheoreticalFit();
    void sectionClicked(int index);
    void finishedTheoreticalLocaleFit();
    void finishedTheoreticalRemoteFit();
    void finishedThickLocalFit();
    void finishedThickRemoteFit();

    void onReadyReadStdOutput();
    void finishedThinLocalFit();
    void finishedThinRemoteFit();

    void on_theoreticalPushButton_clicked();
    void on_theorConfirmPushButton_clicked();
    void on_greyBodyPushButton_clicked();
    void on_ThinRemore_triggered();
    void on_ThickRemote_triggered();
    void on_Thick_clicked();
    void on_thinButton_clicked();
};

QDataStream &operator<<(QDataStream &out, QList<SED *> &sedlist);
QDataStream &operator>>(QDataStream &in, QList<SED *> &sedlist);


#endif // SEDVISUALIZERPLOT_H
