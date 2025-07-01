#include "sedvisualizerplot.h"
#include "loadingwidget.h"
#include "sedfitgrid_thin.h"
#include "sedplotpointcustom.h"
#include "singleton.h"
#include "ui_sedfitgrid_thick.h"
#include "ui_sedfitgrid_thin.h"
#include "ui_sedvisualizerplot.h"
#include "visivoimporterdesktop.h"
#include "vlkbquery.h"
#include "vtkCleanPolyData.h"

#include <limits>
#include <QCheckBox>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QSettings>
#include <QSignalMapper>
#include <QtConcurrent>
#include <QtMath>
#include <QToolTip>

SEDVisualizerPlot::SEDVisualizerPlot(QList<SED *> s, vtkwindow_new *v, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::SEDVisualizerPlot)
{
    ui->setupUi(this);
    ui->theorGroupBox->hide();
    ui->greyBodyGroupBox->hide();

    QString m_sSettingsFile = QDir::homePath().append("/VisIVODesktopTemp/setting.ini");
    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    this->pythonExe = settings.value("python.exe").toString();

    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes
                                    | QCP::iSelectPlottables | QCP::iMultiSelect
                                    | QCP::iSelectItems);

    sed_list = s;
    vtkwin = v;

    // logfile print table
    ui->resultsTableWidget->hide();
    ui->resultsTableWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
    QAction *addFitAction = new QAction("Add this fit", ui->resultsTableWidget);
    ui->resultsTableWidget->addAction(addFitAction);
    connect(addFitAction, SIGNAL(triggered()), this, SLOT(addNewTheoreticalFit()));

    // ticker to generate axes labels
    QSharedPointer<QCPAxisTickerLog> logTicker(new QCPAxisTickerLog);
    logTicker->setLogBase(10);
    ui->customPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->customPlot->yAxis->setTicker(logTicker);
    ui->customPlot->xAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->customPlot->xAxis->setTicker(logTicker);
    ui->customPlot->plotLayout()->insertRow(0); // insert a new row on the graphic layout
    ui->customPlot->plotLayout()->addElement(
            0, 0, new QCPTextElement(ui->customPlot, "SED")); // set title "SED"

    QString sMu = u8"\u00b5";
    QString sLabelTextX = "Wavelength [" + sMu + "m]";
    ui->customPlot->xAxis->setLabel(sLabelTextX);
    ui->customPlot->yAxis->setLabel("Flux [Jy]");

    minWavelen = std::numeric_limits<int>::max();
    maxWavelen = std::numeric_limits<int>::min();
    minFlux = std::numeric_limits<int>::max();
    maxFlux = std::numeric_limits<int>::min();

    // draw SEDNodes
    // avoid multiple SEDNodes and edge graphing
    QList<SEDNode *> duplicateFreeSEDNodes = filterSEDNodes(sed_list);
    // qDebug() << s.count() << "root_nodes from da sed_list";
    for (sedCount = 0; sedCount < duplicateFreeSEDNodes.count(); sedCount++) {
        SEDNode *sed = duplicateFreeSEDNodes.at(sedCount);
        // qDebug() << "-- draw root_nodes"<< sed->getRootNode()->getDesignation();
        drawPlot(sed);
        // create Ellipse for each root and child filtered
        createEllipseActors(sed);
        createEllipseActors(sed->getChild().values()[0]);
    }
    // qDebug() <<"-- how many graph()"<< ui->customPlot->graphCount();
    drawNodes(duplicateFreeSEDNodes);

    // set drag selection parameters
    setDragSelection();
    multiSelectionPointStatus = false;
    shiftMovingStatus = false;

    stringDictWidget = &Singleton<VialacteaStringDictWidget>::Instance();

    double x_deltaRange = (maxWavelen - minWavelen) * 0.02;
    double y_deltaRange = (maxFlux - minFlux) * 0.02;
    ui->customPlot->yAxis->setRange(minFlux - y_deltaRange, maxFlux + y_deltaRange);
    ui->customPlot->xAxis->setRange(minWavelen - x_deltaRange, maxWavelen + x_deltaRange);
    connect(ui->customPlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
    connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent *)), this,
            SLOT(mousePress(QMouseEvent *)));
    connect(ui->customPlot, SIGNAL(mouseWheel(QWheelEvent *)), this, SLOT(mouseWheel()));
    connect(ui->customPlot, SIGNAL(mouseRelease(QMouseEvent *)), this,
            SLOT(mouseRelease(QMouseEvent *)));
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2,
            SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2,
            SLOT(setRange(QCPRange)));
    connect(ui->customPlot,
            SIGNAL(axisDoubleClick(QCPAxis *, QCPAxis::SelectablePart, QMouseEvent *)), this,
            SLOT(axisLabelDoubleClick(QCPAxis *, QCPAxis::SelectablePart)));
    // ui->customPlot->setContextMenuPolicy(Qt::CustomContextMenu); // TODO to place again?
    // connect(ui->customPlot, SIGNAL(customContextMenuRequested(QPoint)), this,
    // SLOT(contextMenuRequest(QPoint)));
    connect(ui->customPlot, &QCustomPlot::mouseMove, this, &SEDVisualizerPlot::handleMouseMove);

    modelFitBands.insert("wise1", 3.4);
    modelFitBands.insert("i1", 3.6);
    modelFitBands.insert("i2", 4.5);
    modelFitBands.insert("wise2", 4.6);
    modelFitBands.insert("i3", 5.8);
    modelFitBands.insert("i4", 8.0);
    modelFitBands.insert("wise3", 12.0);
    modelFitBands.insert("wise4", 22.0);
    modelFitBands.insert("m1", 24.0);
    modelFitBands.insert("pacs1", 70.0);
    modelFitBands.insert("pacs2", 100.0);
    modelFitBands.insert("pacs3", 160.0);
    modelFitBands.insert("spir1", 250.0);
    modelFitBands.insert("spir2", 350.0);
    modelFitBands.insert("spir3", 500.0);
    modelFitBands.insert("laboc", 870.0);
    modelFitBands.insert("bol11", 1100.0);

    // columnNames.append("id");
    columnNames.append("clump_mass");
    columnNames.append("compact_mass_fraction");
    columnNames.append("clump_upper_age");
    columnNames.append("dust_temp");
    columnNames.append("bolometric_luminosity");
    columnNames.append("n_star_tot");
    columnNames.append("m_star_tot");
    columnNames.append("n_star_zams");
    columnNames.append("m_star_zams");
    columnNames.append("l_star_tot");
    columnNames.append("l_star_zams");
    columnNames.append("zams_luminosity_1");
    columnNames.append("zams_mass_1");
    columnNames.append("zams_temperature_1");
    columnNames.append("zams_luminosity_2");
    columnNames.append("zams_mass_2");
    columnNames.append("zams_temperature_2");
    columnNames.append("zams_luminosity_3");
    columnNames.append("zams_mass_3");
    columnNames.append("zams_temperature_3");
    columnNames.append("chi2");
    columnNames.append("d");
    columnNames.append("wise1");
    columnNames.append("i1");
    columnNames.append("i2");
    columnNames.append("wise2");
    columnNames.append("i3");
    columnNames.append("i4");
    columnNames.append("wise3");
    columnNames.append("wise4");
    columnNames.append("m1");
    columnNames.append("pacs1");
    columnNames.append("pacs2");
    columnNames.append("pacs3");
    columnNames.append("spir1");
    columnNames.append("spir2");
    columnNames.append("spir3");
    columnNames.append("laboc");
    columnNames.append("bol11");

    // resultsOutputColumn.insert("id", -1);
    resultsOutputColumn.insert("clump_mass", -1);
    resultsOutputColumn.insert("compact_mass_fraction", -1);
    resultsOutputColumn.insert("clump_upper_age", -1);
    resultsOutputColumn.insert("dust_temp", -1);
    resultsOutputColumn.insert("bolometric_luminosity", -1);
    resultsOutputColumn.insert("n_star_tot", -1);
    resultsOutputColumn.insert("m_star_tot", -1);
    resultsOutputColumn.insert("n_star_zams", -1);
    resultsOutputColumn.insert("m_star_zams", -1);
    resultsOutputColumn.insert("l_star_tot", -1);
    resultsOutputColumn.insert("l_star_zams", -1);
    resultsOutputColumn.insert("zams_luminosity_1", -1);
    resultsOutputColumn.insert("zams_mass_1", -1);
    resultsOutputColumn.insert("zams_temperature_1", -1);
    resultsOutputColumn.insert("zams_luminosity_2", -1);
    resultsOutputColumn.insert("zams_mass_2", -1);
    resultsOutputColumn.insert("zams_temperature_2", -1);
    resultsOutputColumn.insert("zams_luminosity_3", -1);
    resultsOutputColumn.insert("zams_mass_3", -1);
    resultsOutputColumn.insert("zams_temperature_3", -1);
    resultsOutputColumn.insert("chi2", -1);
    resultsOutputColumn.insert("d", -1);
    resultsOutputColumn.insert("wmod", -1);
    resultsOutputColumn.insert("fmod", -1);

    ui->generatedSedBox->setHidden(true);
    nSED = 0;
    temporaryMOD = false;
    doubleClicked = false;
    temporaryRow = 0;
    this->setFocus();
    this->activateWindow(); // set the focus on this windows
    QFile sedFile(QDir::homePath()
                          .append(QDir::separator())
                          .append("VisIVODesktopTemp/tmp_download/SEDList.dat"));
    if (!sedFile.exists()) {
        sedFile.open(QIODevice::WriteOnly);
        QDataStream out(&sedFile);
        out << sed_list; // serialize the object
    }
    sedFile.flush();
    sedFile.close();

    for (int i = 0; i < ui->customPlot->graphCount() - 1; i++) {
        originalGraphs.push_back(ui->customPlot->graph(i));
    }

    ui->histogramPlot->hide();
    connect(ui->resultsTableWidget->horizontalHeader(), SIGNAL(sectionClicked(int)), this,
            SLOT(sectionClicked(int)));

    // create collapsed Nodes and set invisible
    createCollapseNodes();
}

QDataStream &operator<<(QDataStream &out, QList<SED *> &sedlist)
{
    out << sedlist.count();
    foreach (SED *sed, sedlist) {
        out << sed;
    }
    return out;
}

void SEDVisualizerPlot::setTitle(QString t)
{
    this->setWindowTitle(t);
}

void SEDVisualizerPlot::setDistances(double dist)
{
    ui->distTheoLineEdit->setText(QString::number(dist));
}

void SEDVisualizerPlot::sectionClicked(int index)
{
    if (index >= 1 && index <= 4) {
        ui->histogramPlot->clearPlottables();
        QVector<double> values = sedFitValues.value(index + 1);
        double min, max, bin_width;
        QString title;
        if (index == 1) { // clump_mass
            min = 1;
            max = 5.3;
            bin_width = 0.1;
            title = "clump_mass (log scale)";
        }
        if (index == 2) { // compact_mass_fraction
            min = 0;
            max = 0.5;
            bin_width = 0.025;
            title = "compact_mass_fraction";
        }
        if (index == 3) { // clump_upper_age
            min = 4;
            max = 5.7;
            bin_width = 0.25;
            title = "clump_upper_age (log scale)";
        }
        if (index == 4) { // dust_temp
            min = 5;
            max = 50;
            bin_width = 1;
            title = "dust_temp";
        }
        ui->histogramPlot->show();
        int bin_count = (max - min) / bin_width;
        QVector<double> x1(bin_count), y1(bin_count);
        for (int i = 0; i < values.size(); ++i) {
            int bin;
            if (index == 1) {
                bin = (int)((log10(values.at(i)) - min) / bin_width);
            } else if (index == 3) {
                bin = (int)((log10(values.at(i) * 100000) - min) / bin_width);
            } else {
                bin = (int)((values.at(i) - min) / bin_width);
            }
            if (bin >= 0 && bin < bin_count) {
                y1[bin]++;
            }
        }
        x1[0] = min;
        for (int i = 1; i < bin_count; ++i) {
            x1[i] = x1[i - 1] + bin_width;
        }
        QCPBars *bars1 = new QCPBars(ui->histogramPlot->xAxis, ui->histogramPlot->yAxis);
        bars1->setWidth(bin_width);
        bars1->setData(x1, y1);
        ui->histogramPlot->xAxis->setRange(min, max);
        ui->histogramPlot->yAxis->setRange(0, values.size());

        ui->histogramPlot->xAxis->setLabel(title);

        QSharedPointer<QCPAxisTickerFixed> fixedTicker(new QCPAxisTickerFixed);
        ui->histogramPlot->yAxis->setTicker(fixedTicker);
        fixedTicker->setTickStep(1.0); // tick step shall be 1.0
        ui->histogramPlot->replot();
    } else {
        QMessageBox msgBox;
        msgBox.setText("No histogram available for this values.");
        msgBox.exec();
    }
}

