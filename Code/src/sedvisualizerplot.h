#ifndef SEDVISUALIZERPLOT_H
#define SEDVISUALIZERPLOT_H

#include "qcustomplot.h"
#include "sed.h"
#include "sedfitgrid_thick.h"
#include "sedfitgrid_thin.h"
#include "sednode.h"
#include "sedplotpointcustom.h"
#include "vtkwindow_new.h"

#include <QMainWindow>

namespace Ui {
class SEDVisualizerPlot;
}

class SedFitGrid_thin;

class SedFitgrid_thick;

class SEDVisualizerPlot : public QMainWindow
{
    Q_OBJECT

public:
    explicit SEDVisualizerPlot(QList<SED *> s, vtkwindow_new *v, QWidget *parent = 0);
    ~SEDVisualizerPlot();
    void setModelFitValue(QVector<QStringList> headerAndValueList, Qt::GlobalColor color);
    void loadSavedSED(QStringList dirList);
    void setDistances(double dist);
    void setTitle(QString t);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    Ui::SEDVisualizerPlot *ui;
    //SED *sed; // TODO da rimuovere
    QHash<QString, SEDPlotPointCustom *> visualnode_hash;
    double minFlux, maxFlux, minWavelen, maxWavelen;
    vtkwindow_new *vtkwin;
    int sedCount;
    QList<SED *> sed_list;
    QList<SEDNode *> selected_sed_list;
    QMap<int, bool> selectedPointsMap; // TODO da rimuovere?
    // multi selection status to avoid deselecting pending nodes
    bool multiSelectionPointStatus;
    // shift key press status to avoid deselecting pending nodes
    bool shiftMovingStatus;
    // data structure to support drag item selection
    QMap<QPair<double, double>, QCPAbstractItem*> sed_coordinte_to_element;
    // setting drag selection
    void setDragSelection();
    // test mouse over
    void handleMouseMove(QMouseEvent *event);
    // calcolo del nodo più vicino
    QPair<QCPAbstractItem*, double> closestSEDNode(double mouseX, double mouseY);
    // filtro sed list
    QList <SEDNode *> filterSEDNodes(QList<SED *> listsed);

    // info ToolTip
    VialacteaStringDictWidget *stringDictWidget;
    QString designation;
    double image_x;
    double image_y;
    double glon;
    double glat;
    double error_flux;


    bool prepareInputForSedFit(SEDNode *node);
    bool prepareSelectedInputForSedFit();
    QMap<double, double> sedFitInput;
    QMap<int, QVector<double>> sedFitValues;
    QString sedFitInputF;
    QString sedFitInputErrF;
    QString sedFitInputW;
    QString sedFitInputFflag;
    QString sedFitInputUlimitString;
    double dist;
    QString outputSedLog;
    QString outputSedResults;
    QMap<double, QJsonArray> models;

    QStringList plottedSedLabels;

    void addCoordinatesData(SEDNode* node, QVector<double>& x, QVector<double>& y, QVector<double>& y_err, QSet<QString>& visitedNodes);

    void readColumnsFromSedFitResults(const QJsonArray &columns);
    void plotSedFitModel(const QJsonArray &model, Qt::GlobalColor color);

    void readSedFitResultsHeader(QString header);
    void readSedFitOutput(QString filename);
    void loadSedFitOutput(QString filename);
    void loadSedFitThin(QString filename);
    void loadSedFitThick(QString filename);
    bool readThinFit(QString resultPath);
    bool readThickFit(QString resultPath);

    QMap<QString, double> modelFitBands;

    QStringList columnNames;
    QMap<QString, int> resultsOutputColumn;
    QMap<double, double> collapsed_flux;
    QMultiMap<double, SEDNode *> all_sed_node;
    SedFitGrid_thin *sd_thin;
    SedFitgrid_thick *sd_thick;
    QVector<QString> sedFitInputUlimit;
    QFutureWatcher<void> watcher;
    LoadingWidget *loading;
    int nSED;
    void addNewSEDCheckBox(QString label, int newSED);
    void addNewSEDCheckBox(QString label);
    QList<QCPGraph *> sedGraphs;
    QList<QCPGraph *> originalGraphs;
    QCPGraph *graphSEDNodes;
    QCPGraph *collapsedGraph;
    QList<SEDPlotPointCustom *> collapsedGraphPoints;
    QMap<int, QList<SEDPlotPointCustom *>> sedGraphPoints;
    bool temporaryMOD;
    int temporaryRow;
    QCPGraph *temporaryGraph;
    QList<SEDPlotPointCustom *> temporaryGraphPoints;
    bool doubleClicked;

    QProcess *process;

private slots:
    void titleDoubleClick(QMouseEvent *event, QCPTextElement *title);
    void axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part);
    void legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item);
    void mousePress(QMouseEvent *event);
    void mouseWheel();
    void mouseRelease(QMouseEvent *event);
    void checkboxClicked(int cb);

    void removeSelectedGraph();
    void removeAllGraphs();
    //void contextMenuRequest(QPoint pos);
    void graphClicked(QCPAbstractPlottable *plottable);
    void drawPlot(SEDNode *node);
    void drawNodes(QList<SEDNode *> sedlist);
    void updateSEDPlotPoint(SEDNode *node);
    void doThinLocalFit();
    void doThickLocalFit();

    void doThinRemoteFit();
    void doThickRemoteFit();

    void on_actionEdit_triggered();
    void on_actionFit_triggered();
    void on_actionLocal_triggered();
    void on_actionScreenshot_triggered();
    void on_actionCollapse_triggered();
    void on_ThinLocalFit_triggered();
    void on_ThickLocalFit_triggered();
    void on_TheoreticalRemoteFit_triggered();

    void on_logRadioButton_toggled(bool checked);

    static bool checkIDRemoteCall(QString id);
    static void executeRemoteCall(QString sedFitInputW, QString sedFitInputF,
                                  QString sedFitInputErrF, QString sedFitInputFflag,
                                  QMap<double, double> sedFitInput);
    void handleFinished();
    void on_clearAllButton_clicked();
    void on_SEDCheckboxClicked(int n);
    void on_actionSave_triggered();
    void on_actionLoad_triggered();
    void on_collapseCheckBox_toggled(bool checked);
    void on_resultsTableWidget_clicked(const QModelIndex &index);
    void addNewTheoreticalFit();
    void sectionClicked(int index);
    void finishedTheoreticalRemoteFit();
    void finishedThickLocalFit();

    void onReadyReadStdOutput();
    void finishedThinLocalFit();
    void finishedThinRemoteFit();

    void on_theoreticalPushButton_clicked();
    void on_theorConfirmPushButton_clicked();
    void on_greyBodyPushButton_clicked();
    void on_Thick_clicked();
    void on_thinButton_clicked();
};

QDataStream &operator<<(QDataStream &out, QList<SED *> &sedlist);
QDataStream &operator>>(QDataStream &in, QList<SED *> &sedlist);

#endif // SEDVISUALIZERPLOT_H
