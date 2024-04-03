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
    /**
     * Holding 'shift' allows graph navigation (by disabling drag selection)
     * Holding 'Control/Command' allows multi selection/deselection
     * @brief SEDVisualizerPlot::keyPressEvent
     * @param event
     */
    void keyPressEvent(QKeyEvent *event) override;
    /**
     * Reset drag selection mode realeasing 'shift'
     * Reset multi selection mode realeasing 'Control/Command'
     * @brief SEDVisualizerPlot::keyReleaseEvent
     * @param event
     */
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    Ui::SEDVisualizerPlot *ui;
    QHash<QString, SEDPlotPointCustom *> visualnode_hash;
    double minFlux, maxFlux, minWavelen, maxWavelen;
    vtkwindow_new *vtkwin;
    int sedCount;
    QList<SED *> sed_list;
    // ellipse
    vtkSmartPointer<vtkLODActor> ellipseActor;
    QList<vtkSmartPointer<vtkLODActor>> ellipseActorList;
    void drawSingleEllipse(vtkEllipse *el);
    void removeAllEllipse(QList<vtkSmartPointer<vtkLODActor>> ellipseActorList);

    // multi selection status to avoid deselecting pending nodes
    bool multiSelectionPointStatus;
    // shift key press status to avoid deselecting pending nodes
    bool shiftMovingStatus;
    // data structure to support drag item selection
    QMap<QPair<double, double>, QCPAbstractItem *> sed_coordinte_to_element;
    // setting drag selection
    void setDragSelection();
    /**
     * Manage the Tooltip information over mouse cursor event
     * @brief SEDVisualizerPlot::handleMouseMove
     * @param event mouse position
     */
    void handleMouseMove(QMouseEvent *event);
    /**
     * Detect distance and SED node closest to the mouse cursor
     * @brief SEDVisualizerPlot::closestSEDNode
     * @param mouseX Axis mouse
     * @param mouseY Axis mouse
     * @return pair element of closest SED node to mouse cursor and his distance
     */
    QPair<QCPAbstractItem *, double> closestSEDNode(double mouseX, double mouseY);
    /**
     * This method filters the root SEDNodes to be displayed in the graph.
     * All root nodes have references to their child nodes.
     * @brief SEDVisualizerPlot::filterSEDNodes
     * @param sedList
     * @return SEDNode list
     */
    QList<SEDNode *> filterSEDNodes(QList<SED *> sedList);

    // add info ToolTip
    VialacteaStringDictWidget *stringDictWidget;
    QString designation;
    double image_x;
    double image_y;
    double glon;
    double glat;
    double error_flux;

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
    /**
     * Extract the wavelength, flux, and flux-error data from a given sed node.
     * The extracted data is appended to the provided QVector objects.
     * @brief SEDVisualizerPlot::addCoordinatesData
     * @param node SEDNode*. The method will visit this node and all its descendants.
     * @param x Reference QVector of double. Collect the wavelength data of each node.
     * @param y Reference QVector of double. Collect flux data of each node.
     * @param y_err Reference QVector of double. Collect flux-error data of each node.
     */
    void addCoordinatesData(SEDNode *node, QVector<double> &x, QVector<double> &y,
                            QVector<double> &y_err, QSet<QString> &visitedNodes);

    void readColumnsFromSedFitResults(const QJsonArray &columns);
    void plotSedFitModel(const QJsonArray &model, Qt::GlobalColor color);

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
    void axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part);
    // void legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item);
    void selectionChanged();
    void mousePress(QMouseEvent *event);
    void mouseWheel();
    void mouseRelease(QMouseEvent *event);
    void checkboxClicked(int cb);

    void removeSelectedGraph();
    void removeAllGraphs();
    // void contextMenuRequest(QPoint pos);
    /**
     * Draw an edge bethween SEDNode<root> and its child
     * @brief SEDVisualizerPlot::drawPlot
     * @param SEDNode
     */
    void drawPlot(SEDNode *node);
    /**
     * @brief SEDVisualizerPlot::drawNode Draw selectable sed nodes and their flux error
     * @param sedlist A list of sed objects to be visualized
     */
    void drawNodes(QList<SEDNode *> sedlist);
    /**
     * Insert new SEDNode point into all_sed_node
     * The method sets the color, position, designation, X, Y, latitude, longitude, error flux, and
     * ellipse of the SEDPlotPointCustom object. It also updates the maximum and minimum wavelength
     * and flux values.
     * @brief SEDVisualizerPlot::updateSEDPlotPoint
     * @param node new SEDNode to insert
     */
    void updateSEDPlotPoint(SEDNode *node);
    void doThinLocalFit();
    void doThickLocalFit();

    void doThinRemoteFit();
    void doThickRemoteFit();

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