QDataStream &operator>>(QDataStream &in, QList<SED *> &sedlist)
{
    int count;
    in >> count;
    for (int i = 0; i < count; i++) {
        SED *sed = new SED();
        in >> sed;
        sedlist.push_back(sed);
    }
    return in;
}

/**
 * This method filters the root SEDNodes to be displayed in the graph.
 * All root nodes have references to their child nodes.
 * @brief SEDVisualizerPlot::filterSEDNodes
 * @param sedList
 * @return SEDNode list
 */
QList<SEDNode *> SEDVisualizerPlot::filterSEDNodes(QList<SED *> sedList)
{

    QList<SEDNode *> filterSedList;
    QSet<QPair<QString, QString>> seenPairs; // support data structure

    for (auto &sed : sedList) {
        SEDNode *node = sed->getRootNode();
        while (node->hasChild()) {
            QString currentDesignation = node->getDesignation();
            QString childDesignation = node->getChild().values()[0]->getDesignation();
            // check pair: root-child
            QPair<QString, QString> currentPair(currentDesignation, childDesignation);
            // check pair: child-root
            QPair<QString, QString> currentPairVice(childDesignation, currentDesignation);
            // new edge
            if (!seenPairs.contains(currentPair) or !seenPairs.contains(currentPairVice)) {
                // new
                seenPairs.insert(currentPair);
                seenPairs.insert(currentPairVice);
                // qDebug() << "arco lecito" << currentPair << node->getWavelength() <<
                // node->getFlux() << node->getChild().values()[0]->getWavelength() <<
                // node->getChild().values()[0]->getFlux();
                filterSedList.append(node);
            }
            node = node->getChild().values()[0];
        }
    }
    return filterSedList;
}

/**
 * Insert new SEDNode point into all_sed_node
 * The method sets the color, position, designation, X, Y, latitude, longitude, error flux, and
 * ellipse of the SEDPlotPointCustom object. It also updates the maximum and minimum wavelength and
 * flux values.
 * @brief SEDVisualizerPlot::updateSEDPlotPoint
 * @param node new SEDNode to insert
 */
void SEDVisualizerPlot::updateSEDPlotPoint(SEDNode *node)
{
    if (!visualnode_hash.contains(node->getDesignation())) {
        SEDPlotPointCustom *cp = new SEDPlotPointCustom(ui->customPlot, 3.5, vtkwin);
        if (vtkwin != 0) {
            // set cp rgb color
            if (vtkwin->getDesignation2fileMap().values(node->getDesignation()).length() > 0) {
                double r = vtkwin->getEllipseActorList()
                                   .value(vtkwin->getDesignation2fileMap().value(
                                           node->getDesignation()))
                                   ->GetProperty()
                                   ->GetColor()[0];
                double g = vtkwin->getEllipseActorList()
                                   .value(vtkwin->getDesignation2fileMap().value(
                                           node->getDesignation()))
                                   ->GetProperty()
                                   ->GetColor()[1];
                double b = vtkwin->getEllipseActorList()
                                   .value(vtkwin->getDesignation2fileMap().value(
                                           node->getDesignation()))
                                   ->GetProperty()
                                   ->GetColor()[2];

                cp->setColor(QColor(r * 255, g * 255, b * 255));
            }
        }
        // set cp component
        cp->setAntialiased(true);
        cp->setPos(node->getWavelength(), node->getFlux());
        cp->setDesignation(node->getDesignation());
        cp->setX(node->getX());
        cp->setY(node->getY());
        cp->setLat(node->getLat());
        cp->setLon(node->getLon());
        cp->setErrorFlux(node->getErrFlux());
        cp->setEllipse(node->getSemiMinorAxisLength(), node->getSemiMajorAxisLength(),
                       node->getAngle(), node->getArcpix());
        cp->setNode(node);

        // new visualnode_hash entry
        visualnode_hash.insert(node->getDesignation(), cp);

        if (node->getWavelength() > maxWavelen)
            maxWavelen = node->getWavelength();
        if (node->getWavelength() < minWavelen)
            minWavelen = node->getWavelength();

        if (node->getFlux() > maxFlux)
            maxFlux = node->getFlux();
        if (node->getFlux() < minFlux) {
            minFlux = node->getFlux();
        }
        all_sed_node.insert(node->getWavelength(), node);
        sed_coordinte_to_element.insert(qMakePair(node->getWavelength(), node->getFlux()), cp);
    }
}

/**
 * Detect distance and SED node closest to the mouse cursor
 * @brief SEDVisualizerPlot::closestSEDNode
 * @param mouseX Axis mouse
 * @param mouseY Axis mouse
 * @return pair element of closest SED node to mouse cursor and his distance
 */
QPair<QCPAbstractItem *, double> SEDVisualizerPlot::closestSEDNode(double mouseX, double mouseY)
{
    QCPAbstractItem *closestSED = nullptr;
    double minDistance = std::numeric_limits<double>::max();

    for (const auto &key : sed_coordinte_to_element.keys()) {
        double x = ui->customPlot->xAxis->coordToPixel(key.first);
        double y = ui->customPlot->yAxis->coordToPixel(key.second);

        double distance = qSqrt(qPow(mouseX - x, 2) + qPow(mouseY - y, 2));

        if (distance < minDistance) {
            minDistance = distance;
            closestSED = sed_coordinte_to_element[key];
        }
    }

    return qMakePair(closestSED, minDistance);
}

/**
 * Manage the Tooltip information
 * @brief SEDVisualizerPlot::handleMouseMove
 * @param event mouse position
 */
void SEDVisualizerPlot::handleMouseMove(QMouseEvent *event)
{
    double x = event->pos().x();
    double y = event->pos().y();

    QPair<QCPAbstractItem *, double> distance = closestSEDNode(x, y);

    // threshold distance
    if (distance.second <= 8) {
        SEDPlotPointCustom *closestSED = qobject_cast<SEDPlotPointCustom *>(distance.first);
        QString toolTipText;
        if (closestSED->getNode()->getX() != 0 && closestSED->getNode()->getY() != 0
            && closestSED->getNode()->getDesignation().compare("") != 0) {
            QString wav = QString::number(closestSED->getNode()->getWavelength());
            toolTipText =
                    QString(stringDictWidget->getColUtypeStringDict().value(
                                    "compactsources.sed_view_final.designation" + wav)
                            + ":\n%1\nwavelength: %2\nfint: %3\nerr_fint: %4\nglon: %5\nglat: "
                              "%6\nx: %7\ny: %8")
                            .arg(closestSED->getNode()->getDesignation())
                            .arg(closestSED->getNode()->getWavelength())
                            .arg(closestSED->getNode()->getFlux())
                            .arg(closestSED->getNode()->getErrFlux())
                            .arg(closestSED->getNode()->getLon())
                            .arg(closestSED->getNode()->getLat())
                            .arg(closestSED->getNode()->getX())
                            .arg(closestSED->getNode()->getY());
            QToolTip::showText(event->globalPos(), toolTipText);
        } else {
            toolTipText = QString("wavelength: %1\nfint: %2\nerr_fin: %3")
                                  .arg(closestSED->getNode()->getWavelength())
                                  .arg(closestSED->getNode()->getFlux())
                                  .arg(closestSED->getNode()->getErrFlux());
            QToolTip::showText(event->globalPos(), toolTipText);
        }
    } else {
        QToolTip::hideText();
    }
}

/**
 * Draw an edge bethween SEDNode<root> and its child
 * @brief SEDVisualizerPlot::drawPlot
 * @param SEDNode
 */
void SEDVisualizerPlot::drawPlot(SEDNode *node)
{
    QVector<double> x(2), y(2);
    // on 0 set current node values
    x[0] = node->getWavelength();
    y[0] = node->getFlux();
    // on 1 set child node values
    x[1] = node->getChild().values()[0]->getWavelength();
    y[1] = node->getChild().values()[0]->getFlux();

    // plot edge
    ui->customPlot->addGraph();
    ui->customPlot->graph()->setData(x, y);
    ui->customPlot->graph()->setScatterStyle(
            QCPScatterStyle(QCPScatterStyle::ssNone)); // nodes not selectables
    ui->customPlot->graph()->setSelectable(QCP::stNone); // edge not selectable

    // plot nodes rgb and update external structures (if it's a new entry nodes)
    updateSEDPlotPoint(node);
    updateSEDPlotPoint(node->getChild().values()[0]);
}

/** Creation of all ellipse actors of SEDNodes in vtkwin on off visibility
 * @brief SEDVisualizerPlot::createEllipseActors
 * @param node SEDNode to create the ellipse
 */
void SEDVisualizerPlot::createEllipseActors(SEDNode *node)
{
    if (vtkwin != 0
        && (node->getX() != 0 && node->getY() != 0 && node->getDesignation().compare("") != 0)) {
        // qDebug() << node->getDesignation() << node->getX() << node->getY();
        //  set ellipse
        vtkEllipse *ellipse;
        ellipse =
                new vtkEllipse(node->getSemiMinorAxisLength() / 2,
                               node->getSemiMajorAxisLength() / 2, node->getAngle(), node->getX(),
                               node->getY(), node->getArcpix(), 0, 0, node->getDesignation(), NULL);
        // set vtk settings
        vtkSmartPointer<vtkCleanPolyData> cleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();
        cleanFilter->SetInputData(ellipse->getPolyData());
        cleanFilter->Update();
        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection(cleanFilter->GetOutputPort());
        ellipseActor = vtkSmartPointer<vtkLODActor>::New();
        ellipseActor->SetMapper(mapper);
        ellipseActor->GetProperty()->SetColor(0, 0, 0);
        ellipseActor->VisibilityOff();
        ellipseActorMap[node->getDesignation()] = ellipseActor;
        vtkwin->drawSingleEllipse(ellipseActor);
    }
}

/**
 * @brief SEDVisualizerPlot::drawNode Draw selectable sed nodes and their flux error
 * @param sedlist A list of sed objects to be visualized
 */
void SEDVisualizerPlot::drawNodes(QList<SEDNode *> sedlist)
{
    QVector<double> x, y, y_err;
    QSet<QString> visitedNodes;
    for (int i = 0; i < sedlist.count(); i++) {
        SEDNode *sedRootNode = sedlist.at(i);
        addCoordinatesData(sedRootNode, x, y, y_err, visitedNodes);
        if (!sedRootNode->getChild().isEmpty()) {
            addCoordinatesData(sedRootNode->getChild().values()[0], x, y, y_err, visitedNodes);
        }
    }
    // plot nodes
    graphSEDNodes = ui->customPlot->addGraph();
    ui->customPlot->graph()->setData(x, y);
    ui->customPlot->graph()->setLineStyle(QCPGraph::lsNone); // no edge on nodes

    // set error bar on node
    QCPErrorBars *errorBars = new QCPErrorBars(ui->customPlot->xAxis, ui->customPlot->yAxis);
    errorBars->removeFromLegend();
    errorBars->setAntialiased(true);
    errorBars->setData(y_err);
    errorBars->setDataPlottable(ui->customPlot->graph());
    errorBars->setPen(QPen(QColor(180, 180, 180)));
    errorBars->setSelectable(QCP::stNone);
    ui->customPlot->graph()->setPen(QPen(Qt::black));
    ui->customPlot->graph()->selectionDecorator()->setPen(QPen(Qt::red));
    ui->customPlot->graph()->setAntialiased(true);

    ui->customPlot->replot();
}

