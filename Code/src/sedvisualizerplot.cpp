#include "sedvisualizerplot.h"
#include "ui_sedvisualizerplot.h"
#include <QInputDialog>
#include "sedplotpointcustom.h"
#include "loadingwidget.h"
#include "visivoimporterdesktop.h"
#include "vlkbquery.h"
#include <QFileDialog>
#include "sedfitgrid_thin.h"
#include "ui_sedfitgrid_thin.h"
#include "ui_sedfitgrid_thick.h"
#include <QSignalMapper>
#include <QCheckBox>
#include <QtConcurrent>

SEDVisualizerPlot::SEDVisualizerPlot(QList<SED *> s, vtkwindow_new *v, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SEDVisualizerPlot)
{
    ////qDebug()<<"SEDVisualizerPlot created";
    ui->setupUi(this);


    ui->theorGroupBox->hide();
    ui->greyBodyGroupBox->hide();


    // QString m_sSettingsFile = QApplication::applicationDirPath()+"/setting.ini";
    QString  m_sSettingsFile = QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp").append("/setting.ini");

    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);
    idlPath = settings.value("idlpath", "").toString();


    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes | QCP::iSelectLegend | QCP::iSelectPlottables| QCP::iMultiSelect | QCP::iSelectItems);

    sed_list=s;
    vtkwin=v;

    ui->resultsTableWidget->hide();
    ui->resultsTableWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
    QAction* addFitAction = new QAction("Add this fit", ui->resultsTableWidget);
    ui->resultsTableWidget->addAction(addFitAction);
    connect(addFitAction, SIGNAL(triggered()), this, SLOT(addNewTheoreticalFit()));


    ui->customPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->customPlot->yAxis->setScaleLogBase(10);


    ui->customPlot->xAxis->setScaleType(QCPAxis::stLogarithmic);
    ui->customPlot->xAxis->setScaleLogBase(10);

    ui->customPlot->plotLayout()->insertRow(0);
    ui->customPlot->plotLayout()->addElement(0, 0, new QCPPlotTitle(ui->customPlot, "SED"));

    ui->customPlot->xAxis->setLabel("Wavelength ["+QString::fromUtf8("\u00b5")+"m]");
    ui->customPlot->yAxis->setLabel("Flux [Jy]");

    minWavelen=std::numeric_limits<int>::max();;
    maxWavelen= std::numeric_limits<int>::min();

    minFlux=std::numeric_limits<int>::max();
    maxFlux= std::numeric_limits<int>::min();

    QString name;


   // drawNode(s.at(0)->getRootNode());

    for (sedCount=0;sedCount<s.count();sedCount++)
    {
        //qDebug()<<"SEDVisualizerPlot drawing sed "<<QString::number(sedCount);
        sed=s.at(sedCount);
        drawNode(sed->getRootNode());
    }

    double x_deltaRange=(maxWavelen-minWavelen)*0.02;
    double y_deltaRange=(maxFlux-minFlux)*0.02;

    // ui->customPlot->xAxis->setRange(minWavelen-x_deltaRange, maxWavelen+x_deltaRange);
    // ui->customPlot->yAxis->setRange(minFlux-y_deltaRange, maxFlux+y_deltaRange);
    ui->customPlot->yAxis->setRange(minFlux-y_deltaRange, maxFlux+y_deltaRange);
    ui->customPlot->xAxis->setRange(minWavelen-x_deltaRange, maxWavelen+x_deltaRange);


    //qDebug()<<"Wavelen: "<<maxWavelen-minWavelen<<" "<<x_deltaRange;
    //qDebug()<<"Flux: "<<maxFlux-minFlux<<" "<<y_deltaRange;

    // connect slot that ties some axis selections together (especially opposite axes):
    connect(ui->customPlot, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));
    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePress()));
    connect(ui->customPlot, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));
    connect(ui->customPlot, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseRelease()));

    // make bottom and left axes transfer their ranges to top and right axes:
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->customPlot->yAxis2, SLOT(setRange(QCPRange)));

    // connect some interaction slots:
    connect(ui->customPlot, SIGNAL(titleDoubleClick(QMouseEvent*,QCPPlotTitle*)), this, SLOT(titleDoubleClick(QMouseEvent*,QCPPlotTitle*)));
    connect(ui->customPlot, SIGNAL(axisDoubleClick(QCPAxis*,QCPAxis::SelectablePart,QMouseEvent*)), this, SLOT(axisLabelDoubleClick(QCPAxis*,QCPAxis::SelectablePart)));
    //    connect(ui->customPlot, SIGNAL(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*,QMouseEvent*)), this, SLOT(legendDoubleClick(QCPLegend*,QCPAbstractLegendItem*)));

    // connect slot that shows a message in the status bar when a graph is clicked:
    connect(ui->customPlot, SIGNAL(plottableClick(QCPAbstractPlottable*,QMouseEvent*)), this, SLOT(graphClicked(QCPAbstractPlottable*)));

    // setup policy and connect slot for context menu popup:
    ui->customPlot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->customPlot, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenuRequest(QPoint)));



    modelFitBands.insert("wise1",3.37);
    modelFitBands.insert("i1",3.6);
    modelFitBands.insert("i2",4.5);
    modelFitBands.insert("wise2",4.62);
    modelFitBands.insert("i3",5.8);
    modelFitBands.insert("i4",8);
    modelFitBands.insert("wise3",12.08);
    modelFitBands.insert("wise4",22.194);
    modelFitBands.insert("m1",24);
    modelFitBands.insert("pacs1",70);
    modelFitBands.insert("pacs2",100);
    modelFitBands.insert("pacs3",160);
    modelFitBands.insert("spir1",250);
    modelFitBands.insert("spir2",350);
    modelFitBands.insert("spir3",500);
    modelFitBands.insert("laboc",870);
    modelFitBands.insert("bol11",1100);

    columnNames.append("id");
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
    columnNames.append("m1");
    columnNames.append("pacs1");
    columnNames.append("pacs2");
    columnNames.append("pacs3");
    columnNames.append("spir1");
    columnNames.append("spir2");
    columnNames.append("spir3");
    columnNames.append("laboc");
    columnNames.append("bol11");
    columnNames.append("CHI2");
    columnNames.append("DIST");


    resultsOutputColumn.insert("id",-1);
    resultsOutputColumn.insert("clump_mass",-1);
    resultsOutputColumn.insert("compact_mass_fraction",-1);
    resultsOutputColumn.insert("clump_upper_age",-1);
    resultsOutputColumn.insert("dust_temp",-1);
    resultsOutputColumn.insert("bolometric_luminosity",-1);
    resultsOutputColumn.insert("n_star_tot",-1);
    resultsOutputColumn.insert("m_star_tot",-1);
    resultsOutputColumn.insert("n_star_zams",-1);
    resultsOutputColumn.insert("m_star_zams",-1);
    resultsOutputColumn.insert("l_star_tot",-1);
    resultsOutputColumn.insert("l_star_zams",-1);
    resultsOutputColumn.insert("zams_luminosity_1",-1);
    resultsOutputColumn.insert("zams_mass_1",-1);
    resultsOutputColumn.insert("zams_temperature_1",-1);
    resultsOutputColumn.insert("zams_luminosity_2",-1);
    resultsOutputColumn.insert("zams_mass_2",-1);
    resultsOutputColumn.insert("zams_temperature_2",-1);
    resultsOutputColumn.insert("zams_luminosity_3",-1);
    resultsOutputColumn.insert("zams_mass_3",-1);
    resultsOutputColumn.insert("zams_temperature_3",-1);
    resultsOutputColumn.insert("m1",-1);
    resultsOutputColumn.insert("pacs1",-1);
    resultsOutputColumn.insert("pacs2",-1);
    resultsOutputColumn.insert("pacs3",-1);
    resultsOutputColumn.insert("spir1",-1);
    resultsOutputColumn.insert("spir2",-1);
    resultsOutputColumn.insert("spir3",-1);
    resultsOutputColumn.insert("laboc",-1);
    resultsOutputColumn.insert("bol11",-1);
    resultsOutputColumn.insert("CHI2",-1);
    resultsOutputColumn.insert("DIST",-1);

    QString libPath= QFileInfo(idlPath).absolutePath().append("/../lib/")+":"+qApp->applicationDirPath().append("/script_idl/")+":"+qApp->applicationDirPath().append("/script_idl/astrolib")  ;
    setenv("IDL_PATH",libPath.toStdString().c_str(),0);

    ui->generatedSedBox->setHidden(true);
    nSED=0;
    multiSelectMOD=false;
    temporaryMOD=false;
    doubleClicked=false;
    temporaryRow=0;
    this->setFocus();
    this->activateWindow();

    //Save QList<SED *> s to a file to be loaded later on

    QFile sedFile(QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SEDList.dat"));
    if(!sedFile.exists()){
        sedFile.open(QIODevice::WriteOnly);
        QDataStream out(&sedFile);
        out << sed_list;   // serialize the object
    }
    sedFile.flush();
    sedFile.close();

    for(int i=0;i<ui->customPlot->graphCount();i++)
    {
        originalGraphs.push_back(ui->customPlot->graph(i));
    }

    ui->histogramPlot->hide();
    connect(ui->resultsTableWidget->horizontalHeader(), SIGNAL( sectionClicked( int ) ), this, SLOT( sectionClicked( int ) ) );

}

QDataStream &operator<<(QDataStream &out, QList<SED *> &sedlist)
{
    out<<sedlist.count();
    foreach(SED* sed, sedlist){
        out<<sed;
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

void SEDVisualizerPlot::sectionClicked ( int index )
{
    if(index>=1&&index<=4){
        ui->histogramPlot->clearPlottables();
        //qDebug() << "Clicked Index "<<index;
        QVector<double> values=sedFitValues.value(index+1);
        double min,max,bin_width;
        QString title;
        if(index==1){//clump_mass
            //min=10;
            //max=200000;
            min=1;
            max=5.3;
            bin_width=0.1;
            title="clump_mass (log scale)";
        }
        if(index==2){//compact_mass_fraction
            min=0;
            max=0.5;
            bin_width=0.025;
            title="compact_mass_fraction";
        }
        if(index==3){//clump_upper_age
            min=4;
            max=5.7;
            bin_width=0.25;
            title="clump_upper_age (log scale)";
        }
        if(index==4){//dust_temp
            min=5;
            max=50;
            bin_width=1;
            title="dust_temp";
        }
        ui->histogramPlot->show();
        int bin_count=(max-min)/bin_width;
        QVector<double> x1(bin_count), y1(bin_count);
        for (int i=0; i<values.size(); ++i)
        {
            int bin;
            if(index==1){
                bin = (int)((log10(values.at(i)) - min) / bin_width);
            }else if(index==3){
                bin = (int)((log10(values.at(i)*100000) - min) / bin_width);
            }else{
                bin = (int)((values.at(i) - min) / bin_width);
            }
            if(bin>=0 && bin<bin_count){
                y1[bin]++;
            }
        }
        x1[0]=min;
        for (int i=1; i<bin_count; ++i)
        {
            x1[i]=x1[i-1]+bin_width;
        }

        //qDebug()<<x1.size();

        QCPBars *bars1 = new QCPBars(ui->histogramPlot->xAxis, ui->histogramPlot->yAxis);
        bars1->setWidth(bin_width);
        bars1->setData(x1, y1);
        ui->histogramPlot->xAxis->setRange(min, max);
        ui->histogramPlot->yAxis->setRange(0,values.size());
        //if(index==1||index==3){
        //ui->histogramPlot->xAxis->setScaleType(QCPAxis::stLogarithmic);
        //ui->histogramPlot->xAxis->setScaleLogBase(10);

        //}else{
        //    ui->histogramPlot->xAxis->setScaleType(QCPAxis::stLinear);
        //}
        //ui->histogramPlot->plotLayout()->insertRow(0);
        //ui->histogramPlot->plotLayout()->addElement(0, 0, new QCPTextElement(customPlot, title, QFont("sans", 12, QFont::Bold)));
        ui->histogramPlot->xAxis->setLabel(title);
        ui->histogramPlot->yAxis->setAutoTickStep(false);
        ui->histogramPlot->yAxis->setTickStep(1);
        ui->histogramPlot->addPlottable(bars1);
        ui->histogramPlot->replot();
    }else{
        QMessageBox msgBox;
        msgBox.setText("No histogram available for this values.");
        msgBox.exec();
    }
}



QDataStream &operator>>(QDataStream &in, QList<SED *> &sedlist)
{
    //qDebug()<< "reading SED list";
    int count;
    in >> count;
    //qDebug()<< "SED list size : "<<QString::number(count);
    for(int i=0; i<count; i++){
        SED * sed=new SED();
        in >> sed;
        sedlist.push_back(sed);
    }
    //qDebug()<< "SED list size : "<<QString::number(sedlist.count());
    return in;
}



void SEDVisualizerPlot::drawNode( SEDNode *node)
{

    //qDebug()<<"drawNode "<<node->getDesignation()<< " esiste "<<visualnode_hash.contains(node->getDesignation());
    if(!visualnode_hash.contains(node->getDesignation()))
    {
        ////qDebug()<<"drawNode inside if";
        SEDPlotPointCustom *cp = new SEDPlotPointCustom(ui->customPlot,3.5,vtkwin);

        if(vtkwin!=0){

            if (vtkwin->getDesignation2fileMap().values(node->getDesignation()).length() >0)
            {
                double r= vtkwin->getEllipseActorList().value( vtkwin->getDesignation2fileMap().value(node->getDesignation()) )->GetProperty()->GetColor()[0];
                double g= vtkwin->getEllipseActorList().value( vtkwin->getDesignation2fileMap().value(node->getDesignation()) )->GetProperty()->GetColor()[1];
                double b= vtkwin->getEllipseActorList().value( vtkwin->getDesignation2fileMap().value(node->getDesignation()) )->GetProperty()->GetColor()[2];

                cp->setColor(QColor(r*255,g*255,b*255));

            }
        }
        cp->setAntialiased(true);
        cp->setPos(node->getWavelength(), node->getFlux());
        cp->setDesignation(node->getDesignation());
        cp->setX(node->getX());
        cp->setY(node->getY());
        cp->setLat(node->getLat());
        cp->setLon(node->getLon());
        cp->setErrorFlux(node->getErrFlux());

        cp->setEllipse(node->getSemiMinorAxisLength(),node->getSemiMajorAxisLength(),node->getAngle(),node->getArcpix());

        cp->setNode(node);
        ui->customPlot->addItem(cp);
        //connect(ui->customPlot,SIGNAL(mousePress(QMouseEvent*)),cp,SLOT(onMousePress(QMouseEvent*)));

        visualnode_hash.insert(node->getDesignation(),cp);

        if (node->getWavelength()>maxWavelen)
            maxWavelen=node->getWavelength();
        if ( node->getWavelength() < minWavelen)
            minWavelen=node->getWavelength();


        if (node->getFlux()>maxFlux)
            maxFlux=node->getFlux();
        if ( node->getFlux() < minFlux)
        {
            minFlux=node->getFlux();
        }


        all_sed_node.insert(node->getWavelength(),node);
    }

   // //qDebug()<<"drawNode outside if";


    QVector<double> x(2), y(2), y_err(2);

    x[0]=node->getWavelength();
    y[0]=node->getFlux();
    y_err[0]=node->getErrFlux();

   // //qDebug()<<"drawNode child count "<<QString::number(node->getChild().count());

    for (int i=0;i<node->getChild().count();i++)
    {
        drawNode(node->getChild().values()[i]);

        x[1]=node->getChild().values()[i]->getWavelength();
        y[1]=node->getChild().values()[i]->getFlux();
        y_err[1]=node->getChild().values()[i]->getErrFlux();

        ui->customPlot->addGraph();

        ui->customPlot->graph()->setDataValueError(x, y, y_err);
        ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDot, 1));

        ui->customPlot->graph()->setPen(QPen(Qt::black));
        ui->customPlot->graph()->setSelectedPen(QPen(Qt::red));
        ui->customPlot->graph()->setAntialiased(true);
        ui->customPlot->graph()->setErrorType(QCPGraph::etBoth);

    }


}