/**
 * Extract the wavelength, flux, and flux-error data from a given sed node.
 * The extracted data is appended to the provided QVector objects.
 * @brief SEDVisualizerPlot::addCoordinatesData
 * @param node SEDNode*. The method will visit this node and all its descendants.
 * @param x Reference QVector of double. Collect the wavelength data of each node.
 * @param y Reference QVector of double. Collect flux data of each node.
 * @param y_err Reference QVector of double. Collect flux-error data of each node.
 */
void SEDVisualizerPlot::addCoordinatesData(SEDNode *node, QVector<double> &x, QVector<double> &y,
                                           QVector<double> &y_err, QSet<QString> &visitedNodes)
{
    if (!visitedNodes.contains(node->getDesignation())) {
        visitedNodes.insert(node->getDesignation());
        x.append(node->getWavelength());
        y.append(node->getFlux());
        y_err.append(node->getErrFlux());
    }
}

void SEDVisualizerPlot::axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part)
{
    // Set an axis label by double clicking on it
    if (part == QCPAxis::spAxisLabel) // only react when the actual axis label is clicked, not tick
    {
        bool ok;
        QString newLabel =
                QInputDialog::getText(this, "QCustomPlot example",
                                      "New axis label:", QLineEdit::Normal, axis->label(), &ok);
        if (ok) {
            axis->setLabel(newLabel);
            ui->customPlot->replot();
        }
    }
}

/**
 *  Manage the visibility of ellipses actors of selected SEDNodes
 * @brief SEDVisualizerPlot::selectionChanged
 */
void SEDVisualizerPlot::selectionChanged()
{
    // node selection
    QCPDataSelection nodeSelection = graphSEDNodes->selection();
    // for each range of data selected on drag mode
    foreach (QCPDataRange dataRange, nodeSelection.dataRanges()) {
        for (int j = dataRange.begin(); j < dataRange.end(); ++j) {
            // get data-i (not element) selected
            QCPGraphDataContainer::const_iterator dataPoint = graphSEDNodes->data()->at(j);
            SEDPlotPointCustom *cp = qobject_cast<SEDPlotPointCustom *>(
                    sed_coordinte_to_element.value(qMakePair(dataPoint->key, dataPoint->value)));
            if (ellipseActorMap[cp->getNode()->getDesignation()]) {
                ellipseActorMap[cp->getNode()->getDesignation()]->VisibilityOn();
                vtkwin->updateScene();
            }
        }
    }
}

/*
void SEDVisualizerPlot::legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item)
{
    // Rename a graph by double clicking on its legend item
    Q_UNUSED(legend)
    if (item) // only react if item was clicked (user could have clicked on border padding of legend
              // where there is no item, then item is 0)
    {
        QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem *>(item);
        bool ok;
        QString newName = QInputDialog::getText(this, "QCustomPlot example",
                                                "New graph name:", QLineEdit::Normal,
                                                plItem->plottable()->name(), &ok);
        if (ok) {
            plItem->plottable()->setName(newName);
            ui->customPlot->replot();
        }
    }
}
*/

void SEDVisualizerPlot::mouseRelease(QMouseEvent *event)
{
    // disable mouse right click function
    if (event->button() == Qt::RightButton) {
        ui->customPlot->setSelectionRectMode(QCP::srmSelect);
    }
}

void SEDVisualizerPlot::mousePress(QMouseEvent *event)
{
    // disable mouse right click function
    if (event->button() == Qt::RightButton) {
        ui->customPlot->setSelectionRectMode(QCP::srmNone);
    }

    // Deselect all pending SED nodes: every drag selection is a new selection
    if (!multiSelectionPointStatus and !shiftMovingStatus and event->button() == Qt::LeftButton) {
        ui->customPlot->deselectAll();
        ui->customPlot->replot();
    }

    // set all ellipses visibility off
    for (auto ellipseActor = ellipseActorMap.constBegin();
         ellipseActor != ellipseActorMap.constEnd(); ++ellipseActor) {
        ellipseActor.value()->VisibilityOff();
    }
    vtkwin->updateScene();
}

void SEDVisualizerPlot::mouseWheel()
{

    // if an axis is selected, only allow the direction of that axis to be zoomed
    // if no axis is selected, both directions may be zoomed

    if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->xAxis->orientation());
    else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeZoom(ui->customPlot->yAxis->orientation());
    else
        ui->customPlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
}

void SEDVisualizerPlot::removeSelectedGraph()
{
    if (ui->customPlot->selectedGraphs().size() > 0) {
        ui->customPlot->removeGraph(ui->customPlot->selectedGraphs().first());
        ui->customPlot->replot();
    }
}

void SEDVisualizerPlot::removeAllGraphs()
{
    ui->customPlot->clearGraphs();
    ui->customPlot->replot();
}

/*
// TODO should be removed?: right click is disabled right now on mouse press
void SEDVisualizerPlot::contextMenuRequest(QPoint pos)
{

 QMenu *menu = new QMenu(this);
 menu->setAttribute(Qt::WA_DeleteOnClose);

 if (ui->customPlot->legend->selectTest(pos, false) >= 0) // context menu on legend requested
 {
     menu->addAction("Move to top left", this, SLOT(moveLegend()))
             ->setData((int)(Qt::AlignTop | Qt::AlignLeft));
     menu->addAction("Move to top center", this, SLOT(moveLegend()))
             ->setData((int)(Qt::AlignTop | Qt::AlignHCenter));
     menu->addAction("Move to top right", this, SLOT(moveLegend()))
             ->setData((int)(Qt::AlignTop | Qt::AlignRight));
     menu->addAction("Move to bottom right", this, SLOT(moveLegend()))
             ->setData((int)(Qt::AlignBottom | Qt::AlignRight));
     menu->addAction("Move to bottom left", this, SLOT(moveLegend()))
             ->setData((int)(Qt::AlignBottom | Qt::AlignLeft));
 }

 menu->popup(ui->customPlot->mapToGlobal(pos));
}
*/

SEDVisualizerPlot::~SEDVisualizerPlot()
{
    delete ui;
}

bool SEDVisualizerPlot::prepareSelectedInputForSedFit()
{
    bool validFit = false;
    QMap<double, SEDNode *> selected_sed_map;
    QCPDataSelection nodeSelection;
    QCPGraph *nodes;
    QMap<QPair<double, double>, QCPAbstractItem *> coordinate_to_element;

    // get selected sednodes or collapse (drag selection)
    if (!ui->collapseCheckBox->isChecked()) {
        nodeSelection = graphSEDNodes->selection();
        nodes = graphSEDNodes;
        coordinate_to_element = sed_coordinte_to_element;
    } else {
        nodeSelection = collapsedNodes->selection();
        nodes = collapsedNodes;
        coordinate_to_element = collapse_coordinate_to_element;
    }

    // for each range of data selected on drag mode
    foreach (QCPDataRange dataRange, nodeSelection.dataRanges()) {
        for (int j = dataRange.begin(); j < dataRange.end(); ++j) {
            // get data-i (not element) selected
            QCPGraphDataContainer::const_iterator dataPoint = nodes->data()->at(j);
            QString className =
                    coordinate_to_element.value(qMakePair(dataPoint->key, dataPoint->value))
                            ->metaObject()
                            ->className();
            if (QString::compare(className, "SEDPlotPointCustom") == 0) {
                SEDPlotPointCustom *cp = qobject_cast<SEDPlotPointCustom *>(
                        coordinate_to_element.value(qMakePair(dataPoint->key, dataPoint->value)));
                selected_sed_map.insert(cp->getNode()->getWavelength(), cp->getNode());
            }
        }
    }
    if (selected_sed_map.size() >= 2) {
        // reconstruct sedFitInputX from last to first element
        QMap<double, SEDNode *>::iterator iter;
        iter = selected_sed_map.begin();
        SEDNode *node = iter.value();
        sedFitInput.insert(node->getWavelength(), node->getFlux());
        sedFitInputF = QString::number(node->getFlux()) + sedFitInputF;
        sedFitInputErrF = QString::number(node->getErrFlux()) + sedFitInputErrF;
        sedFitInputW = QString::number(node->getWavelength()) + sedFitInputW;
        sedFitInputFflag = "1" + sedFitInputFflag;
        ++iter;
        for (; iter != selected_sed_map.end(); ++iter) {
            node = iter.value();
            sedFitInput.insert(node->getWavelength(), node->getFlux());
            sedFitInputF = QString::number(node->getFlux()) + "," + sedFitInputF;
            sedFitInputErrF = QString::number(node->getErrFlux()) + "," + sedFitInputErrF;
            sedFitInputW = QString::number(node->getWavelength()) + "," + sedFitInputW;
            sedFitInputFflag = "1," + sedFitInputFflag;
        }
        validFit = true;
    } else {
        QMessageBox::critical(NULL, QObject::tr("Error"),
                              QObject::tr("Could not execute SED fit with less than 2 points "
                                          "selected.\n\rPlease select more points and try again."));
        validFit = false;
    }

    return validFit;
}

void SEDVisualizerPlot::readColumnsFromSedFitResults(const QJsonArray &columns)
{
    // This function updates the internal Column->Index map
    for (int i = 0; i < columns.count(); ++i) {
        QString column = columns.at(i).toString().simplified();
        if (resultsOutputColumn.contains(column)) {
            resultsOutputColumn.insert(column, i);
        }
    }
}

void SEDVisualizerPlot::readSedFitOutput(QString filename)
{
    double chi2 = 99999999999;

    // Models Map chi2 -> Model
    // QMap<double, QJsonArray> models;
    models.clear();

    QVector<double> clump_mass;
    QVector<double> compact_mass_fraction;
    QVector<double> clump_upper_age;
    QVector<double> dust_temp;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
    QJsonArray columns = root.value("columns").toArray();
    QJsonArray data = root.value("data").toArray();

    readColumnsFromSedFitResults(columns);
    for (int i = 0; i < data.count(); ++i) {
        QJsonArray item = data.at(i).toArray();

        double chi2_i = item.at(resultsOutputColumn.value("chi2")).toDouble();
        double dist_i = item.at(resultsOutputColumn.value("d")).toDouble();
        models.insert(chi2_i, item);
        if (chi2_i < chi2) {
            chi2 = chi2_i;
            dist = dist_i;
        }

        double clump_mass_i = item.at(resultsOutputColumn.value("clump_mass")).toDouble();
        clump_mass.append(clump_mass_i);

        double compact_mass_fraction_i =
                item.at(resultsOutputColumn.value("compact_mass_fraction")).toDouble();
        compact_mass_fraction.append(compact_mass_fraction_i);

        double clump_upper_age_i = item.at(resultsOutputColumn.value("clump_upper_age")).toDouble();
        clump_upper_age.append(clump_upper_age_i);

        double dust_temp_i = item.at(resultsOutputColumn.value("dust_temp")).toDouble();
        dust_temp.append(dust_temp_i);
    }

    sedFitValues.insert(2, clump_mass);
    sedFitValues.insert(3, compact_mass_fraction);
    sedFitValues.insert(4, clump_upper_age);
    sedFitValues.insert(5, dust_temp);

    int threshold = 5;
    int nFitsResults = models.size() * threshold / 100;

    if (nFitsResults < 1 && models.size() >= 1) {
        nFitsResults = 1;
    }

    if (nFitsResults <= 10) {
        nFitsResults = models.size();
    }

    ui->resultsTableWidget->clearContents();
    ui->resultsTableWidget->setRowCount(0);
    ui->resultsTableWidget->setColumnCount(columnNames.size());
    ui->resultsTableWidget->setHorizontalHeaderLabels(columnNames);

    auto it = models.cbegin();
    for (int r = 0; r < nFitsResults; ++r, ++it) {
        ui->resultsTableWidget->insertRow(r);

        auto item = it.value();
        for (int c = 0; c <= columnNames.indexOf("d"); ++c) {
            int colIdx = resultsOutputColumn.value(columnNames.at(c));
            if (colIdx != -1) {
                auto field_v = item.at(colIdx);
                QString field_s = field_v.isDouble() ? QString::number(field_v.toDouble())
                                                     : field_v.toString();
                ui->resultsTableWidget->setItem(r, c, new QTableWidgetItem(field_s));
            }
        }

        QJsonArray fmod = item.at(resultsOutputColumn.value("fmod")).toArray();
        int fmod_idx = 0;
        for (int c = columnNames.indexOf("wise1"); c <= columnNames.indexOf("bol11");
             ++c, ++fmod_idx) {
            ui->resultsTableWidget->setItem(
                    r, c, new QTableWidgetItem(QString::number(fmod.at(fmod_idx).toDouble())));
        }
    }

    ui->outputTextBrowser->setText("Done.");
    ui->resultsTableWidget->resizeColumnsToContents();
    ui->resultsTableWidget->resizeRowsToContents();

    // Plot the best one
    plotSedFitModel(models.first(), Qt::blue);

    loading->close();
    ui->resultsRadioButton->setChecked(true);
}

void SEDVisualizerPlot::plotSedFitModel(const QJsonArray &model, Qt::GlobalColor color)
{
    QJsonArray wmod = model.at(resultsOutputColumn.value("wmod")).toArray();
    QJsonArray fmod = model.at(resultsOutputColumn.value("fmod")).toArray();

    QVector<double> vec_x, vec_y;
    QList<SEDPlotPointCustom *> points;
    for (int i = 0; i < wmod.count(); ++i) {
        double x = wmod.at(i).toDouble();
        vec_x.append(x);

        double y = fmod.at(i).toDouble();
        vec_y.append(y);

        SEDPlotPointCustom *cp = new SEDPlotPointCustom(ui->customPlot, 3, vtkwin);
        cp->setAntialiased(true);
        cp->setPos(x, y);

        cp->setDesignation("");
        cp->setX(0);
        cp->setY(0);
        cp->setLat(0);
        cp->setLon(0);
        points.push_back(cp);
    }

    QCPGraph *graph = ui->customPlot->addGraph();
    if (color == Qt::blue || doubleClicked) {
        addNewSEDCheckBox("Theoretical Fit");
        sedGraphPoints.insert(nSED - 1, points);
        sedGraphs.push_back(graph);
        doubleClicked = false;
    }

    if (temporaryMOD) {
        temporaryGraph = graph;
        temporaryGraphPoints = points;
    }

    ui->customPlot->graph()->setData(vec_x, vec_y);
    ui->customPlot->graph()->setPen(QPen(color));
    ui->customPlot->graph()->setScatterStyle(QCPScatterStyle::ssNone);
    ui->customPlot->graph()->setSelectable(QCP::stNone); // disable graph selection
    ui->customPlot->replot();

    this->setFocus();
    this->activateWindow();
}

void SEDVisualizerPlot::loadSedFitOutput(QString filename)
{
    double chi2 = 99999999999;

    // Models Map chi2 -> Model
    models.clear();

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
    QJsonArray columns = root.value("columns").toArray();
    QJsonArray data = root.value("data").toArray();

    readColumnsFromSedFitResults(columns);
    for (int i = 0; i < data.count(); ++i) {
        QJsonArray item = data.at(i).toArray();

        double chi2_i = item.at(resultsOutputColumn.value("chi2")).toDouble();
        double dist_i = item.at(resultsOutputColumn.value("d")).toDouble();
        models.insert(chi2_i, item);
        if (chi2_i < chi2) {
            chi2 = chi2_i;
            dist = dist_i;
        }
    }

    int threshold = 5;
    int nFitsResults = models.size() * threshold / 100;

    if (nFitsResults < 1 && models.size() >= 1) {
        nFitsResults = 1;
    }

    if (nFitsResults <= 10) {
        nFitsResults = models.size();
    }

    ui->resultsTableWidget->clearContents();
    ui->resultsTableWidget->setRowCount(0);
    ui->resultsTableWidget->setColumnCount(columnNames.size());
    ui->resultsTableWidget->setHorizontalHeaderLabels(columnNames);

    auto it = models.cbegin();
    for (int r = 0; r < nFitsResults; ++r, ++it) {
        ui->resultsTableWidget->insertRow(r);

        auto item = it.value();
        for (int c = 0; c <= columnNames.indexOf("d"); ++c) {
            int colIdx = resultsOutputColumn.value(columnNames.at(c));
            if (colIdx != -1) {
                auto field_v = item.at(colIdx);
                QString field_s = field_v.isDouble() ? QString::number(field_v.toDouble())
                                                     : field_v.toString();
                ui->resultsTableWidget->setItem(r, c, new QTableWidgetItem(field_s));
            }
        }

        QJsonArray fmod = item.at(resultsOutputColumn.value("fmod")).toArray();
        int fmod_idx = 0;
        for (int c = columnNames.indexOf("wise1"); c <= columnNames.indexOf("bol11");
             ++c, ++fmod_idx) {
            ui->resultsTableWidget->setItem(
                    r, c, new QTableWidgetItem(QString::number(fmod.at(fmod_idx).toDouble())));
        }
    }

    ui->resultsTableWidget->resizeColumnsToContents();
    ui->resultsTableWidget->resizeRowsToContents();
}

void SEDVisualizerPlot::loadSedFitThin(QString filename)
{

    ui->resultsTableWidget->clearContents();
    ui->resultsTableWidget->setColumnCount(6);
    ui->resultsTableWidget->setRowCount(0);
    ui->resultsTableWidget->setHorizontalHeaderLabels(QStringList() << "fmass"
                                                                    << "dmass"
                                                                    << "ftemp"
                                                                    << "dtemp"
                                                                    << "fbeta"
                                                                    << "lum");
    ui->resultsTableWidget->insertRow(0);
    QFile filepar(filename);
    if (!filepar.open(QIODevice::ReadOnly)) {
        return;
    }
    QString line = filepar.readLine();
    outputSedLog.append(line);
    filepar.close();
    QList<QString> line_list_string = line.split(',');
    for (int i = 0; i < line_list_string.size(); i++) {
        ui->resultsTableWidget->setItem(0, i,
                                        new QTableWidgetItem(line_list_string.at(i).trimmed()));
    }
    ui->resultsTableWidget->resizeColumnsToContents();
}

void SEDVisualizerPlot::loadSedFitThick(QString filename)
{
    ui->resultsTableWidget->clearContents();
    ui->resultsTableWidget->setColumnCount(9);
    ui->resultsTableWidget->setRowCount(0);
    ui->resultsTableWidget->setHorizontalHeaderLabels(QStringList() << "mass"
                                                                    << "dmass"
                                                                    << "ftemp"
                                                                    << "dtemp"
                                                                    << "fbeta"
                                                                    << "fl0"
                                                                    << "dlam0"
                                                                    << "sizesec"
                                                                    << "lum");
    ui->resultsTableWidget->insertRow(0);
    QFile filepar(filename);
    if (!filepar.open(QIODevice::ReadOnly)) {
        return;
    }
    QString line = filepar.readLine();
    outputSedLog.append(line);
    filepar.close();
    QList<QString> line_list_string = line.split(',');
    for (int i = 0; i < line_list_string.size(); i++) {
        ui->resultsTableWidget->setItem(0, i,
                                        new QTableWidgetItem(line_list_string.at(i).trimmed()));
    }
    ui->resultsTableWidget->resizeColumnsToContents();
}

void SEDVisualizerPlot::setModelFitValue(QVector<QStringList> headerAndValueList,
                                         Qt::GlobalColor color)
{
    QFile modelFile(QDir::homePath()
                            .append(QDir::separator())
                            .append("VisIVODesktopTemp/tmp_download/SED" + QString::number(nSED)
                                    + "_sedfit_model.dat"));
    if (!modelFile.exists()) {
        modelFile.open(QIODevice::WriteOnly);
        QDataStream out(&modelFile);
        out << headerAndValueList; // serialize the object
    }
    QVector<double> x, y;
    double scale;
    QCPGraph *graph = ui->customPlot->addGraph();
    QList<SEDPlotPointCustom *> points;
    for (QMap<QString, double>::iterator it = modelFitBands.begin(); it != modelFitBands.end();
         ++it) {
        scale = 1000 / pow(dist, 2);
        x.append(it.value());
        int index = headerAndValueList.at(0).indexOf(it.key());
        y.append(headerAndValueList.at(1).at(index).toDouble() * scale);
        SEDPlotPointCustom *cp = new SEDPlotPointCustom(ui->customPlot, 3, vtkwin);
        cp->setAntialiased(true);
        cp->setPos(it.value(), headerAndValueList.at(1).at(index).toDouble() * scale);

        cp->setDesignation("");
        cp->setX(0);
        cp->setY(0);
        cp->setLat(0);
        cp->setLon(0);
        points.push_back(cp);
    }

    if (color == Qt::blue || doubleClicked) {
        addNewSEDCheckBox("Theoretical Fit");
        sedGraphPoints.insert(nSED - 1, points);
        sedGraphs.push_back(graph);
        doubleClicked = false;
    }

    if (temporaryMOD) {
        temporaryGraph = graph;
        temporaryGraphPoints = points;
    }

    ui->customPlot->graph()->setData(x, y);
    ui->customPlot->graph()->setPen(QPen(color));
    ui->customPlot->graph()->setScatterStyle(QCPScatterStyle::ssNone);
    ui->customPlot->replot();
    this->setFocus();
    this->activateWindow();
}

void SEDVisualizerPlot::on_actionScreenshot_triggered()
{
    QPixmap qPixMap = QPixmap::grabWidget(
            this); // *this* is window pointer, the snippet     is in the mainwindow.cpp file

    QImage qImage = qPixMap.toImage();

    QString fileName = QFileDialog::getSaveFileName(this, "Save screenshot", "", ".png");
    if (!fileName.endsWith(".png", Qt::CaseInsensitive))
        fileName.append(".png");
    bool b = qImage.save(fileName);
}