void SEDVisualizerPlot::titleDoubleClick(QMouseEvent* event, QCPPlotTitle* title)
{
    Q_UNUSED(event)
    // Set the plot title by double clicking on it
    bool ok;
    QString newTitle = QInputDialog::getText(this, "SED Title", "New SED title:", QLineEdit::Normal, title->text(), &ok);
    if (ok)
    {
        title->setText(newTitle);
        ui->customPlot->replot();
    }
}

void SEDVisualizerPlot::axisLabelDoubleClick(QCPAxis *axis, QCPAxis::SelectablePart part)
{
    // Set an axis label by double clicking on it
    if (part == QCPAxis::spAxisLabel) // only react when the actual axis label is clicked, not tick label or axis backbone
    {
        bool ok;
        QString newLabel = QInputDialog::getText(this, "QCustomPlot example", "New axis label:", QLineEdit::Normal, axis->label(), &ok);
        if (ok)
        {
            axis->setLabel(newLabel);
            ui->customPlot->replot();
        }
    }
}

void SEDVisualizerPlot::legendDoubleClick(QCPLegend *legend, QCPAbstractLegendItem *item)
{
    // Rename a graph by double clicking on its legend item
    Q_UNUSED(legend)
    if (item) // only react if item was clicked (user could have clicked on border padding of legend where there is no item, then item is 0)
    {
        QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem*>(item);
        bool ok;
        QString newName = QInputDialog::getText(this, "QCustomPlot example", "New graph name:", QLineEdit::Normal, plItem->plottable()->name(), &ok);
        if (ok)
        {
            plItem->plottable()->setName(newName);
            ui->customPlot->replot();
        }
    }
}

void SEDVisualizerPlot::selectionChanged()
{
    if(multiSelectMOD){ // on "multi select mode" prevent a graph to be clicked. Only nodes can be selected.
        // //qDebug()<<"selected graph count "<<ui->customPlot->selectedGraphs().count();
        for(int i = 0; i < ui->customPlot->graphCount(); ++i)
            ui->customPlot->graph(i)->setSelected(false);
    }

}

void SEDVisualizerPlot::mouseRelease()
{

}

void SEDVisualizerPlot::mousePress()
{
    if(multiSelectMOD){
        QList<QCPAbstractItem *> list_items=ui->customPlot->selectedItems();
        ////qDebug()<<"Selected Items : "<<list_items.size();
    }
    else{

        // //qDebug()<<"pre - # selzionato: " <<ui->customPlot->selectedGraphs().count();

        for(int i = 0; i < ui->customPlot->graphCount(); ++i)
            ui->customPlot->graph(i)->setSelected(true);

        ui->customPlot->replot();


        // //qDebug()<<"post - # selzionato: " <<ui->customPlot->selectedGraphs().count();
        // //qDebug()<<"# grap: "<<ui->customPlot->graphCount();
    }

    // if an axis is selected, only allow the direction of that axis to be dragged
    // if no axis is selected, both directions may be dragged

    if (ui->customPlot->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->xAxis->orientation());
    else if (ui->customPlot->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
        ui->customPlot->axisRect()->setRangeDrag(ui->customPlot->yAxis->orientation());
    else
        ui->customPlot->axisRect()->setRangeDrag(Qt::Horizontal|Qt::Vertical);
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
        ui->customPlot->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}


void SEDVisualizerPlot::removeSelectedGraph()
{
    if (ui->customPlot->selectedGraphs().size() > 0)
    {
        ui->customPlot->removeGraph(ui->customPlot->selectedGraphs().first());
        ui->customPlot->replot();
    }
}

void SEDVisualizerPlot::removeAllGraphs()
{
    ui->customPlot->clearGraphs();
    ui->customPlot->replot();
}

void SEDVisualizerPlot::contextMenuRequest(QPoint pos)
{


    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    if (ui->customPlot->legend->selectTest(pos, false) >= 0) // context menu on legend requested
    {
        menu->addAction("Move to top left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignLeft));
        menu->addAction("Move to top center", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignHCenter));
        menu->addAction("Move to top right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignTop|Qt::AlignRight));
        menu->addAction("Move to bottom right", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignRight));
        menu->addAction("Move to bottom left", this, SLOT(moveLegend()))->setData((int)(Qt::AlignBottom|Qt::AlignLeft));
    }
    else  // general context menu on graphs requested
    {
        if (ui->customPlot->selectedGraphs().size() > 0)
            menu->addAction("Remove selected connection", this, SLOT(removeSelectedGraph()));
    }

    menu->popup(ui->customPlot->mapToGlobal(pos));
}


void SEDVisualizerPlot::graphClicked(QCPAbstractPlottable *plottable)
{
    //ui->statusbar->showMessage(QString("Clicked on graph '%1'.").arg(plottable->name()), 1000);
}

SEDVisualizerPlot::~SEDVisualizerPlot()
{
    delete ui;
}



void SEDVisualizerPlot::on_actionEdit_triggered()
{

}

void SEDVisualizerPlot::on_actionFit_triggered()
{


}

bool SEDVisualizerPlot::prepareInputForSedFit( SEDNode *node)
{
    bool validFit=true;

    //qDebug()<<"Selected Items no multi : ";

    sedFitInput.insert(node->getWavelength(),node->getFlux());
    sedFitInputF= QString::number( node->getFlux() )+sedFitInputF;
    sedFitInputErrF= QString::number( node->getErrFlux() )+sedFitInputErrF;
    sedFitInputW= QString::number( node->getWavelength() )+sedFitInputW;

    sedFitInputFflag ="1"+sedFitInputFflag;




    for (int i=0; i<node->getChild().count(); i++)
    {



        sedFitInputF= ","+sedFitInputF;
        sedFitInputErrF= ","+sedFitInputErrF;
        sedFitInputW= ","+sedFitInputW;
        sedFitInputFflag= ","+sedFitInputFflag;
        prepareInputForSedFit(node->getChild().values()[i]);


    }

    return    validFit;


}

bool SEDVisualizerPlot::prepareSelectedInputForSedFit()
{
    bool validFit=false;

    QMap <double, SEDNode *> selected_sed_map;
    QList<QCPAbstractItem *> list_items=ui->customPlot->selectedItems();
    for(int i=0;i<list_items.size();i++){
        QString className=QString::fromUtf8(list_items.at(i)->metaObject()->className());
        QString refName="SEDPlotPointCustom";
        ////qDebug()<<"Class name :"<<className<<" "<<QString::compare(className,refName);
        if(QString::compare(className,refName)==0){
            ////qDebug()<<"Insert item";
            SEDPlotPointCustom * cp=qobject_cast<SEDPlotPointCustom *>(list_items.at(i));
            selected_sed_map.insert(cp->getNode()->getWavelength(),cp->getNode());
        }
    }
    //qDebug()<<"Selected Items : "<<selected_sed_map.size();

    if(selected_sed_map.size()>=2){
        QMap<double, SEDNode *>::iterator iter;
        iter=selected_sed_map.begin();
        SEDNode *node=iter.value();
        sedFitInput.insert(node->getWavelength(),node->getFlux());
        sedFitInputF= QString::number( node->getFlux() )+sedFitInputF;
        sedFitInputErrF= QString::number( node->getErrFlux() )+sedFitInputErrF;
        sedFitInputW= QString::number( node->getWavelength() )+sedFitInputW;
        sedFitInputFflag ="1"+sedFitInputFflag;
        ++iter;
        for (; iter!=selected_sed_map.end(); ++iter)
        {
            node=iter.value();
            sedFitInput.insert(node->getWavelength(),node->getFlux());
            sedFitInputF= QString::number( node->getFlux() )+","+sedFitInputF;
            sedFitInputErrF= QString::number( node->getErrFlux() )+","+sedFitInputErrF;
            sedFitInputW= QString::number( node->getWavelength() )+","+sedFitInputW;
            sedFitInputFflag ="1,"+sedFitInputFflag;
        }
        validFit=true;
    }
    else
    {
        QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("Could not execute SED fit with less than 2 points selected.\n\rPlease select more points and try again."));
        validFit=false;
    }

    return validFit;
}


void SEDVisualizerPlot::on_actionLocal_triggered()
{


}

void SEDVisualizerPlot::readSedFitResultsHeader(QString header)
{
    ////qDebug()<<header;
    QList<QString> line_list_string=header.split(',');
    QString field;
    for(int i=0; i<line_list_string.size();i++){
        field=line_list_string.at(i).simplified();
        ////qDebug()<<field;
        if(resultsOutputColumn.contains(field)){
            resultsOutputColumn.insert(field,i);
            ////qDebug()<<field<<"->"<<resultsOutputColumn.value(field);
        }
    }
}