void SEDVisualizerPlot::createCollapseNodes()
{

    SED *coll_sed = new SED();
    QVector<double> x, y;
    SEDNode *old_tmp_node;
    SEDNode *tmp_node;

    int cnt = 0;
    double j;
    foreach (j, all_sed_node.uniqueKeys()) {
        QList<SEDNode *> node_list_tmp = all_sed_node.values(j);
        double flux_sum = 0;
        double err_flux_sum = 0;
        if (cnt != 0) {
            old_tmp_node = tmp_node;
        }
        tmp_node = new SEDNode();
        for (int z = 0; z < node_list_tmp.count(); z++) {
            flux_sum += node_list_tmp.at(z)->getFlux();
            err_flux_sum += node_list_tmp.at(z)->getErrFlux();
        }

        tmp_node->setDesignation("");
        tmp_node->setFlux(flux_sum);
        tmp_node->setErrFlux(err_flux_sum);
        tmp_node->setWavelength(j);

        if (cnt != 0) {
            old_tmp_node->setParent(tmp_node);
            tmp_node->setChild(old_tmp_node);
        }
        x.append(j);
        y.append(flux_sum);

        SEDPlotPointCustom *cp = new SEDPlotPointCustom(ui->customPlot, 3, vtkwin);
        cp->setAntialiased(true);
        cp->setPos(j, flux_sum);
        cp->setDesignation("");
        cp->setX(0);
        cp->setY(0);
        cp->setLat(0);
        cp->setLon(0);
        cp->setErrorFlux(err_flux_sum);
        cp->setNode(tmp_node);
        cp->setVisible(false);
        collapsedGraphPoints.push_back(cp);
        collapse_coordinate_to_element.insert(qMakePair(j, flux_sum), cp);
        cnt++;
    }
    coll_sed->setRootNode(tmp_node);

    // generate line collapse-graph
    collapsedGraph = ui->customPlot->addGraph();
    collapsedGraph->setData(x, y);
    collapsedGraph->setPen(QPen(Qt::red)); // line color red for second graph
    collapsedGraph->setScatterStyle(QCPScatterStyle::ssNone); // no nodes
    collapsedGraph->setSelectable(QCP::stNone); // no line selectable
    collapsedGraph->setVisible(false);

    // generate nodes collapse-graph
    collapsedNodes = ui->customPlot->addGraph();
    collapsedNodes->setData(x, y);
    collapsedNodes->setLineStyle(QCPGraph::lsNone);
    QCPScatterStyle scatterStyless =
            createScatterStyle(QCPScatterStyle::ssCircle, 6, Qt::transparent, 2.0);
    QCPScatterStyle scatterStylenn = createScatterStyle(QCPScatterStyle::ssCircle, 6, Qt::red, 2.0);
    QCPSelectionDecorator *decorator = new QCPSelectionDecorator();
    decorator->setScatterStyle(scatterStylenn);
    collapsedNodes->setSelectionDecorator(decorator);
    collapsedNodes->setScatterStyle(scatterStyless);
    collapsedNodes->setSelectable(QCP::stMultipleDataRanges);
    collapsedNodes->setVisible(false);
    ui->customPlot->replot();
    // sed_list.insert(0, coll_sed); // to save/export it
}

void SEDVisualizerPlot::finishedTheoreticalRemoteFit()
{
    QDir dir(QApplication::applicationDirPath());

    dir.rename("sedfit_output.dat",
               QDir::homePath()
                       .append(QDir::separator())
                       .append("VisIVODesktopTemp/tmp_download/SED" + QString::number(nSED)
                               + "_sedfit_output.dat"));
    readSedFitOutput(QDir::homePath()
                             .append(QDir::separator())
                             .append("VisIVODesktopTemp/tmp_download/SED" + QString::number(nSED)
                                     + "_sedfit_output.dat"));

    this->setFocus();
    this->activateWindow();
}

void SEDVisualizerPlot::onReadyReadStdOutput()
{
    QTextStream stream(process);
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        // extract progress info from line and etc.
        ui->outputTextBrowser->append(line);
    }
}

// Add a new sed checkbox to the group of SED
void SEDVisualizerPlot::addNewSEDCheckBox(QString label, int nSED)
{
    QSignalMapper *mapper = new QSignalMapper(this);
    ui->generatedSedBox->setVisible(true);
    QCheckBox *newSed = new QCheckBox(label + " " + QString::number(nSED + 1));
    newSed->setChecked(true);
    connect(newSed, SIGNAL(stateChanged(int)), mapper, SLOT(map()));
    mapper->setMapping(newSed, nSED);
    connect(mapper, SIGNAL(mapped(int)), this, SLOT(on_SEDCheckboxClicked(int)));
    ui->generatedSedBox->layout()->addWidget(newSed);
}

// Add a new sed checkbox to the group of SED
void SEDVisualizerPlot::addNewSEDCheckBox(QString label)
{
    QSignalMapper *mapper = new QSignalMapper(this);
    ui->generatedSedBox->setVisible(true);
    QCheckBox *newSed = new QCheckBox(QString::number(nSED + 1) + ":" + label);
    newSed->setChecked(true);
    connect(newSed, SIGNAL(stateChanged(int)), mapper, SLOT(map()));
    mapper->setMapping(newSed, nSED);
    connect(mapper, SIGNAL(mapped(int)), this, SLOT(on_SEDCheckboxClicked(int)));
    ui->generatedSedBox->layout()->addWidget(newSed);
    nSED++;
    plottedSedLabels.append(label);
}

void SEDVisualizerPlot::on_SEDCheckboxClicked(int sedN)
{
    QCPGraph *graph = sedGraphs.at(sedN);
    if (graph->visible()) {
        graph->setVisible(false);
        if (sedGraphPoints.contains(sedN)) {
            QList<SEDPlotPointCustom *> points = sedGraphPoints.value(sedN);
            for (int i = 0; i < points.size(); ++i) {
                SEDPlotPointCustom *cp;
                cp = points.at(i);
                cp->setVisible(false);
            }
        }
        ui->resultsTableWidget->clearContents();
        ui->resultsTableWidget->setRowCount(0);
    } else {
        graph->setVisible(true);
        if (sedGraphPoints.contains(sedN)) { // This is a theoretical SED fit
            QList<SEDPlotPointCustom *> points = sedGraphPoints.value(sedN);
            for (int i = 0; i < points.size(); ++i) {
                SEDPlotPointCustom *cp;
                cp = points.at(i);
                cp->setVisible(true);
            }
            loadSedFitOutput(QDir::homePath()
                                     .append(QDir::separator())
                                     .append("VisIVODesktopTemp/tmp_download/SED"
                                             + QString::number(sedN) + "_sedfit_output.dat"));
        } else { // This is a Thin or Thick SED fit
            QString label = plottedSedLabels.at(sedN);
            if (label.contains("Thick")) {
                loadSedFitThick(QDir::homePath()
                                        .append(QDir::separator())
                                        .append("VisIVODesktopTemp/tmp_download/SED"
                                                + QString::number(sedN) + "_thickfile.csv.par"));
            } else {
                loadSedFitThin(QDir::homePath()
                                       .append(QDir::separator())
                                       .append("VisIVODesktopTemp/tmp_download/SED"
                                               + QString::number(sedN) + "_thinfile.csv.par"));
            }
        }
    }
    ui->customPlot->replot();
}

void SEDVisualizerPlot::on_ThinLocalFit_triggered()
{
    sedFitInputF = "]";
    sedFitInputErrF = "]";
    sedFitInputW = "]";

    sedFitInput.clear();
    bool validFit = false;

    validFit = prepareSelectedInputForSedFit();

    if (validFit) {
        sedFitInputF = "[" + sedFitInputF;
        sedFitInputErrF = "[" + sedFitInputErrF;
        sedFitInputW = "[" + sedFitInputW;

        QString f = sedFitInputW.mid(1, sedFitInputW.length() - 2);
        QStringList wave = f.split(",");
        QSignalMapper *mapper = new QSignalMapper(this);

        sd_thin = new SedFitGrid_thin(this);
        sd_thin->ui->distLineEdit->setText(ui->distTheoLineEdit->text());
        sedFitInputUlimitString = "[";

        for (int i = 0; i < wave.length(); i++) {
            sedFitInputUlimit.insert(i, "0");
            sedFitInputUlimitString += "0";
            if (i < wave.size() - 1)
                sedFitInputUlimitString += ",";

            int row = sd_thin->ui->tableWidget->model()->rowCount();
            sd_thin->ui->tableWidget->insertRow(row);

            QCheckBox *cb1 = new QCheckBox();
            cb1->setChecked(false);
            sd_thin->ui->tableWidget->setCellWidget(row, 1, cb1);

            connect(cb1, SIGNAL(stateChanged(int)), mapper, SLOT(map()));
            mapper->setMapping(cb1, row);

            QTableWidgetItem *item_1 = new QTableWidgetItem();
            item_1->setText(wave.at(i));

            sd_thin->ui->tableWidget->setItem(row, 0, item_1);
        }
        sedFitInputUlimitString += "]";
        connect(mapper, SIGNAL(mapped(int)), this, SLOT(checkboxClicked(int)));

        sd_thin->show();

        connect(sd_thin->ui->pushButton, SIGNAL(clicked()), this, SLOT(doThinLocalFit()));
    }
}

void SEDVisualizerPlot::checkboxClicked(int cb)
{
    sedFitInputUlimit[cb].compare("0") == 0 ? sedFitInputUlimit[cb] = "1"
                                            : sedFitInputUlimit[cb] = "0";
    sedFitInputUlimitString = "[";

    for (int i = 0; i < sedFitInputUlimit.size(); i++) {
        sedFitInputUlimitString += sedFitInputUlimit[i];
        if (i < sedFitInputUlimit.size() - 1)
            sedFitInputUlimitString += ",";
    }
    sedFitInputUlimitString += "]";
}

void SEDVisualizerPlot::doThinLocalFit()
{
    sd_thin->close();
    ui->outputTextBrowser->setText("");

    process = new QProcess(this);
    process->setProcessChannelMode(QProcess::MergedChannels);
    connect(process, SIGNAL(readyReadStandardError()), this, SLOT(onReadyReadStdOutput()));
    connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(onReadyReadStdOutput()));
    connect(process, SIGNAL(finished(int)), this, SLOT(finishedThinLocalFit()));

    QString mrange = "[" + sd_thin->ui->minMassLineEdit->text() + ", "
            + sd_thin->ui->maxMassLineEdit->text() + ", " + sd_thin->ui->stepMassLineEdit->text()
            + "]";
    QString trange = "[" + sd_thin->ui->tempMinLineEdit->text() + ", "
            + sd_thin->ui->tempMaxLineEdit->text() + ", " + sd_thin->ui->tempStepLineEdit->text()
            + "]";
    QString brange = "[" + sd_thin->ui->betaMinLineEdit->text() + ", "
            + sd_thin->ui->betaMaxLineEdit->text() + ", " + sd_thin->ui->betaStepLineEdit->text()
            + "]";
    QString srefRange = "[" + sd_thin->ui->srefOpacityLineEdit->text() + ", "
            + sd_thin->ui->srefWavelengthLineEdit->text() + "]";

    QString outputFile = QDir::homePath()
                                 .append(QDir::separator())
                                 .append("VisIVODesktopTemp/tmp_download/SED"
                                         + QString::number(nSED) + "_thinfile.csv");

    QDir dir(QApplication::applicationDirPath());

    QStringList args;
    args << dir.absoluteFilePath("sedfit_main.py") << "thin" << sedFitInputW << sedFitInputF
         << mrange << trange << brange << sd_thin->ui->distLineEdit->text() << srefRange
         << sedFitInputErrF << sedFitInputUlimitString << outputFile;

    process->setWorkingDirectory(dir.absolutePath());
    process->start(this->pythonExe, args);
}

void SEDVisualizerPlot::doThinRemoteFit()
{
    sd_thin->close();
    ui->outputTextBrowser->setText("");

    process = new QProcess();
    connect(process, SIGNAL(readyReadStandardError()), this, SLOT(onReadyReadStdOutput()));
    connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(onReadyReadStdOutput()));
    connect(process, SIGNAL(finished(int)), this, SLOT(finishedThinRemoteFit()));

    process->setProcessChannelMode(QProcess::MergedChannels);

    QString mrange = "[" + sd_thin->ui->minMassLineEdit->text() + ", "
            + sd_thin->ui->maxMassLineEdit->text() + ", " + sd_thin->ui->stepMassLineEdit->text()
            + "]";
    QString trange = "[" + sd_thin->ui->tempMinLineEdit->text() + ", "
            + sd_thin->ui->tempMaxLineEdit->text() + ", " + sd_thin->ui->tempStepLineEdit->text()
            + "]";
    QString brange = "[" + sd_thin->ui->betaMinLineEdit->text() + ", "
            + sd_thin->ui->betaMaxLineEdit->text() + ", " + sd_thin->ui->betaStepLineEdit->text()
            + "]";
    QString srefRange = "[" + sd_thin->ui->srefOpacityLineEdit->text() + ", "
            + sd_thin->ui->srefWavelengthLineEdit->text() + "]";

    process->setWorkingDirectory(QApplication::applicationDirPath());
    process->start("java -jar " + QApplication::applicationDirPath().append("/vsh-ws-client.jar")
                   + " \"sedfitgrid_engine_thin_vialactea," + sedFitInputW + "," + sedFitInputF
                   + "," + mrange + "," + trange + "," + brange + ","
                   + sd_thin->ui->distLineEdit->text() + "," + srefRange
                   + ",lambdatn,flussotn,mtn,ttn,btn,ltn,dmtn,dttn,errorbars=" + sedFitInputErrF
                   + ",ulimit=" + sedFitInputUlimitString + ",printfile= 'sedfit_output.dat'\" ");

    ui->outputTextBrowser->append(
            "java -jar " + QApplication::applicationDirPath().append("/vsh-ws-client.jar")
            + " \"sedfitgrid_engine_thin_vialactea," + sedFitInputW + "," + sedFitInputF + ","
            + mrange + "," + trange + "," + brange + "," + sd_thin->ui->distLineEdit->text() + ","
            + srefRange
            + ",lambdatn,flussotn,mtn,ttn,btn,ltn,dmtn,dttn,errorbars=" + sedFitInputErrF
            + ",ulimit=" + sedFitInputUlimitString + ",printfile= 'sedfit_output.dat'\" ");
    ui->outputTextBrowser->append("\nDIR: " + process->workingDirectory());
    ui->outputTextBrowser->append("\nApp path : " + qApp->applicationDirPath());
    ui->outputTextBrowser->append("\nQDir::currentPath() :" + QDir::currentPath());
}

void SEDVisualizerPlot::doThickRemoteFit()
{
    sd_thick->close();
    ui->outputTextBrowser->setText("");

    process = new QProcess();
    connect(process, SIGNAL(readyReadStandardError()), this, SLOT(onReadyReadStdOutput()));
    connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(onReadyReadStdOutput()));
    connect(process, SIGNAL(finished(int)), this, SLOT(finishedThickRemoteFit()));

    process->setProcessChannelMode(QProcess::MergedChannels);
    QString trange = "[" + sd_thick->ui->tempMinLineEdit->text() + ", "
            + sd_thick->ui->tempMaxLineEdit->text() + ", " + sd_thick->ui->tempStepLineEdit->text()
            + "]";
    QString brange = "[" + sd_thick->ui->betaMinLineEdit->text() + ", "
            + sd_thick->ui->betaMaxLineEdit->text() + ", " + sd_thick->ui->betaStepLineEdit->text()
            + "]";
    QString l0range = "[" + sd_thick->ui->minLambda_0LineEdit->text() + ", "
            + sd_thick->ui->maxLambda_0LineEdit->text() + ", "
            + sd_thick->ui->stepLambda_0LineEdit->text() + "]";
    QString sfactrange = "[" + sd_thick->ui->scaleMinLineEdit->text() + ", "
            + sd_thick->ui->scaleMaxLineEdit->text() + ", "
            + sd_thick->ui->scaleStepLineEdit->text() + "]";
    QString srefRange = "[" + sd_thick->ui->srefOpacityLineEdit->text() + ", "
            + sd_thick->ui->srefWavelengthLineEdit->text() + "]";

    process->setWorkingDirectory(QApplication::applicationDirPath());
    process->start("java -jar " + QApplication::applicationDirPath().append("/vsh-ws-client.jar")
                   + " \"sedfitgrid_engine_thick_vialactea," + sedFitInputW + "," + sedFitInputF
                   + "," + sd_thick->ui->sizeLineEdit->text() + "," + trange + " ," + brange + " ,"
                   + l0range + ", " + sfactrange + "," + sd_thick->ui->distLineEdit->text() + ","
                   + srefRange
                   + ",lambdagb,flussogb,mtk,ttk,btk,l0,sizesec,ltk,dmtk,dtk,dl0,errorbars="
                   + sedFitInputErrF + ",ulimit=" + sedFitInputUlimitString
                   + ",printfile= 'sedfit_output.dat'\" ");
}

void SEDVisualizerPlot::finishedThinLocalFit()
{
    bool res = readThinFit(QDir::homePath()
                                   .append(QDir::separator())
                                   .append("VisIVODesktopTemp/tmp_download/SED"
                                           + QString::number(nSED) + "_thinfile.csv"));
    if (res)
        addNewSEDCheckBox("Thin Fit");
    else
        QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("No results."));
}

bool SEDVisualizerPlot::readThinFit(QString resultPath)
{
    QFile file(resultPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QVector<double> x, y;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();

        x.append(line.split(',').first().simplified().toDouble());
        y.append(line.split(',').last().simplified().toDouble());
    }

    ui->customPlot->addGraph();
    ui->customPlot->graph()->setData(x, y);
    ui->customPlot->graph()->setScatterStyle(QCPScatterStyle::ssNone);
    ui->customPlot->graph()->setSelectable(QCP::stNone);
    sedGraphs.push_back(ui->customPlot->graph());
    ui->customPlot->replot();

    ui->resultsTableWidget->clearContents();
    ui->resultsTableWidget->setColumnCount(6);
    ui->resultsTableWidget->setRowCount(0);
    ui->resultsTableWidget->setHorizontalHeaderLabels(QStringList() << "fmass"
                                                                    << "dmass"
                                                                    << "ftemp"
                                                                    << "dtemp"
                                                                    << "fbeta"
                                                                    << "lum");
    ui->resultsTableWidget->insertRow(0);

    QFile filepar(resultPath + ".par");
    if (!filepar.open(QIODevice::ReadOnly)) {
        return false;
    }
    QString line = filepar.readLine();
    filepar.close();
    QList<QString> line_list_string = line.split(',');
    for (int i = 0; i < line_list_string.size(); i++) {
        ui->resultsTableWidget->setItem(0, i,
                                        new QTableWidgetItem(line_list_string.at(i).trimmed()));
    }
    ui->resultsTableWidget->resizeColumnsToContents();
    return true;
}

bool SEDVisualizerPlot::readThickFit(QString resultPath)
{
    QFile file(resultPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QVector<double> x, y;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        x.append(line.split(',').first().simplified().toDouble());
        y.append(line.split(',').last().simplified().toDouble());
    }

    ui->customPlot->addGraph();
    ui->customPlot->graph()->setData(x, y);
    ui->customPlot->graph()->setScatterStyle(QCPScatterStyle::ssNone);
    ui->customPlot->graph()->setSelectable(QCP::stNone);
    sedGraphs.push_back(ui->customPlot->graph());
    ui->customPlot->replot();

    ui->resultsTableWidget->clearContents();
    ui->resultsTableWidget->setColumnCount(9);
    ui->resultsTableWidget->setRowCount(0);
    ui->resultsTableWidget->setHorizontalHeaderLabels(QStringList() << "mass"
                                                                    << "dmass"
                                                                    << "ftemp"
                                                                    << "dtemp"
                                                                    << "fbeta"
                                                                    << "fl0"
                                                                    << "dlam0"
                                                                    << "sizesec"
                                                                    << "lum");
    ui->resultsTableWidget->insertRow(0);
    QFile filepar(resultPath + ".par");
    if (!filepar.open(QIODevice::ReadOnly)) {
        return false;
    }
    QString line = filepar.readLine();
    filepar.close();
    QList<QString> line_list_string = line.split(',');
    for (int i = 0; i < line_list_string.size(); i++) {
        ui->resultsTableWidget->setItem(0, i,
                                        new QTableWidgetItem(line_list_string.at(i).trimmed()));
    }
    ui->resultsTableWidget->resizeColumnsToContents();

    return true;
}

void SEDVisualizerPlot::doThickLocalFit()
{
    sd_thick->close();
    ui->outputTextBrowser->setText("");

    process = new QProcess(this);
    process->setProcessChannelMode(QProcess::MergedChannels);
    connect(process, SIGNAL(readyReadStandardError()), this, SLOT(onReadyReadStdOutput()));
    connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(onReadyReadStdOutput()));
    connect(process, SIGNAL(finished(int)), this, SLOT(finishedThickLocalFit()));

    QString trange = "[" + sd_thick->ui->tempMinLineEdit->text() + ", "
            + sd_thick->ui->tempMaxLineEdit->text() + ", " + sd_thick->ui->tempStepLineEdit->text()
            + "]";
    QString brange = "[" + sd_thick->ui->betaMinLineEdit->text() + ", "
            + sd_thick->ui->betaMaxLineEdit->text() + ", " + sd_thick->ui->betaStepLineEdit->text()
            + "]";
    QString l0range = "[" + sd_thick->ui->minLambda_0LineEdit->text() + ", "
            + sd_thick->ui->maxLambda_0LineEdit->text() + ", "
            + sd_thick->ui->stepLambda_0LineEdit->text() + "]";
    QString sfactrange = "[" + sd_thick->ui->scaleMinLineEdit->text() + ", "
            + sd_thick->ui->scaleMaxLineEdit->text() + ", "
            + sd_thick->ui->scaleStepLineEdit->text() + "]";
    QString srefRange = "[" + sd_thick->ui->srefOpacityLineEdit->text() + ", "
            + sd_thick->ui->srefWavelengthLineEdit->text() + "]";

    QString outputFile = QDir::homePath()
                                 .append(QDir::separator())
                                 .append("VisIVODesktopTemp/tmp_download/SED"
                                         + QString::number(nSED) + "_thickfile.csv");

    QDir dir(QApplication::applicationDirPath());

    QStringList args;
    args << dir.absoluteFilePath("sedfit_main.py") << "thick" << sedFitInputW << sedFitInputF
         << sd_thick->ui->sizeLineEdit->text() << trange << brange << l0range << sfactrange
         << sd_thick->ui->distLineEdit->text() << srefRange << sedFitInputErrF
         << sedFitInputUlimitString << outputFile;

    process->setWorkingDirectory(dir.absolutePath());
    process->start(this->pythonExe, args);
}

void SEDVisualizerPlot::finishedThinRemoteFit()
{

    QDir dir(QApplication::applicationDirPath());

    dir.rename("sedfit_output.dat",
               QDir::homePath()
                       .append(QDir::separator())
                       .append("VisIVODesktopTemp/tmp_download/SED" + QString::number(nSED)
                               + "_thinfile.csv"));
    dir.rename("sedfit_output.dat.par",
               QDir::homePath()
                       .append(QDir::separator())
                       .append("VisIVODesktopTemp/tmp_download/SED" + QString::number(nSED)
                               + "_thinfile.csv.par"));
    bool res = readThickFit(QDir::homePath()
                                    .append(QDir::separator())
                                    .append("VisIVODesktopTemp/tmp_download/SED"
                                            + QString::number(nSED) + "_thinfile.csv"));
    if (res)
        addNewSEDCheckBox("Thin Fit");
    else
        QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("No results."));
}

void SEDVisualizerPlot::finishedThickLocalFit()
{

    bool res = readThickFit(QDir::homePath()
                                    .append(QDir::separator())
                                    .append("VisIVODesktopTemp/tmp_download/SED"
                                            + QString::number(nSED) + "_thickfile.csv"));
    if (res)
        addNewSEDCheckBox("Thick Fit");
    else
        QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("No results."));
}