void SEDVisualizerPlot::readSedFitOutput(QString filename)
{
    int id;
    double chi2=99999999999;
    double tmp;
    QString chi_str,id_str,dist_str,clamp_mass_str,clamp_upper_age_str, compact_mass_fraction_str, dust_temp_str;
    QMap <double, int> results;
    QMap <double, QList<QByteArray> > resultsLines;
    QVector<double> clamp_mass;
    QVector<double> clamp_upper_age;
    QVector<double> compact_mass_fraction;
    QVector<double> dust_temp;

    //qDebug()<<"filename: "<<filename;
    QFile file(filename);
    // QFile file(qApp->applicationDirPath()+"/sedfit_output.dat");
    if (!file.open(QIODevice::ReadOnly)) {
        //qDebug() << file.errorString();
        return ;
    }
    //read, and skip the first line (header)
    //file.readLine();
    QString header=file.readLine();

    outputSedResults.append(header);
    readSedFitResultsHeader(header);
    //  QStringList wordList;
    int i=0;
    ui->resultsTableWidget->clearContents();
    ui->resultsTableWidget->setRowCount(0);
    ui->resultsTableWidget->setColumnCount(columnNames.size());
    ui->resultsTableWidget->setHorizontalHeaderLabels(columnNames);

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        //wordList.append(line.split(',').first());
        outputSedResults.append(line);



        QList<QByteArray> line_list_string=line.split(',');

        ////qDebug()<<i<<" line size = "<<line_list_string.length();


        QList<QByteArray> tmp_string=line_list_string;
        id_str=line_list_string.first().simplified();
        dist_str=line_list_string.last().simplified();
        line_list_string.removeLast();
        chi_str=line_list_string.last().simplified();
        resultsLines.insert(chi_str.toDouble(),tmp_string);
        clamp_mass_str=line_list_string.at(2).simplified();
        clamp_upper_age_str=line_list_string.at(4).simplified();
        compact_mass_fraction_str=line_list_string.at(3).simplified();
        dust_temp_str=line_list_string.at(5).simplified();
        //    //qDebug()<<"id_ "<<id_str<<" dist_ "<<dist_str<<" chi2 "<<chi_str;
        //   //qDebug()<<"clamp_mass_str "<<clamp_mass_str;
        clamp_mass.append(clamp_mass_str.toDouble());
        clamp_upper_age.append(clamp_upper_age_str.toDouble());
        dust_temp.append(dust_temp_str.toDouble());
        compact_mass_fraction.append(compact_mass_fraction_str.toDouble());

        results.insert(chi_str.toDouble(), id_str.toInt());
        if (chi_str.toDouble()<chi2)
        {
            id=id_str.toInt();
            chi2=chi_str.toDouble();
            dist=dist_str.toDouble();
        }
    }
    sedFitValues.insert(2,clamp_mass);
    sedFitValues.insert(4,clamp_upper_age);
    sedFitValues.insert(3,compact_mass_fraction);
    sedFitValues.insert(5,dust_temp);


    int threshold = 5;
    int nFitsResults=results.size()*threshold/100;
    if(nFitsResults<1&&results.size()>=1){
        nFitsResults=1;
    }
    if(nFitsResults<=10){
        nFitsResults=results.size();
    }

    QMap<double, int>::iterator iter;
    QMap <double, QList<QByteArray> >::iterator iterLines;
    iter=results.begin();
    iterLines=resultsLines.begin();
    for(int i=0;i<nFitsResults;i++,iter++,iterLines++){
        //for(int i=0;i<results.size();i++,iter++,iterLines++){
        int new_id=iter.value();
        if(i==0){ // Plot only the best one
            QString query="SELECT * FROM vlkb_compactsources.sed_models where id="+ QString::number(new_id);
            //   //qDebug()<<"query: " << query;
            //   //qDebug()<<"color pre : blue " << Qt::blue;
            new VLKBQuery(query,vtkwin, "model", this, Qt::blue);
        }
        /*
        else{
            if(i<10){
            QString query="SELECT * FROM vlkb_compactsources.sed_models where id="+ QString::number(new_id);
            new VLKBQuery(query,vtkwin, "model", this, Qt::cyan);

            }
        }*/
        QList<QByteArray> line_list_string=iterLines.value();
        ui->resultsTableWidget->insertRow(i);
        for(int j=0;j<columnNames.length();j++){
            if(resultsOutputColumn.value(columnNames.at(j))!=-1){
                ui->resultsTableWidget->setItem(i, j, new QTableWidgetItem(QString(line_list_string.at(resultsOutputColumn.value(columnNames.at(j))).simplified())));
            }
        }
    }
    ui->resultsTableWidget->resizeColumnsToContents();
    ui->resultsTableWidget->resizeRowsToContents();
    loading->close();
}

void SEDVisualizerPlot::loadSedFitOutput(QString filename){
    int id;
    double chi2=99999999999;
    double tmp;
    QString chi_str,id_str,dist_str;
    QMap <double, int> results;
    QMap <double, QList<QByteArray> > resultsLines;

    //   //qDebug()<<"filename: "<<filename;
    QFile file(filename);
    // QFile file(qApp->applicationDirPath()+"/sedfit_output.dat");
    if (!file.open(QIODevice::ReadOnly)) {
        //  //qDebug() << file.errorString();
        return ;
    }
    //read, and skip the first line (header)
    //file.readLine();
    QString header=file.readLine();

    outputSedResults.append(header);
    readSedFitResultsHeader(header);
    //  QStringList wordList;
    int i=0;
    ui->resultsTableWidget->clearContents();
    ui->resultsTableWidget->setRowCount(0);
    ui->resultsTableWidget->setColumnCount(columnNames.size());
    ui->resultsTableWidget->setHorizontalHeaderLabels(columnNames);
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        //wordList.append(line.split(',').first());
        outputSedResults.append(line);



        QList<QByteArray> line_list_string=line.split(',');

        ////qDebug()<<i<<" line size = "<<line_list_string.length();

        /*
            ui->resultsTableWidget->insertRow(i);
            for(int j=0;j<columnNames.length();j++){
                if(resultsOutputColumn.value(columnNames.at(j))!=-1){
                    ui->resultsTableWidget->setItem(i, j, new QTableWidgetItem(QString(line_list_string.at(resultsOutputColumn.value(columnNames.at(j))).simplified())));
                }
            }
            i++;
            */
        QList<QByteArray> tmp_string=line_list_string;
        id_str=line_list_string.first().simplified();
        dist_str=line_list_string.last().simplified();
        line_list_string.removeLast();
        chi_str=line_list_string.last().simplified();
        //  //qDebug()<<"id_ "<<id_str<<" dist_ "<<dist_str<<" chi2 "<<chi_str;
        results.insert(chi_str.toDouble(), id_str.toInt());
        resultsLines.insert(chi_str.toDouble(),tmp_string);
        if (chi_str.toDouble()<chi2)
        {
            id=id_str.toInt();
            chi2=chi_str.toDouble();
            dist=dist_str.toDouble();
        }
    }


    //qDebug()<<"id: "<<id<<" chi2: "<<chi2<<"dist: "<<dist;
    //qDebug()<<"results chi2 :"<<results.firstKey();
    int threshold = 5;
    int nFitsResults=results.size()*threshold/100;
    if(nFitsResults<1&&results.size()>=1){
        nFitsResults=1;
    }
    if(nFitsResults<=10){
        nFitsResults=results.size();
    }
    //qDebug()<<"All Fits :"<<QString::number(results.size());
    //qDebug()<<"results chi2 :"<<results.firstKey()<<" nFitsResults: "<<QString::number(nFitsResults);
    //qDebug()<<"resultsLines chi2 :"<<resultsLines.firstKey();
    //  //qDebug() << wordList;
    QMap<double, int>::iterator iter;
    QMap <double, QList<QByteArray> >::iterator iterLines;
    iter=results.begin();
    iterLines=resultsLines.begin();
    for(int i=0;i<nFitsResults;i++,iter++,iterLines++){
        //for(int i=0;i<results.size();i++,iter++,iterLines++){
        QList<QByteArray> line_list_string=iterLines.value();
        ui->resultsTableWidget->insertRow(i);
        for(int j=0;j<columnNames.length();j++){
            if(resultsOutputColumn.value(columnNames.at(j))!=-1){
                ui->resultsTableWidget->setItem(i, j, new QTableWidgetItem(QString(line_list_string.at(resultsOutputColumn.value(columnNames.at(j))).simplified())));
            }
        }
    }
    ui->resultsTableWidget->resizeColumnsToContents();
}

void SEDVisualizerPlot::loadSedFitThin(QString filename){

    qDebug()<<"leggo thin: "<<filename;
    ui->resultsTableWidget->clearContents();
    ui->resultsTableWidget->setColumnCount(6);
    ui->resultsTableWidget->setRowCount(0);
    ui->resultsTableWidget->setHorizontalHeaderLabels(QStringList() << "fmass" << "dmass" << "ftemp" << "dtemp" << "fbeta" << "lum");
    ui->resultsTableWidget->insertRow(0);

    QFile filepar(filename);
    if (!filepar.open(QIODevice::ReadOnly)) {
        //qDebug() << filepar.errorString();
        //qDebug() << "ERRORE Parametri";
        return ;
    }
    QString line=filepar.readLine();
    outputSedResults.append(line);
    filepar.close();
    QList<QString> line_list_string=line.split(',');
    for(int i=0; i<line_list_string.size();i++){
        ui->resultsTableWidget->setItem(0,i,new QTableWidgetItem(line_list_string.at(i).trimmed()));
    }
    ui->resultsTableWidget->resizeColumnsToContents();
}

void SEDVisualizerPlot::loadSedFitThick(QString filename){

    qDebug()<<"leggo thick "<<filename;

    ui->resultsTableWidget->clearContents();
    ui->resultsTableWidget->setColumnCount(9);
    ui->resultsTableWidget->setRowCount(0);
    ui->resultsTableWidget->setHorizontalHeaderLabels(QStringList() << "mass" << "dmass" << "ftemp" << "dtemp" << "fbeta" << "fl0" << "dlam0" << "sizesec" << "lum");
    ui->resultsTableWidget->insertRow(0);
    QFile filepar(filename);
    if (!filepar.open(QIODevice::ReadOnly)) {
        //qDebug() << filepar.errorString();
        return ;
    }
    QString line=filepar.readLine();
    outputSedResults.append(line);
    filepar.close();
    QList<QString> line_list_string=line.split(',');
    for(int i=0; i<line_list_string.size();i++){
        ui->resultsTableWidget->setItem(0,i,new QTableWidgetItem(line_list_string.at(i).trimmed()));
    }
    ui->resultsTableWidget->resizeColumnsToContents();
}

void SEDVisualizerPlot::setModelFitValue(QVector<QStringList> headerAndValueList, Qt::GlobalColor color)
{
    //qDebug()<<"color set model fit : " << color;
    QFile modelFile(QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(nSED)+"_sedfit_model.dat"));
    if(!modelFile.exists()){
        modelFile.open(QIODevice::WriteOnly);
        QDataStream out(&modelFile);
        out << headerAndValueList;   // serialize the object
    }
    /*
        QVectorIterator<QStringList> i(headerAndValueList);
        int count=0;
        while (i.hasNext()){
            //qDebug()<<"List: "<<QString::number(count);
            QStringList list= i.next();
            foreach (const QString &str, list) {
                //qDebug()<<str;
            }
            count++;
        }
            //modelFile.write()
    }*/
    QVector<double> x, y;
    double scale;
    QCPGraph *graph=ui->customPlot->addGraph();
    QList<SEDPlotPointCustom *> points;
    for( QMap<QString, double>::iterator it = modelFitBands.begin(); it != modelFitBands.end(); ++it)
    {

        //  scale= 1/sqrt(dist);
        scale = 1000/pow(dist,2);
        //scale=1;
        x.append(it.value());
        int index=headerAndValueList.at(0).indexOf(it.key());
        y.append(headerAndValueList.at(1).at(index).toDouble()*scale);
        //y.append(headerAndValueList.at(1).at(index).toDouble());

        SEDPlotPointCustom *cp = new SEDPlotPointCustom(ui->customPlot,3,vtkwin);
        cp->setAntialiased(true);

        //cp->setPos(it.value(), headerAndValueList.at(1).at(index).toDouble()*scale);
        cp->setPos(it.value(), headerAndValueList.at(1).at(index).toDouble()*scale);

        cp->setDesignation("");
        cp->setX(0);
        cp->setY(0);
        cp->setLat(0);
        cp->setLon(0);
        points.push_back(cp);
        //cp->setParent(graph);
        ui->customPlot->addItem(cp);
    }



    //qDebug()<<"x:\n"<<x;
    //qDebug()<<"y:\n"<<y;



    //ui->customPlot->addGraph();

    //qDebug()<<"temporaryMOD : "<<temporaryMOD;

    if(color==Qt::blue || doubleClicked){
        addNewSEDCheckBox("Theoretical Fit");
        //qDebug()<<"Adding points of SED "<<QString::number(nSED-1);
        sedGraphPoints.insert(nSED-1,points);
        sedGraphs.push_back(graph);
        doubleClicked=false;
    }

    if(temporaryMOD){
        //qDebug()<<"Temporary MOD";
        temporaryGraph=graph;
        temporaryGraphPoints=points;
    }

    ui->customPlot->graph()->setData(x, y);
    // ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 6));
    ui->customPlot->graph()->setPen(QPen(color));
    ui->customPlot->graph()->setScatterStyle( QCPScatterStyle::ssNone);
    ui->customPlot->replot();
    this->setFocus();
    this->activateWindow();
}


void SEDVisualizerPlot::on_actionScreenshot_triggered()
{
    QPixmap qPixMap = QPixmap::grabWidget(this);  // *this* is window pointer, the snippet     is in the mainwindow.cpp file

    QImage qImage = qPixMap.toImage();

    QString fileName = QFileDialog::getSaveFileName(this,"Save screenshot", "", ".png");
    //qDebug()<<"pre "<<fileName;
    if(!fileName.endsWith(".png",Qt::CaseInsensitive) )
        fileName.append(".png");
    bool b = qImage.save(fileName);
}

void SEDVisualizerPlot::on_actionCollapse_triggered()
{

    QPen pen;
    pen.setStyle(Qt::DashLine);
    pen.setColor(Qt::lightGray);

    for(int i=0;i<originalGraphs.size();i++)
    {
        //originalGraphs.push_back(ui->customPlot->graph(i));
        originalGraphs.at(i)->setPen(pen);
    }

    SED *coll_sed= new SED();
    QVector<double> x, y;
    SEDNode *old_tmp_node;
    SEDNode *tmp_node;

    int cnt=0;
    double j;
    foreach( j, all_sed_node.uniqueKeys() )
    {
        //qDebug() <<"j: "<< j;
        QList<SEDNode*> node_list_tmp=all_sed_node.values(j);
        double flux_sum=0;
        double err_flux_sum=0;

        if(cnt!=0)
        {
            old_tmp_node=tmp_node;
        }

        tmp_node=new SEDNode();

        for (int z=0;z<node_list_tmp.count(); z++ )
        {
            //qDebug() <<"\t"<< node_list_tmp.at(z)->getDesignation();
            flux_sum+=node_list_tmp.at(z)->getFlux();
            err_flux_sum+=node_list_tmp.at(z)->getErrFlux();
        }

        tmp_node->setDesignation("");
        tmp_node->setFlux(flux_sum);
        tmp_node->setErrFlux(err_flux_sum);
        tmp_node->setWavelength(j);

        if(cnt!=0)
        {
            old_tmp_node->setParent(tmp_node);
            tmp_node->setChild(old_tmp_node);
        }



        x.append(j);
        y.append(flux_sum);


        SEDPlotPointCustom *cp = new SEDPlotPointCustom(ui->customPlot,3,vtkwin);
        cp->setAntialiased(true);
        cp->setPos(j, flux_sum);
        cp->setDesignation("");
        cp->setX(0);
        cp->setY(0);
        cp->setLat(0);
        cp->setLon(0);
        cp->setErrorFlux(err_flux_sum);
        cp->setNode(tmp_node);
        collapsedGraphPoints.push_back(cp);
        ui->customPlot->addItem(cp);

        cnt++;
    }

    //qDebug()<<"x:\n"<<x;
    //qDebug()<<"y:\n"<<y;


    coll_sed->setRootNode(tmp_node);

    collapsedGraph=ui->customPlot->addGraph();
    ui->customPlot->graph()->setData(x, y);
    ui->customPlot->graph()->setPen(QPen(Qt::red)); // line color red for second graph
    ui->customPlot->graph()->setScatterStyle( QCPScatterStyle::ssNone);

    ui->customPlot->replot();

    //qDebug()<<sed_list.count();
    sed_list.insert(0,coll_sed);
    //qDebug()<<sed_list.count();


}


void SEDVisualizerPlot::on_TheoreticalLocaleFit_triggered()
{
    sedFitInputFflag = "]";
    sedFitInputF= "]";
    sedFitInputW= "]";

    sedFitInputErrF= "]";


    sedFitInput.clear();

    bool validFit=false;

    //  if(multiSelectMOD){
    validFit=prepareSelectedInputForSedFit();
    /*}else{
        validFit=prepareInputForSedFit(sed_list.at(0)->getRootNode());
    }
*/
    if(validFit)
    {
        sedFitInputF ="["+sedFitInputF ;
        sedFitInputW ="["+sedFitInputW;
        sedFitInputFflag = "["+sedFitInputFflag;
        sedFitInputErrF ="["+sedFitInputErrF ;

        loading = new LoadingWidget();
        loading->init();
        loading->setFileName("Executing vialactea_tap_sedfit");
        loading ->show();
        loading->activateWindow();
        loading->setFocus();

        ui->outputTextBrowser->setText("");

        process= new QProcess();

        process->setProcessChannelMode(QProcess::MergedChannels);

        //qDebug()<<idlPath+" -e \"sedpar=vialactea_tap_sedfit_v7("+sedFitInputW+","+sedFitInputF+","+sedFitInputErrF+","+sedFitInputFflag+","+ui->distTheoLineEdit->text()+","+ui->prefilterLineEdit->text()+",sed_weights=["+ui->mid_irLineEdit->text()+","+ui->far_irLineEdit->text()+","+ui->submmLineEdit->text()+"],use_wave="+sedFitInputW+",outdir='"+QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(nSED))+"_',delta_chi2="+ui->delta_chi2_lineEdit->text()+")\" ";

        connect(process,SIGNAL(readyReadStandardError()),this,SLOT(onReadyReadStdOutput()));
        connect(process,SIGNAL(readyReadStandardOutput()),this,SLOT(onReadyReadStdOutput()));
        connect(process,SIGNAL(finished(int)),this,SLOT(finishedTheoreticalLocaleFit()));

        process->start (idlPath+" -e \"sedpar=vialactea_tap_sedfit_v7("+sedFitInputW+","+sedFitInputF+","+sedFitInputErrF+","+sedFitInputFflag+","+ui->distTheoLineEdit->text()+","+ui->prefilterLineEdit->text()+",sed_weights=["+ui->mid_irLineEdit->text()+","+ui->far_irLineEdit->text()+","+ui->submmLineEdit->text()+"],use_wave="+sedFitInputW+",outdir='"+QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(nSED))+"_',delta_chi2="+ui->delta_chi2_lineEdit->text()+")\" ");
    }
}

void SEDVisualizerPlot::finishedTheoreticalLocaleFit()
{
    /*
    QString output(process.readAll());
    //qDebug()<<output;
    outputSedLog=output;

    ui->outputTextBrowser->setText(output);
  */
    readSedFitOutput(QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(nSED)+"_sedfit_output.dat"));

    //addNewSEDCheckBox("Theoretical Fit");
    this->setFocus();
    this->activateWindow();
}


void SEDVisualizerPlot::finishedTheoreticalRemoteFit()
{
    QDir dir (QApplication::applicationDirPath());

    dir.rename("sedfit_output.dat",QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(nSED)+"_sedfit_output.dat"));
    readSedFitOutput(QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(nSED)+"_sedfit_output.dat"));

    this->setFocus();
    this->activateWindow();
}

void SEDVisualizerPlot::onReadyReadStdOutput()
{
    //  process.setReadChannel(QProcess::readAllStandardError());
    QTextStream stream(process);
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        // extract progress info from line and etc.
        ui->outputTextBrowser->append(line);
    }
}

//Add a new sed checkbox to the group of SED
void SEDVisualizerPlot::addNewSEDCheckBox(QString label, int nSED){
    //qDebug()<<"Adding new SED Check Box "+QString::number(nSED);
    QSignalMapper* mapper = new QSignalMapper(this);
    ui->generatedSedBox->setVisible(true);
    QCheckBox *newSed = new QCheckBox(label+" "+QString::number(nSED+1));
    newSed->setChecked(true);
    connect(newSed, SIGNAL(stateChanged(int)),mapper, SLOT(map()));
    mapper->setMapping(newSed, nSED);
    connect(mapper, SIGNAL(mapped(int)), this, SLOT(on_SEDCheckboxClicked(int)));
    ui->generatedSedBox->layout()->addWidget(newSed);
    //nSED++;
}


//Add a new sed checkbox to the group of SED
void SEDVisualizerPlot::addNewSEDCheckBox(QString label){
    //qDebug()<<"Adding new SED Check Box "+QString::number(nSED);
    QSignalMapper* mapper = new QSignalMapper(this);
    ui->generatedSedBox->setVisible(true);
    QCheckBox *newSed = new QCheckBox(QString::number(nSED+1)+":"+label);
    newSed->setChecked(true);
    connect(newSed, SIGNAL(stateChanged(int)),mapper, SLOT(map()));
    mapper->setMapping(newSed, nSED);
    connect(mapper, SIGNAL(mapped(int)), this, SLOT(on_SEDCheckboxClicked(int)));
    ui->generatedSedBox->layout()->addWidget(newSed);
    nSED++;
    plottedSedLabels.append(label);
}

void SEDVisualizerPlot::on_SEDCheckboxClicked(int sedN)
{
    qDebug()<<"processing checkbox "<<sedN;
    qDebug()<<"sedGraphs.size() "<<QString::number(sedGraphs.size());
    QCPGraph * graph=sedGraphs.at(sedN);
    if(graph->visible()){
        //qDebug()<<"Remove Graph "<<sedGraphPoints.contains(sedN);
        graph->setVisible(false);
        //temporaryGraph->setVisible(false);
        //qDebug()<<"graph visibility "<<graph->visible();
        if(sedGraphPoints.contains(sedN)){
            QList<SEDPlotPointCustom *> points=sedGraphPoints.value(sedN);
            for (int i = 0; i < points.size(); ++i)
            {
                SEDPlotPointCustom *cp;
                cp = points.at(i);
                cp->setVisible(false);
                //qDebug()<<"points visibility "<<cp->visible();
            }
        }
        //        ui->customPlot->removePlottable(graph);
        ui->resultsTableWidget->clearContents();
        ui->resultsTableWidget->setRowCount(0);
    }else{
        ////qDebug()<<"Add Graph";
        //ui->customPlot->addGraph();
        //ui->customPlot->graph();
        graph->setVisible(true);
        //qDebug()<<"graph visibility "<<graph->visible();
        if(sedGraphPoints.contains(sedN)){ // This is a theoretical SED fit
            QList<SEDPlotPointCustom *> points=sedGraphPoints.value(sedN);
            for (int i = 0; i < points.size(); ++i)
            {
                SEDPlotPointCustom *cp;
                cp = points.at(i);
                cp->setVisible(true);
            }
            loadSedFitOutput(QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(sedN)+"_sedfit_output.dat"));
        }else{ // This is a Thin or Thick SED fit
            QString label=plottedSedLabels.at(sedN);
            if(label.contains("Thick")){
                loadSedFitThick(QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(sedN)+"_thickfile.csv.par"));
            }else{
                loadSedFitThin(QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(sedN)+"_thinfile.csv.par"));

            }
        }

    }
    ui->customPlot->replot();
}


void SEDVisualizerPlot::on_ThinLocalFit_triggered()
{

    sedFitInputF= "]";
    sedFitInputErrF= "]";
    sedFitInputW= "]";

    sedFitInput.clear();
    /*  if(multiSelectMOD){
        prepareSelectedInputForSedFit();
    }else{
        prepareInputForSedFit(sed_list.at(0)->getRootNode());
    }
    */
    bool validFit=false;

    validFit=prepareSelectedInputForSedFit();

    if(validFit)
    {


        sedFitInputF ="["+sedFitInputF ;
        sedFitInputErrF ="["+sedFitInputErrF ;
        sedFitInputW ="["+sedFitInputW;


        QString f=sedFitInputW.mid(1,sedFitInputW.length()-2);

        QStringList wave = f.split(",");

        QSignalMapper* mapper = new QSignalMapper(this);

        sd_thin = new SedFitGrid_thin(this);
        sd_thin->ui->distLineEdit->setText(ui->distTheoLineEdit->text());
        sedFitInputUlimitString="[";

        for(int i=0;i<wave.length();i++)
        {
            //qDebug()<<wave.at(i);
            sedFitInputUlimit.insert(i,"0");

            sedFitInputUlimitString+="0";
            if(i<wave.size()-1)
                sedFitInputUlimitString+=",";

            int row= sd_thin->ui->tableWidget->model()->rowCount();
            sd_thin->ui->tableWidget->insertRow(row);

            QCheckBox  *cb1= new QCheckBox();
            cb1->setChecked(false);
            sd_thin->ui->tableWidget->setCellWidget(row,1,cb1);

            connect(cb1, SIGNAL(stateChanged(int)),mapper, SLOT(map()));
            mapper->setMapping(cb1, row);

            QTableWidgetItem *item_1 = new QTableWidgetItem();
            item_1->setText(wave.at(i));

            sd_thin->ui->tableWidget->setItem(row,0,item_1);

        }
        sedFitInputUlimitString+="]";
        connect(mapper, SIGNAL(mapped(int)), this, SLOT(checkboxClicked(int)));

        //qDebug()<<sedFitInputUlimitString;



        sd_thin->show();

        connect( sd_thin->ui->pushButton ,SIGNAL(clicked()),this,SLOT(doThinLocalFit()));
    }
}


void SEDVisualizerPlot::checkboxClicked(int cb)
{

    sedFitInputUlimit[cb].compare("0")==0? sedFitInputUlimit[cb]="1" : sedFitInputUlimit[cb]="0";


    sedFitInputUlimitString="[";

    for(int i=0;i<sedFitInputUlimit.size();i++)
    {
        sedFitInputUlimitString+=sedFitInputUlimit[i];
        if(i<sedFitInputUlimit.size()-1)
            sedFitInputUlimitString+=",";
    }
    sedFitInputUlimitString+="]";


}