void SEDVisualizerPlot::on_ThickLocalFit_triggered()
{
    sedFitInputF = "]";
    sedFitInputErrF = "]";
    sedFitInputW = "]";
    sedFitInput.clear();
    bool validFit = false;
    validFit = prepareSelectedInputForSedFit();

    if (validFit) {

        sedFitInputF = "[" + sedFitInputF;
        sedFitInputErrF = "[" + sedFitInputErrF;
        sedFitInputW = "[" + sedFitInputW;

        QString f = sedFitInputW.mid(1, sedFitInputW.length() - 2);
        QStringList wave = f.split(",");
        QSignalMapper *mapper = new QSignalMapper(this);

        sd_thick = new SedFitgrid_thick(this);
        sd_thick->ui->distLineEdit->setText(ui->distTheoLineEdit->text());

        sedFitInputUlimitString = "[";

        for (int i = 0; i < wave.length(); i++) {
            sedFitInputUlimit.insert(i, "0");
            sedFitInputUlimitString += "0";
            if (i < wave.size() - 1)
                sedFitInputUlimitString += ",";

            int row = sd_thick->ui->tableWidget->model()->rowCount();
            sd_thick->ui->tableWidget->insertRow(row);

            QCheckBox *cb1 = new QCheckBox();
            cb1->setChecked(false);
            sd_thick->ui->tableWidget->setCellWidget(row, 1, cb1);

            connect(cb1, SIGNAL(stateChanged(int)), mapper, SLOT(map()));
            mapper->setMapping(cb1, row);

            QTableWidgetItem *item_1 = new QTableWidgetItem();
            item_1->setText(wave.at(i));

            sd_thick->ui->tableWidget->setItem(row, 0, item_1);
        }
        sedFitInputUlimitString += "]";
        connect(mapper, SIGNAL(mapped(int)), this, SLOT(checkboxClicked(int)));

        sd_thick->show();
        connect(sd_thick->ui->pushButton, SIGNAL(clicked()), this, SLOT(doThickLocalFit()));
    }
}

void SEDVisualizerPlot::executeRemoteCall(QString sedFitInputW, QString sedFitInputF,
                                          QString sedFitInputErrF, QString sedFitInputFflag,
                                          QMap<double, double> sedFitInput)
{
    QString filename = "sed_fit/execute.bin";
    QFile file(filename);

    if (file.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        QTextStream stream(&file);
        stream.reset();
        stream << "#!/bin/bash" << endl;
        stream << "unzip scripts.zip" << endl;
        stream << "/usr/local/bin/idl -e \"sedpar=vialactea_tap_sedfit_v3(" + sedFitInputW + ","
                        + sedFitInputF + "," + sedFitInputErrF + "," + sedFitInputFflag
                        + ",2000.,0.8,sed_weights=[1.,1.,1.],use_wave=" + sedFitInputW
                        + ")\" &> log.dat"
               << endl;
        stream << "zip output.zip *dat" << endl;
    }

    QProcess process_zip;
    process_zip.start("zip -j sed_fit/inputs.zip sed_fit/scripts.zip sed_fit/execute.bin ");
    process_zip.waitForFinished(); // sets current thread to sleep and waits for process end

    QProcess process;
    process.start("curl -k -F m=submit -F pass=hp39A11  -F wfdesc=@sed_fit/workflow.xml -F "
                  "inputzip=@sed_fit/inputs.zip -F portmapping=@sed_fit/portmapping.txt -F "
                  "certs=@sed_fit/certs.zip "
                  "http://via-lactea-sg00.iaps.inaf.it:8080/wspgrade/RemoteServlet");

    process.waitForFinished(); // sets current thread to sleep and waits for process end
    QString output(process.readAll());

    QString id = output.simplified();
    QFuture<bool> future = QtConcurrent::run(checkIDRemoteCall, id);
    bool finished = future.result();
    if (finished) {

        QProcess process_download;
        process_download.start("curl -k -F m=download -F ID=" + output.simplified()
                               + " -F pass=hp39A11 -o sed_fit/output.zip "
                                 "http://via-lactea-sg00.iaps.inaf.it:8080/wspgrade/RemoteServlet");
        process_download.waitForFinished(); // sets current thread to sleep and waits for
                                            // process end
        QString output_download(process_download.readAll());
        QProcess process_unzip;
        process_unzip.start("unzip sed_fit/output.zip -d "
                            + QDir::homePath()
                                      .append(QDir::separator())
                                      .append("VisIVODesktopTemp/tmp_download/"));
        process_unzip.waitForFinished(); // sets current thread to sleep and waits for
                                         // process end
        QString output_unzip(process_unzip.readAll());
        QStringList pieces = output_unzip.split(" ");
        QString path = pieces[pieces.length() - 3];

        path.replace(path.indexOf("guse.jsdl"), path.size(), QLatin1String("output.zip"));
        QProcess process_unzip_2;
        process_unzip_2.start("unzip " + path + " -d "
                              + QDir::homePath()
                                        .append(QDir::separator())
                                        .append("VisIVODesktopTemp/tmp_download/"));
        process_unzip_2.waitForFinished(); // sets current thread to sleep and waits for
                                           // process end
        QString output_unzip_2(process_unzip_2.readAll());
    }
}

bool SEDVisualizerPlot::checkIDRemoteCall(QString id)
{
    QString output_status = "";

    QProcess process_status;

    while (output_status.compare("finished") != 0) {
        process_status.start("curl -k -F m=info -F pass=hp39A11 -F ID=" + id
                             + " http://via-lactea-sg00.iaps.inaf.it:8080/wspgrade/RemoteServlet");
        process_status.waitForFinished(); // sets current thread to sleep and waits for
                                          // process end
        output_status = process_status.readAll().simplified();
        if (output_status.compare("error") == 0) {
            return false;
        }
    }
    return true;
}

void SEDVisualizerPlot::on_TheoreticalRemoteFit_triggered()
{
    bool validFit = false;

    sedFitInputFflag = "]";
    sedFitInputF = "]";
    sedFitInputW = "]";
    sedFitInputErrF = "]";
    sedFitInput.clear();

    validFit = prepareSelectedInputForSedFit();
    if (!validFit) {
        return;
    }

    sedFitInputF = "[" + sedFitInputF;
    sedFitInputW = "[" + sedFitInputW;
    sedFitInputFflag = "[" + sedFitInputFflag;
    sedFitInputErrF = "[" + sedFitInputErrF;
    // qDebug() <<"--sedFitInput data:" <<sedFitInputFflag << sedFitInputF << sedFitInputW
    // <<
    sedFitInputErrF;

    QString sedWeights = "[" + ui->mid_irLineEdit->text() + "," + ui->far_irLineEdit->text() + ","
            + ui->submmLineEdit->text() + "]";

    loading = new LoadingWidget();
    loading->init();
    loading->setText("Executing vialactea_tap_sedfit");
    loading->show();
    loading->activateWindow();
    loading->setFocus();

    ui->outputTextBrowser->setText("");

    QString args = QString("'%1_%2_%3_%4_%5_%6_%7_0_%8_%9'")
                           .arg(sedFitInputW)
                           .arg(sedFitInputF)
                           .arg(sedFitInputErrF)
                           .arg(sedFitInputFflag)
                           .arg(ui->distTheoLineEdit->text())
                           .arg(ui->prefilterLineEdit->text())
                           .arg(sedWeights)
                           .arg(sedFitInputW)
                           .arg(ui->delta_chi2_lineEdit->text());

    QString urlEncoded = "http://vlkb.ia2.inaf.it/searchd/?" + QUrl::toPercentEncoding(args);
    QNetworkRequest req(urlEncoded);
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    auto nam = new QNetworkAccessManager;
    auto reply = nam->get(req);

    loading->setLoadingProcess(reply);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        reply->deleteLater();
        nam->deleteLater();
        loading->close();

        // Network Error
        if (reply->error()) {
            qDebug() << "Remote SEDFit service error:" << reply->errorString();
            ui->outputTextBrowser->append(reply->errorString());
            QMessageBox::critical(this, "Error", reply->errorString());
            return;
        }

        // Service error
        auto response = reply->readAll();
        QJsonParseError parseError;
        auto json = QJsonDocument::fromJson(response, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "Remote SEDFit service error:" << response;
            ui->outputTextBrowser->append(response);
            QMessageBox::critical(this, "Error", response);
            return;
        }

        QString filePath = QDir::homePath().append("/VisIVODesktopTemp/tmp_download/SED"
                                                   + QString::number(nSED) + "_sedfit_output.dat");
        QFile outputFile(filePath);
        if (!outputFile.open(QIODevice::WriteOnly)) {
            qDebug() << "Remote SEDFit service error: " << outputFile.errorString();
            ui->outputTextBrowser->append(outputFile.errorString());
            QMessageBox::critical(this, "Error", outputFile.errorString());
            return;
        }

        outputFile.write(response);
        outputFile.close();

        readSedFitOutput(filePath);

        this->raise();
        this->activateWindow();
    });
}

void SEDVisualizerPlot::handleFinished()
{
    readSedFitOutput(QDir::homePath()
                             .append(QDir::separator())
                             .append("VisIVODesktopTemp/tmp_download/sedfit_output.dat"));
    QFile file_log(QDir::homePath()
                           .append(QDir::separator())
                           .append("VisIVODesktopTemp/tmp_download/log.dat"));
    if (!file_log.open(QFile::ReadOnly | QFile::Text))
        return;
    QTextStream in(&file_log);
    outputSedLog = in.readAll();
    ui->outputTextBrowser->setText(outputSedLog);
    loading->close();
    addNewSEDCheckBox("Theoretical Fit");
}

void SEDVisualizerPlot::on_logRadioButton_toggled(bool checked)
{
    if (checked == true) {
        // Show log file
        ui->outputTextBrowser->setText(outputSedLog);
        ui->resultsTableWidget->hide();
        ui->outputTextBrowser->show();
        if (temporaryRow != 0) {
            ui->customPlot->removeGraph(temporaryGraph);
            for (int i = 0; i < temporaryGraphPoints.size(); ++i) {
                SEDPlotPointCustom *cp;
                cp = temporaryGraphPoints.at(i);
                ui->customPlot->removeItem(cp);
            }
            temporaryGraphPoints.clear();
            temporaryRow = 0;
        }

    } else {
        // Show results
        ui->outputTextBrowser->setText(outputSedResults);
        ui->resultsTableWidget->show();
        ui->outputTextBrowser->hide();
    }
}

void SEDVisualizerPlot::on_clearAllButton_clicked()
{
    int size = sedGraphs.size();
    for (int i = 0; i < size; ++i) {
        QCPGraph *qcp;
        qcp = sedGraphs.at(i);
        ui->customPlot->removeGraph(qcp);
    }
    // TODO: To check if items are removed from memory!
    sedGraphs.clear();
    QMap<int, QList<SEDPlotPointCustom *>>::iterator iter;
    for (iter = sedGraphPoints.begin(); iter != sedGraphPoints.end(); ++iter) {
        QList<SEDPlotPointCustom *> points = sedGraphPoints.value(iter.key());
        for (int i = 0; i < points.size(); ++i) {
            SEDPlotPointCustom *cp;
            cp = points.at(i);
            ui->customPlot->removeItem(cp);
        }
    }
    sedGraphPoints.clear();

    if (temporaryRow != 0) { // there is a temporary graph still drawn
        ui->customPlot->removeGraph(temporaryGraph);
        for (int i = 0; i < temporaryGraphPoints.size(); ++i) {
            SEDPlotPointCustom *cp;
            cp = temporaryGraphPoints.at(i);
            ui->customPlot->removeItem(cp);
        }
        temporaryGraphPoints.clear();
    }

    ui->customPlot->replot();
    nSED = 0;
    temporaryMOD = false;
    doubleClicked = false;
    temporaryRow = 0;
    QLayoutItem *item;

    // the key point here is that the layout items are stored inside the layout in a stack
    while ((item = ui->generatedSedBox->layout()->takeAt(1)) != 0) {
        if (item->widget()) {
            ui->generatedSedBox->layout()->removeWidget(item->widget());
            delete item->widget();
        }
        delete item;
    }
    ui->resultsTableWidget->clearContents();
    ui->resultsTableWidget->setRowCount(0);
    ui->outputTextBrowser->setText("");
    outputSedLog = "";
    QDir tmp_download(QDir::homePath() + "/VisIVODesktopTemp/tmp_download");
    QStringList filters;
    filters << "SED*";
    tmp_download.setNameFilters(filters);
    QStringList dirList = tmp_download.entryList();

    for (int i = 0; i < dirList.size(); i++) {
        QFile sedFile(QDir::homePath() + "/VisIVODesktopTemp/tmp_download/" + dirList.at(i));
        sedFile.remove();
    }
    plottedSedLabels.clear();
}