void SEDVisualizerPlot::doThinLocalFit()
{
    sd_thin->close();

    ui->outputTextBrowser->setText("");

    process= new QProcess();
    connect(process,SIGNAL(readyReadStandardError()),this,SLOT(onReadyReadStdOutput()));
    connect(process,SIGNAL(readyReadStandardOutput()),this,SLOT(onReadyReadStdOutput()));
    connect(process,SIGNAL(finished(int)),this,SLOT(finishedThinLocalFit()));

    process->setProcessChannelMode(QProcess::MergedChannels);

    QString mrange="["+ sd_thin->ui->minMassLineEdit->text()+", "+sd_thin->ui->maxMassLineEdit->text()+", "+ sd_thin->ui->stepMassLineEdit->text() +"]";
    QString trange="["+ sd_thin->ui->tempMinLineEdit->text()+", "+sd_thin->ui->tempMaxLineEdit->text()+", "+ sd_thin->ui->tempStepLineEdit->text() +"]";
    QString brange="["+ sd_thin->ui->betaMinLineEdit->text()+", "+sd_thin->ui->betaMaxLineEdit->text()+", "+ sd_thin->ui->betaStepLineEdit->text() +"]";
    QString srefRange="["+ sd_thin->ui->srefOpacityLineEdit->text()+", "+ sd_thin->ui->srefWavelengthLineEdit->text() +"]";

    qDebug()<<idlPath<<" -e \"sedfitgrid_engine_thin_vialactea,"<<sedFitInputW<<", "<<sedFitInputF<<","<<mrange<<","<<trange<<" ,"<<brange<<" ,"<<sd_thin->ui->distLineEdit->text()<<","<<srefRange<<",lambdatn,flussotn,mtn,ttn,btn,ltn,dmtn,dttn,errorbars="<<sedFitInputErrF<<",ulimit="<<sedFitInputUlimitString<<",printfile= '"<<QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(nSED)+"_thinfile.csv")<<"'";
    process->start (idlPath+" -e \"sedfitgrid_engine_thin_vialactea,"+sedFitInputW+","+sedFitInputF+","+mrange+","+trange+" ,"+brange+" ,"+sd_thin->ui->distLineEdit->text()+","+srefRange+",lambdatn,flussotn,mtn,ttn,btn,ltn,dmtn,dttn,errorbars="+sedFitInputErrF+",ulimit="+sedFitInputUlimitString+",printfile= '"+QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(nSED)+"_thinfile.csv")+"'");

    // process.waitForFinished(); // sets current thread to sleep and waits for process end
}

void SEDVisualizerPlot::doThinRemoteFit()
{
    sd_thin->close();

    ui->outputTextBrowser->setText("");

    process= new QProcess();
    connect(process,SIGNAL(readyReadStandardError()),this,SLOT(onReadyReadStdOutput()));
    connect(process,SIGNAL(readyReadStandardOutput()),this,SLOT(onReadyReadStdOutput()));
    connect(process,SIGNAL(finished(int)),this,SLOT(finishedThinRemoteFit()));

    process->setProcessChannelMode(QProcess::MergedChannels);

    QString mrange="["+ sd_thin->ui->minMassLineEdit->text()+", "+sd_thin->ui->maxMassLineEdit->text()+", "+ sd_thin->ui->stepMassLineEdit->text() +"]";
    QString trange="["+ sd_thin->ui->tempMinLineEdit->text()+", "+sd_thin->ui->tempMaxLineEdit->text()+", "+ sd_thin->ui->tempStepLineEdit->text() +"]";
    QString brange="["+ sd_thin->ui->betaMinLineEdit->text()+", "+sd_thin->ui->betaMaxLineEdit->text()+", "+ sd_thin->ui->betaStepLineEdit->text() +"]";
    QString srefRange="["+ sd_thin->ui->srefOpacityLineEdit->text()+", "+ sd_thin->ui->srefWavelengthLineEdit->text() +"]";

    ////qDebug()<<"java -jar  \"sedfitgrid_engine_thin_vialactea,"<<sedFitInputW<<", "<<sedFitInputF<<","<<mrange<<","<<trange<<" ,"<<brange<<" ,"<<sd_thin->ui->distLineEdit->text()<<","<<srefRange<<",lambdatn,flussotn,mtn,ttn,btn,ltn,dmtn,dttn,errorbars="<<sedFitInputErrF<<",ulimit="<<sedFitInputUlimitString<<",printfile= '"<<QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(nSED)+"_thinfile.csv")<<"'";
    //process->start (idlPath+" -e \"sedfitgrid_engine_thin_vialactea,"+sedFitInputW+","+sedFitInputF+","+mrange+","+trange+" ,"+brange+" ,"+sd_thin->ui->distLineEdit->text()+","+srefRange+",lambdatn,flussotn,mtn,ttn,btn,ltn,dmtn,dttn,errorbars="+sedFitInputErrF+",ulimit="+sedFitInputUlimitString+",printfile= '"+QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(nSED)+"_thinfile.csv")+"'");




    qDebug()<<" java -jar "<<QApplication::applicationDirPath().append("/vsh-ws-client.jar")<<" \"sedfitgrid_engine_thin_vialactea,"+sedFitInputW+","+sedFitInputF+","+mrange+","+trange+","+brange+","+sd_thin->ui->distLineEdit->text()+","+srefRange+",lambdatn,flussotn,mtn,ttn,btn,ltn,dmtn,dttn,errorbars="+sedFitInputErrF+",ulimit="+sedFitInputUlimitString+",printfile= 'sedfit_output.dat'";

    process->setWorkingDirectory(QApplication::applicationDirPath());

    //qDebug()<<"DIR: "<<process->workingDirectory();
    //qDebug() << "App path : " << qApp->applicationDirPath();
    //qDebug()<<"QDir::currentPath() :"<<QDir::currentPath() ;

    process->start ("java -jar "+QApplication::applicationDirPath().append("/vsh-ws-client.jar")+" \"sedfitgrid_engine_thin_vialactea,"+sedFitInputW+","+sedFitInputF+","+mrange+","+trange+","+brange+","+sd_thin->ui->distLineEdit->text()+","+srefRange+",lambdatn,flussotn,mtn,ttn,btn,ltn,dmtn,dttn,errorbars="+sedFitInputErrF+",ulimit="+sedFitInputUlimitString+",printfile= 'sedfit_output.dat'\" ");
    qDebug()<<"sedfitgrid_engine_thin_vialactea,"+sedFitInputW+","+sedFitInputF+","+mrange+","+trange+","+brange+","+sd_thin->ui->distLineEdit->text()+","+srefRange+",lambdatn,flussotn,mtn,ttn,btn,ltn,dmtn,dttn,errorbars="+sedFitInputErrF+",ulimit="+sedFitInputUlimitString+",printfile= 'sedfit_output.dat'\"";


    ui->outputTextBrowser->append("java -jar "+QApplication::applicationDirPath().append("/vsh-ws-client.jar")+" \"sedfitgrid_engine_thin_vialactea,"+sedFitInputW+","+sedFitInputF+","+mrange+","+trange+","+brange+","+sd_thin->ui->distLineEdit->text()+","+srefRange+",lambdatn,flussotn,mtn,ttn,btn,ltn,dmtn,dttn,errorbars="+sedFitInputErrF+",ulimit="+sedFitInputUlimitString+",printfile= 'sedfit_output.dat'\" ");
    ui->outputTextBrowser->append("\nDIR: "+process->workingDirectory());
    ui->outputTextBrowser->append("\nApp path : " +qApp->applicationDirPath());
    ui->outputTextBrowser->append("\nQDir::currentPath() :"+QDir::currentPath());


    // process.waitForFinished(); // sets current thread to sleep and waits for process end
}


void SEDVisualizerPlot::doThickRemoteFit()
{


    sd_thick->close();
    ui->outputTextBrowser->setText("");

    process= new QProcess();
    connect(process,SIGNAL(readyReadStandardError()),this,SLOT(onReadyReadStdOutput()));
    connect(process,SIGNAL(readyReadStandardOutput()),this,SLOT(onReadyReadStdOutput()));
    connect(process,SIGNAL(finished(int)),this,SLOT(finishedThickRemoteFit()));

    process->setProcessChannelMode(QProcess::MergedChannels);
    QString trange="["+ sd_thick->ui->tempMinLineEdit->text()+", "+sd_thick->ui->tempMaxLineEdit->text()+", "+ sd_thick->ui->tempStepLineEdit->text() +"]";
    QString brange="["+ sd_thick->ui->betaMinLineEdit->text()+", "+sd_thick->ui->betaMaxLineEdit->text()+", "+ sd_thick->ui->betaStepLineEdit->text() +"]";
    QString l0range="["+ sd_thick->ui->minLambda_0LineEdit->text()+", "+sd_thick->ui->maxLambda_0LineEdit->text()+", "+ sd_thick->ui->stepLambda_0LineEdit->text() +"]";
    QString sfactrange="["+ sd_thick->ui->scaleMinLineEdit->text()+", "+sd_thick->ui->scaleMaxLineEdit->text()+", "+ sd_thick->ui->scaleStepLineEdit->text() +"]";
    QString srefRange="["+ sd_thick->ui->srefOpacityLineEdit->text()+", "+ sd_thick->ui->srefWavelengthLineEdit->text() +"]";

    // process->start (idlPath+" -e \"sedfitgrid_engine_thick_vialactea,"+sedFitInputW+","+sedFitInputF+","+sd_thick->ui->sizeLineEdit->text()+","+trange+" ,"+brange+" ,"+l0range+", "+sfactrange+","+sd_thick->ui->distLineEdit->text()+","+srefRange+",lambdagb,flussogb,mtk,ttk,btk,l0,sizesec,ltk,dmtk,dtk,dl0,errorbars="+sedFitInputErrF+",ulimit="+sedFitInputUlimitString+",printfile= '"+QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(nSED)+"_thickfile.csv")+"'");

    process->setWorkingDirectory(QApplication::applicationDirPath());
    process->start ("java -jar "+QApplication::applicationDirPath().append("/vsh-ws-client.jar")+" \"sedfitgrid_engine_thick_vialactea,"+sedFitInputW+","+sedFitInputF+","+sd_thick->ui->sizeLineEdit->text()+","+trange+" ,"+brange+" ,"+l0range+", "+sfactrange+","+sd_thick->ui->distLineEdit->text()+","+srefRange+",lambdagb,flussogb,mtk,ttk,btk,l0,sizesec,ltk,dmtk,dtk,dl0,errorbars="+sedFitInputErrF+",ulimit="+sedFitInputUlimitString+",printfile= 'sedfit_output.dat'\" ");

    qDebug()<<"sedfitgrid_engine_thick_vialactea,"+sedFitInputW+","+sedFitInputF+","+sd_thick->ui->sizeLineEdit->text()+","+trange+" ,"+brange+" ,"+l0range+", "+sfactrange+","+sd_thick->ui->distLineEdit->text()+","+srefRange+",lambdagb,flussogb,mtk,ttk,btk,l0,sizesec,ltk,dmtk,dtk,dl0,errorbars="+sedFitInputErrF+",ulimit="+sedFitInputUlimitString+",printfile= 'sedfit_output.dat'\" ";



}

void SEDVisualizerPlot::finishedThinLocalFit()
{
    bool res = readThinFit(QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(nSED)+"_thinfile.csv"));
    if(res)
        addNewSEDCheckBox("Thin Fit");
    else
        QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("No results."));

}

bool SEDVisualizerPlot::readThinFit(QString resultPath){
    QFile file(resultPath);
    if (!file.open(QIODevice::ReadOnly)) {
        //qDebug() << file.errorString();
        //qDebug() << "ERRORE Disegno FIT";
        return false ;
    }

    QVector<double> x, y;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();

        x.append( line.split(',').first().simplified().toDouble() );
        y.append( line.split(',').last().simplified().toDouble() );

    }

    ui->customPlot->addGraph();
    ui->customPlot->graph()->setData(x, y);
    ui->customPlot->graph()->setScatterStyle( QCPScatterStyle::ssNone);
    sedGraphs.push_back(ui->customPlot->graph());
    ui->customPlot->replot();

    ui->resultsTableWidget->clearContents();
    ui->resultsTableWidget->setColumnCount(6);
    ui->resultsTableWidget->setRowCount(0);
    ui->resultsTableWidget->setHorizontalHeaderLabels(QStringList() << "fmass" << "dmass" << "ftemp" << "dtemp" << "fbeta" << "lum");
    ui->resultsTableWidget->insertRow(0);

    QFile filepar(resultPath+".par");
    if (!filepar.open(QIODevice::ReadOnly)) {
        //qDebug() << file.errorString();
        //qDebug() << "ERRORE Parametri";
        return false ;
    }
    QString line=filepar.readLine();
    filepar.close();
    QList<QString> line_list_string=line.split(',');
    for(int i=0; i<line_list_string.size();i++){
        ui->resultsTableWidget->setItem(0,i,new QTableWidgetItem(line_list_string.at(i).trimmed()));
    }
    ui->resultsTableWidget->resizeColumnsToContents();
    return true;
}

bool SEDVisualizerPlot::readThickFit(QString resultPath){
    QFile file(resultPath);
    if (!file.open(QIODevice::ReadOnly)) {
        //qDebug() << file.errorString();
        //qDebug() << "ERRORE Disegno FIT";
        return false;
    }

    QVector<double> x, y;

    while (!file.atEnd()) {
        QByteArray line = file.readLine();

        x.append( line.split(',').first().simplified().toDouble() );
        y.append( line.split(',').last().simplified().toDouble() );

    }

    ui->customPlot->addGraph();
    ui->customPlot->graph()->setData(x, y);
    ui->customPlot->graph()->setScatterStyle( QCPScatterStyle::ssNone);
    sedGraphs.push_back(ui->customPlot->graph());
    ui->customPlot->replot();

    ui->resultsTableWidget->clearContents();
    ui->resultsTableWidget->setColumnCount(9);
    ui->resultsTableWidget->setRowCount(0);
    ui->resultsTableWidget->setHorizontalHeaderLabels(QStringList() << "mass" << "dmass" << "ftemp" << "dtemp" << "fbeta" << "fl0" << "dlam0" << "sizesec" << "lum");
    ui->resultsTableWidget->insertRow(0);
    QFile filepar(resultPath+".par");
    if (!filepar.open(QIODevice::ReadOnly)) {
        //qDebug() << filepar.errorString();
        return false ;
    }
    QString line=filepar.readLine();
    filepar.close();
    QList<QString> line_list_string=line.split(',');
    for(int i=0; i<line_list_string.size();i++){
        ui->resultsTableWidget->setItem(0,i,new QTableWidgetItem(line_list_string.at(i).trimmed()));
    }
    ui->resultsTableWidget->resizeColumnsToContents();

    return true;

}

void SEDVisualizerPlot::doThickLocalFit()
{

    sd_thick->close();
    ui->outputTextBrowser->setText("");

    process= new QProcess();
    connect(process,SIGNAL(readyReadStandardError()),this,SLOT(onReadyReadStdOutput()));
    connect(process,SIGNAL(readyReadStandardOutput()),this,SLOT(onReadyReadStdOutput()));
    connect(process,SIGNAL(finished(int)),this,SLOT(finishedThickLocalFit()));

    process->setProcessChannelMode(QProcess::MergedChannels);
    QString trange="["+ sd_thick->ui->tempMinLineEdit->text()+", "+sd_thick->ui->tempMaxLineEdit->text()+", "+ sd_thick->ui->tempStepLineEdit->text() +"]";
    QString brange="["+ sd_thick->ui->betaMinLineEdit->text()+", "+sd_thick->ui->betaMaxLineEdit->text()+", "+ sd_thick->ui->betaStepLineEdit->text() +"]";
    QString l0range="["+ sd_thick->ui->minLambda_0LineEdit->text()+", "+sd_thick->ui->maxLambda_0LineEdit->text()+", "+ sd_thick->ui->stepLambda_0LineEdit->text() +"]";
    QString sfactrange="["+ sd_thick->ui->scaleMinLineEdit->text()+", "+sd_thick->ui->scaleMaxLineEdit->text()+", "+ sd_thick->ui->scaleStepLineEdit->text() +"]";
    QString srefRange="["+ sd_thick->ui->srefOpacityLineEdit->text()+", "+ sd_thick->ui->srefWavelengthLineEdit->text() +"]";

    //qDebug()<<"sedfitgrid_engine_thick_vialactea,"+sedFitInputW+","+sedFitInputF+","+sd_thick->ui->sizeLineEdit->text()+","+trange+" ,"+brange+" ,"+l0range+", "+sfactrange+","+sd_thick->ui->distLineEdit->text()+","+srefRange+",lambdagb,flussogb,mtk,ttk,btk,l0,sizesec,ltk,dmtk,dtk,dl0,errorbars="+sedFitInputErrF+",ulimit="+sedFitInputUlimitString;

    process->start (idlPath+" -e \"sedfitgrid_engine_thick_vialactea,"+sedFitInputW+","+sedFitInputF+","+sd_thick->ui->sizeLineEdit->text()+","+trange+" ,"+brange+" ,"+l0range+", "+sfactrange+","+sd_thick->ui->distLineEdit->text()+","+srefRange+",lambdagb,flussogb,mtk,ttk,btk,l0,sizesec,ltk,dmtk,dtk,dl0,errorbars="+sedFitInputErrF+",ulimit="+sedFitInputUlimitString+",printfile= '"+QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(nSED)+"_thickfile.csv")+"'");



}




void SEDVisualizerPlot::finishedThinRemoteFit()
{

    QDir dir (QApplication::applicationDirPath());

    dir.rename("sedfit_output.dat",QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(nSED)+"_thinfile.csv"));
    dir.rename("sedfit_output.dat.par",QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(nSED)+"_thinfile.csv.par"));


    // readSedFitOutput(QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(nSED)+"_sedfit_output.dat"));


    bool res= readThickFit(QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(nSED)+"_thinfile.csv"));

    if (res)
        addNewSEDCheckBox("Thin Fit");
    else
        QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("No results."));


}

void SEDVisualizerPlot::finishedThickRemoteFit()
{


    QDir dir (QApplication::applicationDirPath());

    dir.rename("sedfit_output.dat",QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(nSED)+"_thickfile.csv"));
    dir.rename("sedfit_output.dat.par",QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(nSED)+"_thickfile.csv.par"));


    bool res = readThickFit(QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(nSED)+"_thickfile.csv"));

    if (res)
        addNewSEDCheckBox("Thick Fit");
    else
        QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("No results."));

}

void SEDVisualizerPlot::finishedThickLocalFit()
{

    bool res = readThickFit(QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/SED"+QString::number(nSED)+"_thickfile.csv"));
    if (res)
        addNewSEDCheckBox("Thick Fit");
    else
        QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("No results."));


}


void SEDVisualizerPlot::on_ThickLocalFit_triggered()
{
    sedFitInputF= "]";
    sedFitInputErrF= "]";
    sedFitInputW= "]";

    sedFitInput.clear();
    /*  if(multiSelectMOD){
        prepareSelectedInputForSedFit();
    }else{
        prepareInputForSedFit(sed_list.at(0)->getRootNode());
    }
    */

    bool validFit=false;

    validFit=prepareSelectedInputForSedFit();

    if(validFit)
    {


        sedFitInputF ="["+sedFitInputF ;
        sedFitInputErrF ="["+sedFitInputErrF ;
        sedFitInputW ="["+sedFitInputW;

        //qDebug() << "f: " <<sedFitInputF;
        //qDebug() << "e_f: " <<sedFitInputErrF;
        //qDebug() << "w: " <<sedFitInputW;


        QString f=sedFitInputW.mid(1,sedFitInputW.length()-2);

        QStringList wave = f.split(",");

        QSignalMapper* mapper = new QSignalMapper(this);

        sd_thick = new SedFitgrid_thick(this);
        sd_thick->ui->distLineEdit->setText(ui->distTheoLineEdit->text());

        sedFitInputUlimitString="[";

        for(int i=0;i<wave.length();i++)
        {
            //qDebug()<<wave.at(i);
            sedFitInputUlimit.insert(i,"0");

            sedFitInputUlimitString+="0";
            if(i<wave.size()-1)
                sedFitInputUlimitString+=",";

            int row= sd_thick->ui->tableWidget->model()->rowCount();
            sd_thick->ui->tableWidget->insertRow(row);

            QCheckBox  *cb1= new QCheckBox();
            cb1->setChecked(false);
            sd_thick->ui->tableWidget->setCellWidget(row,1,cb1);

            connect(cb1, SIGNAL(stateChanged(int)),mapper, SLOT(map()));
            mapper->setMapping(cb1, row);

            QTableWidgetItem *item_1 = new QTableWidgetItem();
            item_1->setText(wave.at(i));

            sd_thick->ui->tableWidget->setItem(row,0,item_1);

        }
        sedFitInputUlimitString+="]";
        connect(mapper, SIGNAL(mapped(int)), this, SLOT(checkboxClicked(int)));

        sd_thick->show();
        connect( sd_thick->ui->pushButton ,SIGNAL(clicked()),this,SLOT(doThickLocalFit()));
    }
}