void SEDVisualizerPlot::on_actionSave_triggered()
{
    QProcess process_zip;
    QDir tmp_download(QDir::homePath() + "/VisIVODesktopTemp/tmp_download");
    QStringList filters;
    filters << "SED*";
    tmp_download.setNameFilters(filters);
    QStringList dirList = tmp_download.entryList();
    QString sedZipPath = QDir::homePath() + "/VisIVODesktopTemp/tmp_download/SED.zip";
    for (int i = 0; i < dirList.size(); i++) {
        QString sedPath = QDir::homePath() + "/VisIVODesktopTemp/tmp_download/" + dirList.at(i);
        process_zip.start("zip -j " + sedZipPath + " " + sedPath);
        process_zip.waitForFinished(); // sets current thread to sleep and waits for process
                                       // end
        QString output_zip(process_zip.readAll());
    }

    QString fileName =
            QFileDialog::getSaveFileName(this, tr("Save SED fits"), QDir::homePath(), tr("*.zip"));
    if (!fileName.endsWith(".zip", Qt::CaseInsensitive))
        fileName.append(".zip");
    QFile::copy(sedZipPath, fileName);
}

void SEDVisualizerPlot::on_actionLoad_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load SED fits"), QDir::homePath(),
                                                    tr("Archive (*.zip)"));
    QString sedZipPath = QDir::homePath() + "/VisIVODesktopTemp/tmp_download/SED.zip";
    QFile::copy(fileName, sedZipPath);
    QProcess process_unzip;
    process_unzip.start("unzip " + sedZipPath + " -d " + QDir::homePath()
                        + "/VisIVODesktopTemp/tmp_download");
    process_unzip.waitForFinished(); // sets current thread to sleep and waits for process end
    QString output_unzip(process_unzip.readAll());
    QDir tmp_download(QDir::homePath() + "/VisIVODesktopTemp/tmp_download");
    QStringList filters;
    filters << "SED*";
    tmp_download.setNameFilters(filters);
    QStringList dirList = tmp_download.entryList();

    for (int i = 0; i < dirList.size(); i++) {
        QString sedPath = QDir::homePath() + "/VisIVODesktopTemp/tmp_download/" + dirList.at(i);
        QString sedNumber = dirList.at(i).mid(3, 1); // substring after "SED";
        bool ok;
        int value = sedNumber.toInt(&ok);
        if (ok) { // is an integer
            if (sedPath.endsWith("sedfit_model.dat")) {
                addNewSEDCheckBox("Theoretical Fit");
                QFile file(sedPath);
                file.open(QIODevice::ReadOnly);
                QDataStream in(&file); // read the data serialized from the file
                QVector<QStringList> headerAndValueList;
                in >> headerAndValueList; // extract data
                setModelFitValue(headerAndValueList, Qt::blue);
            }
            if (sedPath.endsWith("sedfit_output.dat")) {
                loadSedFitOutput(sedPath);
            }
            if (sedPath.endsWith("thinfile.csv")) {
                readThinFit(sedPath);
                addNewSEDCheckBox("Thin Fit");
            }
            if (sedPath.endsWith("thickfile.csv")) {
                readThinFit(sedPath);
                addNewSEDCheckBox("Thick Fit");
            }
        }
    }
}

void SEDVisualizerPlot::loadSavedSED(QStringList dirList)
{
    for (int i = 0; i < dirList.size(); i++) {
        QString sedPath = QDir::homePath() + "/VisIVODesktopTemp/tmp_download/" + dirList.at(i);
        QString sedNumber = dirList.at(i).mid(3, 1); // substring after "SED";
        bool ok;
        int value = sedNumber.toInt(&ok);
        if (ok) { // is an integer
            if (sedPath.endsWith("sedfit_model.dat")) {
                QString sedOutput = QDir::homePath() + "/VisIVODesktopTemp/tmp_download/SED"
                        + QString::number(value) + "_sedfit_output.dat";
                loadSedFitOutput(sedOutput);
                QFile file(sedPath);
                file.open(QIODevice::ReadOnly);
                QDataStream in(&file); // read the data serialized from the file
                QVector<QStringList> headerAndValueList;
                in >> headerAndValueList; // extract data
                setModelFitValue(headerAndValueList, Qt::blue);
            }

            if (sedPath.endsWith("thinfile.csv")) {
                readThinFit(sedPath);
                addNewSEDCheckBox("Thin Fit");
            }
            if (sedPath.endsWith("thickfile.csv")) {
                readThinFit(sedPath);
                addNewSEDCheckBox("Thick Fit");
            }
        }
    }
}

void SEDVisualizerPlot::on_collapseCheckBox_toggled(bool checked)
{
    collapsedGraph->setVisible(checked);
    collapsedNodes->setVisible(checked);
    for (int i = 0; i < collapsedGraphPoints.size(); ++i) {
        SEDPlotPointCustom *cp;
        cp = collapsedGraphPoints.at(i);
        cp->setVisible(checked);
    }
    QPen pen;

    if (checked) {
        graphSEDNodes->setSelectable(QCP::stNone); // disable graphSEDNodes selection
        // set originalgraph in dotline gray
        pen.setStyle(Qt::DashLine);
        pen.setColor(Qt::lightGray);
        // TODO rendere selezionabile i collapse node (già gestita dalla setVisible?)
    } else {
        // set original graph line blue
        pen.setStyle(Qt::SolidLine);
        pen.setColor(Qt::blue);

        // TODO alla chiusura della finestra eliminare tutto
        // collapsedGraphPoints.clear();
        // removeItem(cp)
        // TODO rendere non selezionabile i collpase nodi (già gestita dalla setVisible?)
        graphSEDNodes->setSelectable(QCP::stMultipleDataRanges); // enable graphSEDNodes selection
    }
    // set originalGraphs style dash-lightGray or soliline-blue
    for (int i = 0; i < originalGraphs.size(); i++) {
        originalGraphs.at(i)->setPen(pen);
    }
    ui->customPlot->replot();
}

void SEDVisualizerPlot::addNewTheoreticalFit()
{
    QModelIndex index = ui->resultsTableWidget->selectionModel()->selectedIndexes().first();
    doubleClicked = true;
    temporaryMOD = false;
    int id = ui->resultsTableWidget->item(index.row(), 0)->text().toInt();
    if (index.row() == 0) {
        // Do nothing on the first row which is already drawn by default
    } else {

        QString query = "SELECT * FROM compactsources.sed_models where id=" + QString::number(id);
        new VLKBQuery(query, vtkwin, "model", this, Qt::cyan);
        if (temporaryRow != 0) {
            ui->customPlot->removeGraph(temporaryGraph);
            for (int i = 0; i < temporaryGraphPoints.size(); ++i) {
                SEDPlotPointCustom *cp;
                cp = temporaryGraphPoints.at(i);
                ui->customPlot->removeItem(cp);
            }
            temporaryGraphPoints.clear();
            temporaryRow = 0;
        }
    }
    this->setFocus();
    this->activateWindow();
}

void SEDVisualizerPlot::on_resultsTableWidget_clicked(const QModelIndex &index)
{
    if (!doubleClicked) {
        temporaryMOD = true;
        if (index.row() == 0) {
            // Do nothing on the first row which is already drawn by default
            // if a temporary graph was shown delete it
            if (temporaryGraphPoints.size() != 0) {
                ui->customPlot->removeGraph(temporaryGraph);
                for (int i = 0; i < temporaryGraphPoints.size(); ++i) {
                    SEDPlotPointCustom *cp;
                    cp = temporaryGraphPoints.at(i);
                    ui->customPlot->removeItem(cp);
                }
                temporaryGraphPoints.clear();
                temporaryRow = 0;
                ui->customPlot->replot();
            }
        } else if (index.row() != temporaryRow) {
            if (temporaryRow != 0) {
                ui->customPlot->removeGraph(temporaryGraph);
                for (int i = 0; i < temporaryGraphPoints.size(); ++i) {
                    SEDPlotPointCustom *cp;
                    cp = temporaryGraphPoints.at(i);
                    ui->customPlot->removeItem(cp);
                }
                temporaryGraphPoints.clear();
            }

            temporaryRow = index.row();
            plotSedFitModel(models.values().at(index.row()), Qt::lightGray);
        }
        doubleClicked = false;
    }
    this->setFocus();
    this->activateWindow();
}

void SEDVisualizerPlot::on_theoreticalPushButton_clicked()
{
    ui->theorGroupBox->show();
    ui->greyBodyGroupBox->hide();
}

void SEDVisualizerPlot::on_theorConfirmPushButton_clicked()
{
    on_TheoreticalRemoteFit_triggered();
}

void SEDVisualizerPlot::on_greyBodyPushButton_clicked()
{
    ui->theorGroupBox->hide();
    ui->greyBodyGroupBox->show();
}

void SEDVisualizerPlot::on_Thick_clicked()
{
    on_ThickLocalFit_triggered();
}

void SEDVisualizerPlot::on_thinButton_clicked()
{
    on_ThinLocalFit_triggered();
}

/**
 * Method for setting SEDNodes and CollapseNodes decorator style on drag selection
 * @brief createScatterStyle
 * @param shape
 * @param size
 * @param color
 * @param penWidth
 * @return QCPScatterStyle
 */
QCPScatterStyle SEDVisualizerPlot::createScatterStyle(QCPScatterStyle::ScatterShape shape,
                                                      double size, QColor color, double penWidth)
{
    QCPScatterStyle scatterStyle;
    scatterStyle.setShape(shape);
    scatterStyle.setSize(size);
    QPen pen(color);
    pen.setWidthF(penWidth);
    scatterStyle.setPen(pen);
    return scatterStyle;
}

void SEDVisualizerPlot::setDragSelection()
{
    QCPScatterStyle myScatter = createScatterStyle(QCPScatterStyle::ssCircle, 8, Qt::transparent,
                                                   2.0); // invisible by default
    QCPScatterStyle selectedScatter =
            createScatterStyle(QCPScatterStyle::ssCircle, 8, Qt::red, 2.0); // red circle selection

    QCPSelectionDecorator *decorator = new QCPSelectionDecorator();
    decorator->setScatterStyle(selectedScatter);

    graphSEDNodes->setSelectionDecorator(decorator);
    graphSEDNodes->setScatterStyle(myScatter);

    // or
    // graphSEDNodes->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 16));

    // drag selection mode
    ui->customPlot->setSelectionRectMode(QCP::srmSelect);
    graphSEDNodes->setSelectable(QCP::stMultipleDataRanges);
}

/**
 * Holding 'shift' allows graph navigation (by disabling drag selection)
 * Holding 'Control/Command' allows multi selection/deselection
 * @brief SEDVisualizerPlot::keyPressEvent
 * @param event
 */
void SEDVisualizerPlot::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Shift) {
        ui->customPlot->setSelectionRectMode(QCP::srmNone);
        shiftMovingStatus = true;
    }
    if (event->key() == Qt::Key_Control) {
        multiSelectionPointStatus = true;
    }

    QMainWindow::keyPressEvent(event);
}

/**
 * Reset drag selection mode realeasing 'shift'
 * Reset multi selection mode realeasing 'Control/Command'
 * @brief SEDVisualizerPlot::keyReleaseEvent
 * @param event
 */
void SEDVisualizerPlot::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Shift) {
        ui->customPlot->setSelectionRectMode(QCP::srmSelect);
        shiftMovingStatus = false;
    }
    if (event->key() == Qt::Key_Control) {
        multiSelectionPointStatus = false;
    }

    QMainWindow::keyPressEvent(event);
}

void SEDVisualizerPlot::closeEvent(QCloseEvent *event)
{
    if (vtkwin != 0) {
        // remove all ellipses
        for (auto ellipseActor = ellipseActorMap.constBegin();
             ellipseActor != ellipseActorMap.constEnd(); ++ellipseActor) {
            vtkwin->removeActor(ellipseActor.value());
        }
        // clear ellipses support
        ellipseActorMap.clear();
        vtkwin->updateScene();
    }
    event->accept();
}