void SEDVisualizerPlot::executeRemoteCall(QString sedFitInputW, QString sedFitInputF,QString sedFitInputErrF, QString sedFitInputFflag, QMap<double,double> sedFitInput){
    //qDebug()<<"Sono it thread executeRemoteCall"<<QThread::currentThread();

    QString filename="sed_fit/execute.bin";
    QFile file( filename );

    if ( file.open(QIODevice::ReadWrite | QIODevice::Truncate) )
    {
        QTextStream stream( &file );
        stream.reset();
        stream <<"#!/bin/bash"<<endl;
        stream <<"unzip scripts.zip"<<endl;
        //stream <<"/usr/local/bin/idl -e \"sedpar=vialactea_tap_sedfit("+sedFitInputW+","+sedFitInputF+","+sedFitInputFflag+",1000.,0.2,sed_weights=[1.,1.,1.],use_wave="+QString::number(sedFitInput.keys()[0])+")\"" << endl;
        stream <<"/usr/local/bin/idl -e \"sedpar=vialactea_tap_sedfit_v3("+sedFitInputW+","+sedFitInputF+","+sedFitInputErrF+","+sedFitInputFflag+",2000.,0.8,sed_weights=[1.,1.,1.],use_wave="+sedFitInputW+")\" &> log.dat"<< endl;
        stream <<"zip output.zip *dat"<<endl;
    }


    //qDebug()<<"/usr/local/bin/idl -e \"sedpar=vialactea_tap_sedfit_v3("+sedFitInputW+","+sedFitInputF+","+sedFitInputErrF+","+sedFitInputFflag+",2000.,0.8,sed_weights=[1.,1.,1.],use_wave="+sedFitInputW+")\"  &> log.dat"<< endl;


    QProcess process_zip;
    process_zip.start ("zip -j sed_fit/inputs.zip sed_fit/scripts.zip sed_fit/execute.bin ");
    process_zip.waitForFinished(); // sets current thread to sleep and waits for process end

    //curl -k -F m=submit -F pass=hp39A11  -F wfdesc=@workflow.xml -F inputzip=@inputs.zip -F portmapping=@portmapping.txt -F certs=@certs.zip http://via-lactea-sg00.iaps.inaf.it:8080/wspgrade/RemoteServlet
    QProcess process;
    process.start ("curl -k -F m=submit -F pass=hp39A11  -F wfdesc=@sed_fit/workflow.xml -F inputzip=@sed_fit/inputs.zip -F portmapping=@sed_fit/portmapping.txt -F certs=@sed_fit/certs.zip http://via-lactea-sg00.iaps.inaf.it:8080/wspgrade/RemoteServlet");
    //qDebug()<<"curl -k -F m=submit -F pass=hp39A11  -F wfdesc=@sed_fit/workflow.xml -F inputzip=@sed_fit/inputs.zip -F portmapping=@sed_fit/portmapping.txt -F certs=@sed_fit/certs.zip http://via-lactea-sg00.iaps.inaf.it:8080/wspgrade/RemoteServlet";
    process.waitForFinished(); // sets current thread to sleep and waits for process end
    QString output(process.readAll());
    //qDebug()<<output.simplified();

    /*
    QString output_status="";

    QProcess process_status;
    //output_status.compare("finished")!=0 ||

    while (output_status.compare("finished")!=0)
    {
        process_status.start ("curl -k -F m=info -F pass=hp39A11 -F ID="+output.simplified()+" http://via-lactea-sg00.iaps.inaf.it:8080/wspgrade/RemoteServlet");
        process_status.waitForFinished(); // sets current thread to sleep and waits for process end
        output_status= process_status.readAll().simplified();
        //qDebug()<<output_status;
        if(output_status.compare("error")==0){
            return;
        }
    }
    */
    //extern bool checkIDRemoteCall(QString id);
    QString id = output.simplified();
    QFuture<bool> future = QtConcurrent::run(checkIDRemoteCall, id);
    //    future.waitForFinished();
    bool finished=future.result();
    if(finished){


        QProcess process_download;
        process_download.start ("curl -k -F m=download -F ID="+output.simplified()+" -F pass=hp39A11 -o sed_fit/output.zip http://via-lactea-sg00.iaps.inaf.it:8080/wspgrade/RemoteServlet");
        process_download.waitForFinished(); // sets current thread to sleep and waits for process end
        QString output_download(process_download.readAll());
        //qDebug()<<output_download.simplified();


        QProcess process_unzip;
        //process_unzip.start ("unzip ""sed_fit/output.zip");
        process_unzip.start ("unzip sed_fit/output.zip -d "+QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/"));
        process_unzip.waitForFinished(); // sets current thread to sleep and waits for process end
        QString output_unzip(process_unzip.readAll());
        //qDebug()<<output_unzip.simplified();

        QStringList pieces = output_unzip.split( " " );
        //qDebug()<<pieces[pieces.length()-3];
        QString path=pieces[pieces.length()-3];

        path.replace(path.indexOf("guse.jsdl"),path.size(), QLatin1String("output.zip"));
        //qDebug() << path;

        //qDebug()<<"---- "<<"unzip "+path+" -d "+QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/");
        QProcess process_unzip_2;
        process_unzip_2.start ("unzip "+path+" -d "+QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/"));
        process_unzip_2.waitForFinished(); // sets current thread to sleep and waits for process end
        QString output_unzip_2(process_unzip_2.readAll());
        //qDebug()<<output_unzip_2.simplified();


        //    readSedFitOutput(QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/sedfit_output.dat"));

        //    QFile file_log( QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/log.dat"));
        //    if (!file_log.open(QFile::ReadOnly | QFile::Text)) return;
        //    QTextStream in(&file_log);
        //    outputSedLog=in.readAll();
        //    ui->outputTextBrowser->setText(outputSedLog);
        //    //qDebug()<<"****LOG***"<<outputSedLog;
    }

}

bool SEDVisualizerPlot::checkIDRemoteCall(QString id){

    //qDebug()<<"Sono it thread "<<QThread::currentThread();
    QString output_status="";

    QProcess process_status;

    while (output_status.compare("finished")!=0)
    {
        process_status.start ("curl -k -F m=info -F pass=hp39A11 -F ID="+id+" http://via-lactea-sg00.iaps.inaf.it:8080/wspgrade/RemoteServlet");
        process_status.waitForFinished(); // sets current thread to sleep and waits for process end
        output_status= process_status.readAll().simplified();
        ////qDebug()<<output_status;
        if(output_status.compare("error")==0){
            return false;
        }
    }
    return true;
}

void SEDVisualizerPlot::on_TheoreticalRemoteFit_triggered()
{

    bool validFit=false;

    sedFitInputFflag = "]";
    sedFitInputF= "]";
    sedFitInputW= "]";

    sedFitInputErrF= "]";


    sedFitInput.clear();

    // if(multiSelectMOD){
    validFit=prepareSelectedInputForSedFit();
    // }
    /* else{
        validFit=prepareInputForSedFit(sed_list.at(0)->getRootNode());
    }
    */

    if(validFit)
    {
        sedFitInputF ="["+sedFitInputF ;
        sedFitInputW ="["+sedFitInputW;
        sedFitInputFflag = "["+sedFitInputFflag;
        sedFitInputErrF ="["+sedFitInputErrF ;

        loading = new LoadingWidget();
        loading->init();
        loading->setFileName("Executing vialactea_tap_sedfit");
        loading ->show();
        loading->activateWindow();
        loading->setFocus();

        ui->outputTextBrowser->setText("");

        process= new QProcess();

        process->setProcessChannelMode(QProcess::MergedChannels);

        //qDebug()<<"sedFitInputErrF: "<<sedFitInputErrF;

        //qDebug()<<" java -jar "<<QApplication::applicationDirPath().append("/vsh-ws-client.jar")<<" \"sedpar=vialactea_tap_sedfit_v7("+sedFitInputW+","+sedFitInputF+","+sedFitInputErrF+","+sedFitInputFflag+","+ui->distTheoLineEdit->text()+","+ui->prefilterLineEdit->text()+",sed_weights=["+ui->mid_irLineEdit->text()+","+ui->far_irLineEdit->text()+","+ui->submmLineEdit->text()+"],use_wave="+sedFitInputW+",outdir='./',delta_chi2="+ui->delta_chi2_lineEdit->text()+")\" ";



        connect(process,SIGNAL(readyReadStandardError()),this,SLOT(onReadyReadStdOutput()));
        connect(process,SIGNAL(readyReadStandardOutput()),this,SLOT(onReadyReadStdOutput()));
        connect(process,SIGNAL(finished(int)),this,SLOT(finishedTheoreticalRemoteFit()));


        process->setWorkingDirectory(QApplication::applicationDirPath());
        process->start ("java -jar "+QApplication::applicationDirPath().append("/vsh-ws-client.jar")+" \"sedpar=vialactea_tap_sedfit_v7("+sedFitInputW+","+sedFitInputF+","+sedFitInputErrF+","+sedFitInputFflag+","+ui->distTheoLineEdit->text()+","+ui->prefilterLineEdit->text()+",sed_weights=["+ui->mid_irLineEdit->text()+","+ui->far_irLineEdit->text()+","+ui->submmLineEdit->text()+"],use_wave="+sedFitInputW+",outdir='./',delta_chi2="+ui->delta_chi2_lineEdit->text()+")\" ");

        qDebug()<<"vialactea_tap_sedfit_v7("+sedFitInputW+","+sedFitInputF+","+sedFitInputErrF+","+sedFitInputFflag+","+ui->distTheoLineEdit->text()+","+ui->prefilterLineEdit->text()+",sed_weights=["+ui->mid_irLineEdit->text()+","+ui->far_irLineEdit->text()+","+ui->submmLineEdit->text()+"],use_wave="+sedFitInputW+",outdir='./',delta_chi2="+ui->delta_chi2_lineEdit->text()+")\" ";
    }
}



void SEDVisualizerPlot::handleFinished(){
    //qDebug()<<"handleFinished";
    readSedFitOutput(QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/sedfit_output.dat"));
    QFile file_log( QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download/log.dat"));
    if (!file_log.open(QFile::ReadOnly | QFile::Text)) return;
    QTextStream in(&file_log);
    outputSedLog=in.readAll();
    ui->outputTextBrowser->setText(outputSedLog);
    //qDebug()<<"****LOG***"<<outputSedLog;
    loading->close();
    addNewSEDCheckBox("Theoretical Fit");
}

void SEDVisualizerPlot::on_logRadioButton_toggled(bool checked)
{
    if(checked==true){
        //Show log file
        ui->outputTextBrowser->setText(outputSedLog);
        ui->resultsTableWidget->hide();
        ui->outputTextBrowser->show();
        if(temporaryRow!=0){
            ui->customPlot->removeGraph(temporaryGraph);
            for (int i = 0; i < temporaryGraphPoints.size(); ++i)
            {
                SEDPlotPointCustom *cp;
                cp = temporaryGraphPoints.at(i);
                ui->customPlot->removeItem(cp);
            }
            temporaryGraphPoints.clear();
            temporaryRow=0;
        }

    }else{
        //Show results
        ui->outputTextBrowser->setText(outputSedResults);
        ui->resultsTableWidget->show();
        ui->outputTextBrowser->hide();
        //ui->resultsTableWidget->setColumnCount(2);
        //ui->resultsTableWidget->setRowCount(2);
        //ui->resultsTableWidget->setItem(0, 1, new QTableWidgetItem("Hello"));
    }
}

void SEDVisualizerPlot::on_clearAllButton_clicked()
{
    //qDebug()<<"Graphs size "<<sedGraphs.size();
    int size=sedGraphs.size();
    for (int i = 0; i < size; ++i)
    {
        QCPGraph *qcp;
        qcp = sedGraphs.at(i);
        ui->customPlot->removeGraph(qcp);
        //sedGraphs.removeAt(i);
    }
    //TODO: To check if items are removed from memory!
    sedGraphs.clear();
    //qDebug()<<"Graphs size "<<sedGraphs.size();
    //
    //qDebug()<<"Graph Points size "<<sedGraphPoints.size();
    QMap <int, QList<SEDPlotPointCustom *> >::iterator iter;
    for (iter = sedGraphPoints.begin(); iter != sedGraphPoints.end(); ++iter){
        //qDebug()<<"Processing Graph: "<<iter.key();
        QList<SEDPlotPointCustom *> points=sedGraphPoints.value(iter.key());
        //qDebug()<<"Points size: "<<points.size();
        for (int i = 0; i < points.size(); ++i)
        {
            SEDPlotPointCustom *cp;
            cp = points.at(i);
            ui->customPlot->removeItem(cp);
        }
    }
    sedGraphPoints.clear();

    if(temporaryRow!=0){//there is a temporary graph still drawn
        ui->customPlot->removeGraph(temporaryGraph);
        for (int i = 0; i < temporaryGraphPoints.size(); ++i)
        {
            SEDPlotPointCustom *cp;
            cp = temporaryGraphPoints.at(i);
            ui->customPlot->removeItem(cp);
        }
        temporaryGraphPoints.clear();
    }

    ui->customPlot->replot();
    nSED=0;
    temporaryMOD=false;
    doubleClicked=false;
    temporaryRow=0;

    QLayoutItem *item;

    //the key point here is that the layout items are stored inside the layout in a stack
    while((item = ui->generatedSedBox->layout()->takeAt(1)) != 0) {
        if (item->widget()) {
            //if(item->widget()->metaObject()->className()=="QCheckBox"){
            ui->generatedSedBox->layout()->removeWidget(item->widget());
            delete item->widget();
            //}
        }
        delete item;
    }
    //removeAllGraphs();
    ui->resultsTableWidget->clearContents();
    ui->resultsTableWidget->setRowCount(0);
    ui->outputTextBrowser->setText("");
    outputSedLog="";
    QDir tmp_download(QDir::homePath()+"/VisIVODesktopTemp/tmp_download");
    QStringList filters;
    filters << "SED*";
    tmp_download.setNameFilters(filters);
    QStringList dirList=tmp_download.entryList();
    //qDebug()<<"Dir size "<<dirList.size();
    for(int i=0; i<dirList.size(); i++){
        //qDebug()<<dirList.at(i);
        QFile sedFile(QDir::homePath()+"/VisIVODesktopTemp/tmp_download/"+dirList.at(i));
        sedFile.remove();
    }
    //tmp_download.removeRecursively();
    plottedSedLabels.clear();
}

void SEDVisualizerPlot::on_normalRadioButton_toggled(bool selectNormal)
{
    if(selectNormal==true){
        multiSelectMOD=false;
        ui->customPlot->setMultiSelectModifier(Qt::ControlModifier);
        QList<QCPAbstractItem *> list_items=ui->customPlot->selectedItems();
        for(int i=0;i<list_items.size();i++){
            list_items.at(i)->setSelected(false);
        }
        ui->customPlot->replot();
    }else{
        multiSelectMOD=true;
        ui->customPlot->setMultiSelectModifier(Qt::NoModifier);
    }
    //qDebug()<<"multiSelectMOD: "<<multiSelectMOD;
}

void SEDVisualizerPlot::on_actionSave_triggered()
{
    QProcess process_zip;
    QDir tmp_download(QDir::homePath()+"/VisIVODesktopTemp/tmp_download");
    QStringList filters;
    filters << "SED*";
    tmp_download.setNameFilters(filters);
    QStringList dirList=tmp_download.entryList();
    //qDebug()<<"Dir size "<<dirList.size();
    QString sedZipPath=QDir::homePath()+"/VisIVODesktopTemp/tmp_download/SED.zip";
    for(int i=0; i<dirList.size(); i++){
        QString sedPath=QDir::homePath()+"/VisIVODesktopTemp/tmp_download/"+dirList.at(i);
        process_zip.start ("zip -j "+sedZipPath+" "+sedPath);
        //qDebug()<<"zip -j "+sedZipPath+" "+sedPath;
        process_zip.waitForFinished(); // sets current thread to sleep and waits for process end
        QString output_zip(process_zip.readAll());
        //qDebug()<<output_zip.simplified();

        //FolderCompressor *fc=new FolderCompressor();
        //fc->compressFolder(QDir::homePath()+"/VisIVODesktopTemp/tmp_download/"+dirList.at(i), sedZipPath);
    }

    QString fileName = QFileDialog::getSaveFileName(this,tr("Save SED fits"), QDir::homePath(), tr("*.zip"));
    if(!fileName.endsWith(".zip",Qt::CaseInsensitive) )
        fileName.append(".zip");
    //qDebug()<<"Path: "<<fileName;
    QFile::copy(sedZipPath, fileName);
}

void SEDVisualizerPlot::on_actionLoad_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("Load SED fits"), QDir::homePath(), tr("Archive (*.zip)"));
    QString sedZipPath=QDir::homePath()+"/VisIVODesktopTemp/tmp_download/SED.zip";
    QFile::copy(fileName, sedZipPath);
    QProcess process_unzip;
    process_unzip.start ("unzip "+sedZipPath+" -d "+QDir::homePath()+"/VisIVODesktopTemp/tmp_download");
    //qDebug()<<"unzip "+sedZipPath+" -d "+QDir::homePath()+"/VisIVODesktopTemp/tmp_download";
    process_unzip.waitForFinished(); // sets current thread to sleep and waits for process end
    QString output_unzip(process_unzip.readAll());
    //qDebug()<<output_unzip.simplified();

    QDir tmp_download(QDir::homePath()+"/VisIVODesktopTemp/tmp_download");
    QStringList filters;
    filters << "SED*";
    tmp_download.setNameFilters(filters);
    QStringList dirList=tmp_download.entryList();
    //qDebug()<<"Dir size "<<dirList.size();
    for(int i=0; i<dirList.size(); i++){
        QString sedPath=QDir::homePath()+"/VisIVODesktopTemp/tmp_download/"+dirList.at(i);
        QString sedNumber = dirList.at(i).mid(3,1); // substring after "SED";
        //qDebug()<<"SED Number "<<sedNumber;
        bool ok;
        int value = sedNumber.toInt(&ok);
        if (ok) { // is an integer
            if(sedPath.endsWith("sedfit_model.dat")){
                //qDebug()<<"sedGraphs.size() before : "<<sedGraphs.size();
                //qDebug()<<"sedGraphPoints.size() before : "<<sedGraphPoints.size();
                //qDebug()<<"Drawing "+sedPath;
                addNewSEDCheckBox("Theoretical Fit");
                QFile file(sedPath);
                file.open(QIODevice::ReadOnly);
                QDataStream in(&file);    // read the data serialized from the file
                QVector<QStringList> headerAndValueList;
                in >> headerAndValueList;           // extract data
                setModelFitValue(headerAndValueList, Qt::blue);
            }
            if(sedPath.endsWith("sedfit_output.dat")){
                loadSedFitOutput(sedPath);
            }
            if(sedPath.endsWith("thinfile.csv")){
                //qDebug()<<"sedGraphs.size() before : "<<sedGraphs.size();
                //qDebug()<<"Drawing "+sedPath;
                readThinFit(sedPath);
                addNewSEDCheckBox("Thin Fit");
                //qDebug()<<"sedGraphs.size() after : "<<sedGraphs.size();
            }
            if(sedPath.endsWith("thickfile.csv")){
                //qDebug()<<"sedGraphs.size() before : "<<sedGraphs.size();
                //qDebug()<<"Drawing "+sedPath;
                readThinFit(sedPath);
                addNewSEDCheckBox("Thick Fit");
                //qDebug()<<"sedGraphs.size() after : "<<sedGraphs.size();
            }
        }
    }

}

void SEDVisualizerPlot::loadSavedSED(QStringList dirList){
    for(int i=0; i<dirList.size(); i++){
        QString sedPath=QDir::homePath()+"/VisIVODesktopTemp/tmp_download/"+dirList.at(i);
        QString sedNumber = dirList.at(i).mid(3,1); // substring after "SED";
        //qDebug()<<"SED Number "<<sedNumber;
        bool ok;
        int value = sedNumber.toInt(&ok);
        if (ok) { // is an integer
            if(sedPath.endsWith("sedfit_model.dat")){
                //qDebug()<<"sedGraphs.size() before : "<<sedGraphs.size();
                //qDebug()<<"sedGraphPoints.size() before : "<<sedGraphPoints.size();
                //qDebug()<<"Drawing "+sedPath;
                QString sedOutput=QDir::homePath()+"/VisIVODesktopTemp/tmp_download/SED"+QString::number(value)+"_sedfit_output.dat";
                loadSedFitOutput(sedOutput);
                //addNewSEDCheckBox("Theoretical Fit");
                QFile file(sedPath);
                file.open(QIODevice::ReadOnly);
                QDataStream in(&file);    // read the data serialized from the file
                QVector<QStringList> headerAndValueList;
                in >> headerAndValueList;           // extract data
                setModelFitValue(headerAndValueList, Qt::blue);
            }
            /*if(sedPath.endsWith("sedfit_output.dat")){
                loadSedFitOutput(sedPath);
            }*/
            if(sedPath.endsWith("thinfile.csv")){
                //qDebug()<<"sedGraphs.size() before : "<<sedGraphs.size();
                //qDebug()<<"Drawing "+sedPath;
                readThinFit(sedPath);
                addNewSEDCheckBox("Thin Fit");
                //qDebug()<<"sedGraphs.size() after : "<<sedGraphs.size();
            }
            if(sedPath.endsWith("thickfile.csv")){
                //qDebug()<<"sedGraphs.size() before : "<<sedGraphs.size();
                //qDebug()<<"Drawing "+sedPath;
                readThinFit(sedPath);
                addNewSEDCheckBox("Thick Fit");
                //qDebug()<<"sedGraphs.size() after : "<<sedGraphs.size();
            }
        }
    }
}

void SEDVisualizerPlot::on_collapseCheckBox_toggled(bool checked)
{
    if(checked){
        this->on_actionCollapse_triggered();
    }else{
        //qDebug()<<"Return previous state";
        QPen pen;
        pen.setStyle(Qt::SolidLine);
        pen.setColor(Qt::black);
        for(int i=0;i<originalGraphs.size();i++)
        {
            originalGraphs.at(i)->setPen(pen);
        }
        //originalGraphs.clear();
        ui->customPlot->removeGraph(collapsedGraph);
        for (int i = 0; i < collapsedGraphPoints.size(); ++i)
        {
            SEDPlotPointCustom *cp;
            cp = collapsedGraphPoints.at(i);
            ui->customPlot->removeItem(cp);
        }
        collapsedGraphPoints.clear();
        ui->customPlot->replot();
    }
}

void SEDVisualizerPlot::on_multiSelectCheckBox_toggled(bool checked)
{
    if(checked==true){
        multiSelectMOD=true;
        ui->customPlot->setMultiSelectModifier(Qt::NoModifier);
    }else{
        multiSelectMOD=false;
        ui->customPlot->setMultiSelectModifier(Qt::ControlModifier);
        QList<QCPAbstractItem *> list_items=ui->customPlot->selectedItems();
        for(int i=0;i<list_items.size();i++){
            list_items.at(i)->setSelected(false);
        }
        ui->customPlot->replot();
    }
    //qDebug()<<"multiSelectMOD: "<<multiSelectMOD;
}

void SEDVisualizerPlot::addNewTheoreticalFit(){
    QModelIndex index= ui->resultsTableWidget->selectionModel()->selectedIndexes().first();
    //qDebug()<<"Clicked Menu ";
    doubleClicked=true;
    temporaryMOD=false;
    //qDebug()<<"Double Cliked Row "+QString::number(index.row());
    int id=ui->resultsTableWidget->item(index.row(),0)->text().toInt();
    //qDebug()<<"Selected id "+QString::number(id);
    if(index.row()==0){
        // Do nothing on the first row which is already drawn by default
    }
    else{

        QString query="SELECT * FROM vlkb_compactsources.sed_models where id="+ QString::number(id);
        //qDebug()<<"query: " << query;
        new VLKBQuery(query,vtkwin, "model", this, Qt::cyan);
        if(temporaryRow!=0){
            ui->customPlot->removeGraph(temporaryGraph);
            for (int i = 0; i < temporaryGraphPoints.size(); ++i)
            {
                SEDPlotPointCustom *cp;
                cp = temporaryGraphPoints.at(i);
                ui->customPlot->removeItem(cp);
            }
            temporaryGraphPoints.clear();
            temporaryRow=0;
        }
    }
    this->setFocus();
    this->activateWindow();
}


void SEDVisualizerPlot::on_resultsTableWidget_clicked(const QModelIndex &index)
{
    if(!doubleClicked){
        temporaryMOD=true;
        //qDebug()<<"Cliked Row "+QString::number(index.row());
        int id=ui->resultsTableWidget->item(index.row(),0)->text().toInt();
        //qDebug()<<"Selected id "+QString::number(id);
        if(index.row()==0){
            // Do nothing on the first row which is already drawn by default
            // if a temporary graph was shown delete it
            //qDebug()<<temporaryGraphPoints.size();
            if(temporaryGraphPoints.size()!=0){
                ui->customPlot->removeGraph(temporaryGraph);
                for (int i = 0; i < temporaryGraphPoints.size(); ++i)
                {
                    SEDPlotPointCustom *cp;
                    cp = temporaryGraphPoints.at(i);
                    ui->customPlot->removeItem(cp);
                }
                temporaryGraphPoints.clear();
                temporaryRow=0;
                ui->customPlot->replot();
            }
        }
        else if(index.row()!=temporaryRow){
            if(temporaryRow!=0){
                ui->customPlot->removeGraph(temporaryGraph);
                for (int i = 0; i < temporaryGraphPoints.size(); ++i)
                {
                    SEDPlotPointCustom *cp;
                    cp = temporaryGraphPoints.at(i);
                    ui->customPlot->removeItem(cp);
                }
                temporaryGraphPoints.clear();
            }

            QString query="SELECT * FROM vlkb_compactsources.sed_models where id="+ QString::number(id);
            //qDebug()<<"query: " << query;
            new VLKBQuery(query,vtkwin, "model", this, Qt::lightGray);
            temporaryRow=index.row();
        }
        doubleClicked=false;
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
    /*
    if(false)
        QMessageBox::critical(NULL, QObject::tr("Error"), QObject::tr("Could not execute SED fit with less than 2 points selected.\n\rPlease select more points and try again."));
*/

    on_TheoreticalRemoteFit_triggered();
}

void SEDVisualizerPlot::on_greyBodyPushButton_clicked()
{
    ui->theorGroupBox->hide();
    ui->greyBodyGroupBox->show();

}

void SEDVisualizerPlot::on_ThinRemore_triggered()
{


    sedFitInputF= "]";
    sedFitInputErrF= "]";
    sedFitInputW= "]";

    sedFitInput.clear();
    /*  if(multiSelectMOD){
        prepareSelectedInputForSedFit();
    }else{
        prepareInputForSedFit(sed_list.at(0)->getRootNode());
    }
    */
    bool validFit=false;

    validFit=prepareSelectedInputForSedFit();

    if(validFit)
    {


        sedFitInputF ="["+sedFitInputF ;
        sedFitInputErrF ="["+sedFitInputErrF ;
        sedFitInputW ="["+sedFitInputW;


        QString f=sedFitInputW.mid(1,sedFitInputW.length()-2);

        QStringList wave = f.split(",");

        QSignalMapper* mapper = new QSignalMapper(this);

        sd_thin = new SedFitGrid_thin(this);
        sd_thin->ui->distLineEdit->setText(ui->distTheoLineEdit->text());
        sedFitInputUlimitString="[";

        for(int i=0;i<wave.length();i++)
        {
            //qDebug()<<wave.at(i);
            sedFitInputUlimit.insert(i,"0");

            sedFitInputUlimitString+="0";
            if(i<wave.size()-1)
                sedFitInputUlimitString+=",";

            int row= sd_thin->ui->tableWidget->model()->rowCount();
            sd_thin->ui->tableWidget->insertRow(row);

            QCheckBox  *cb1= new QCheckBox();
            cb1->setChecked(false);
            sd_thin->ui->tableWidget->setCellWidget(row,1,cb1);

            connect(cb1, SIGNAL(stateChanged(int)),mapper, SLOT(map()));
            mapper->setMapping(cb1, row);

            QTableWidgetItem *item_1 = new QTableWidgetItem();
            item_1->setText(wave.at(i));

            sd_thin->ui->tableWidget->setItem(row,0,item_1);

        }
        sedFitInputUlimitString+="]";
        connect(mapper, SIGNAL(mapped(int)), this, SLOT(checkboxClicked(int)));

        //qDebug()<<sedFitInputUlimitString;



        sd_thin->show();

        connect( sd_thin->ui->pushButton ,SIGNAL(clicked()),this,SLOT(doThinRemoteFit()));
    }
}

void SEDVisualizerPlot::on_ThickRemote_triggered()
{
    sedFitInputF= "]";
    sedFitInputErrF= "]";
    sedFitInputW= "]";

    sedFitInput.clear();
    /*
    if(multiSelectMOD){
        prepareSelectedInputForSedFit();
    }else{
        prepareInputForSedFit(sed_list.at(0)->getRootNode());
    }
*/
    bool validFit=true;

    validFit=prepareSelectedInputForSedFit();

    if(validFit)
    {


        sedFitInputF ="["+sedFitInputF ;
        sedFitInputErrF ="["+sedFitInputErrF ;
        sedFitInputW ="["+sedFitInputW;

        //qDebug() << "f: " <<sedFitInputF;
        //qDebug() << "e_f: " <<sedFitInputErrF;
        //qDebug() << "w: " <<sedFitInputW;


        QString f=sedFitInputW.mid(1,sedFitInputW.length()-2);

        QStringList wave = f.split(",");

        QSignalMapper* mapper = new QSignalMapper(this);

        sd_thick = new SedFitgrid_thick(this);
        sd_thick->ui->distLineEdit->setText(ui->distTheoLineEdit->text());

        sedFitInputUlimitString="[";

        for(int i=0;i<wave.length();i++)
        {
            //qDebug()<<wave.at(i);
            sedFitInputUlimit.insert(i,"0");

            sedFitInputUlimitString+="0";
            if(i<wave.size()-1)
                sedFitInputUlimitString+=",";

            int row= sd_thick->ui->tableWidget->model()->rowCount();
            sd_thick->ui->tableWidget->insertRow(row);

            QCheckBox  *cb1= new QCheckBox();
            cb1->setChecked(false);
            sd_thick->ui->tableWidget->setCellWidget(row,1,cb1);

            connect(cb1, SIGNAL(stateChanged(int)),mapper, SLOT(map()));
            mapper->setMapping(cb1, row);

            QTableWidgetItem *item_1 = new QTableWidgetItem();
            item_1->setText(wave.at(i));

            sd_thick->ui->tableWidget->setItem(row,0,item_1);

        }
        sedFitInputUlimitString+="]";
        connect(mapper, SIGNAL(mapped(int)), this, SLOT(checkboxClicked(int)));

        sd_thick->show();
        connect( sd_thick->ui->pushButton ,SIGNAL(clicked()),this,SLOT(doThickRemoteFit()));
    }

}


void SEDVisualizerPlot::on_Thick_clicked()
{
    on_ThickRemote_triggered();
}

void SEDVisualizerPlot::on_thinButton_clicked()
{
    on_ThinRemore_triggered();
}
