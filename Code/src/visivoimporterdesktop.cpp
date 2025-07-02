/***************************************************************************
 *   Copyright (C) 2010 by Alessandro Costa                                *
 *   alessandro.costa@oact.inaf.it                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "visivoimporterdesktop.h"
#include "astroutils.h"
#include "mainwindow.h"
#include "singleton.h"
#include "treemodel.h"
#include "ui_mainwindow.h"
#include "vialactea.h"
#include "vtkColorTransferFunction.h"
#include "vtkfitsreader.h"
#include "vtkFixedPointVolumeRayCastMapper.h"
#include "vtkImageViewer2.h"
#include "vtkPiecewiseFunction.h"
#include "vtkSmartPointer.h"
#include "vtkVolume.h"
#include "vtkVolumeProperty.h"
#include "vtkwindow_new.h"
#include "vtkXMLImageDataReader.h"
#include "vtkXMLPolyDataReader.h"
#include <QDebug>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QProcess>
#include <QTreeView>

#include "sed.h"
#include "sednode.h"
#include "vialacteasource.h"
#include "vlkbquery.h"
#include <math.h>

#include "sedvisualizerplot.h"

#include <QtConcurrent>

/*
extern "C" {
#include "visivo.h"
}

*/

VisIVOImporterDesktop::VisIVOImporterDesktop(QString f, TreeModel *m, bool isBandMergedCatalogue,
                                             vtkwindow_new *v, QString wavelen)
    : m_VisIVOTable(new VSTableDesktop)
{
    auto vialactea_source = new VialacteaSource(f.toStdString());
    m_VisIVOTable->setNumberOfColumns(vialactea_source->getNumberOfColumns());
    m_VisIVOTable->setNumberOfRows(vialactea_source->getNumberOfRows());
    m_VisIVOTable->setColsNames(vialactea_source->getColumnsNames());
    m_VisIVOTable->setWavelength(wavelen.toInt());
    QFileInfo infoFile = QFileInfo(f);
    queue = &Singleton<OperationQueue>::Instance();
    operation_queue_row = queue->addOperation("Importing " + infoFile.fileName());

    m_VisIVOTable->setName(infoFile.absoluteFilePath().toStdString());
    m_VisIVOTable->setTableData(vialactea_source->getData());
    vtkwin = v;

    if (!isBandMergedCatalogue) {
        if (wavelen.compare("all") == 0)
            vtkwin->addSourcesFromBM(m_VisIVOTable);
        else
            vtkwin->addSources(m_VisIVOTable);
    } else {
        firstSEDNode = true;
        double distances;

        QStringList bands { "1100", "870", "500", "250", "160", "70", "24", "22" };
        for (int i = 0; i < m_VisIVOTable->getNumberOfRows() / 2; ++i) {
            firstSEDNode = true;

            auto sed = new SED();

            SEDNode *node = nullptr;
            SEDNode *tmp_node = nullptr;

            double flux, e_flux, minAxes, maxAxes, angle, arcpix, glon, glat;
            double coord[3];

            for (const auto &b : bands) {
                const std::string band = b.toStdString();
                QString designation = QString::fromStdString(
                        m_VisIVOTable
                                ->getTableData()[m_VisIVOTable->getColId("designation" + band)][i]);
                if (designation != "missing") {
                    firstSEDNode = false;

                    flux = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux" + band)][i]
                                    .c_str());
                    if (flux <= 0) {
                        continue;
                    }

                    e_flux = std::atof(
                            m_VisIVOTable
                                    ->getTableData()[m_VisIVOTable->getColId("err_flux" + band)][i]
                                    .c_str());
                    minAxes = std::atof(
                            m_VisIVOTable
                                    ->getTableData()[m_VisIVOTable->getColId("fwhma" + band)][i]
                                    .c_str());
                    maxAxes = std::atof(
                            m_VisIVOTable
                                    ->getTableData()[m_VisIVOTable->getColId("fwhmb" + band)][i]
                                    .c_str());
                    angle = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("pa" + band)][i]
                                    .c_str());
                    glon = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon" + band)][i]
                                    .c_str());
                    glat = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat" + band)][i]
                                    .c_str());

                    AstroUtils::sky2xy(vtkwin->filenameWithPath, glon, glat, coord);

                    tmp_node = node;
                    node = new SEDNode();
                    node->setDesignation(designation);
                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setWavelength(b.toDouble());
                    node->setSky(glon, glat);
                    node->setXY(coord[0], coord[1]);
                    node->setEllipse(minAxes, maxAxes, angle, coord[2]);

                    sed->updateMaxFlux(flux);
                    if (!sed->hasRoot()) {
                        sed->setRootNode(node);
                    }

                    if (tmp_node) {
                        node->setParent(tmp_node);
                        tmp_node->setChild(node);
                    }
                }
            }

            // 21, 14.7, 12.1 are based on 21 um
            QList<QPair<double, QString>> suffices { { 21, "_e" }, { 14.7, "_d" }, { 12.1, "_c" } };
            for (const auto &b : suffices) {
                QString designation = QString::fromStdString(
                        m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("designation21")][i]);
                if (designation != "missing") {
                    firstSEDNode = false;
                    flux = std::atof(m_VisIVOTable
                                             ->getTableData()[m_VisIVOTable->getColId(
                                                     "flux21" + b.second.toStdString())][i]
                                             .c_str());
                    if (flux <= 0) {
                        continue;
                    }
                    e_flux = std::atof(m_VisIVOTable
                                               ->getTableData()[m_VisIVOTable->getColId(
                                                       "err_flux21" + b.second.toStdString())][i]
                                               .c_str());
                    minAxes = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhma21")][i]
                                    .c_str());
                    maxAxes = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhmb21")][i]
                                    .c_str());
                    angle = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("pa21")][i]
                                    .c_str());
                    glon = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon21")][i]
                                    .c_str());
                    glat = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat21")][i]
                                    .c_str());
                    AstroUtils::sky2xy(vtkwin->filenameWithPath, glon, glat, coord);
                    tmp_node = node;
                    node = new SEDNode();
                    node->setDesignation(designation + b.second);
                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setWavelength(b.first);
                    node->setSky(glon, glat);
                    node->setXY(coord[0], coord[1]);
                    node->setEllipse(minAxes, maxAxes, angle, coord[2]);
                    sed->updateMaxFlux(flux);
                    if (!sed->hasRoot()) {
                        sed->setRootNode(node);
                    }

                    if (tmp_node) {
                        node->setParent(tmp_node);
                        tmp_node->setChild(node);
                    }
                }
            }

            // 12um based on 22um
            QString designation = QString::fromStdString(
                    m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("designation22")][i]);
            if (designation != "missing") {
                firstSEDNode = false;
                flux = std::atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux12")][i]
                                         .c_str());
                if (flux > 0) {
                    e_flux = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("err_flux12")][i]
                                    .c_str());
                    minAxes = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhma22")][i]
                                    .c_str());
                    maxAxes = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhmb22")][i]
                                    .c_str());
                    angle = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("pa22")][i]
                                    .c_str());
                    glon = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon22")][i]
                                    .c_str());
                    glat = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat22")][i]
                                    .c_str());
                    AstroUtils::sky2xy(vtkwin->filenameWithPath, glon, glat, coord);
                    tmp_node = node;
                    node = new SEDNode();
                    node->setDesignation(designation + "_12um");
                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setWavelength(12);
                    node->setSky(glon, glat);
                    node->setXY(coord[0], coord[1]);
                    node->setEllipse(minAxes, maxAxes, angle, coord[2]);
                    sed->updateMaxFlux(flux);
                    if (!sed->hasRoot()) {
                        sed->setRootNode(node);
                    }

                    if (tmp_node) {
                        node->setParent(tmp_node);
                        tmp_node->setChild(node);
                    }
                }
            }

            // 8.3um based on 21um
            designation = QString::fromStdString(
                    m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("designation21")][i]);
            if (designation != "missing") {
                firstSEDNode = false;
                flux = std::atof(
                        m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux21_a")][i]
                                .c_str());
                if (flux > 0) {
                    e_flux = std::atof(
                            m_VisIVOTable
                                    ->getTableData()[m_VisIVOTable->getColId("err_flux21_a")][i]
                                    .c_str());
                    minAxes = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhma21")][i]
                                    .c_str());
                    maxAxes = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhmb21")][i]
                                    .c_str());
                    angle = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("pa21")][i]
                                    .c_str());
                    glon = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon21")][i]
                                    .c_str());
                    glat = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat21")][i]
                                    .c_str());
                    AstroUtils::sky2xy(vtkwin->filenameWithPath, glon, glat, coord);
                    tmp_node = node;
                    node = new SEDNode();
                    node->setDesignation(designation + "_a");
                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setWavelength(8.3);
                    node->setSky(glon, glat);
                    node->setXY(coord[0], coord[1]);
                    node->setEllipse(minAxes, maxAxes, angle, coord[2]);
                    sed->updateMaxFlux(flux);
                    if (!sed->hasRoot()) {
                        sed->setRootNode(node);
                    }

                    if (tmp_node) {
                        node->setParent(tmp_node);
                        tmp_node->setChild(node);
                    }
                }
            }

            // 4.6, 3.4, 2.2, 1.65, 1.25 based on 22um
            QList<QPair<double, QString>> suffices2 {
                { 4.6, "46" }, { 3.4, "34" }, { 2.2, "2M_K" }, { 1.65, "2M_H" }, { 1.25, "2M_J" }
            };
            for (const auto &b : suffices2) {
                QString designation = QString::fromStdString(
                        m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("designation22")][i]);
                if (designation != "missing") {
                    firstSEDNode = false;
                    flux = std::atof(m_VisIVOTable
                                             ->getTableData()[m_VisIVOTable->getColId(
                                                     "flux" + b.second.toStdString())][i]
                                             .c_str());
                    if (flux <= 0) {
                        continue;
                    }
                    e_flux = std::atof(m_VisIVOTable
                                               ->getTableData()[m_VisIVOTable->getColId(
                                                       "err_flux" + b.second.toStdString())][i]
                                               .c_str());
                    minAxes = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhma22")][i]
                                    .c_str());
                    maxAxes = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhmb22")][i]
                                    .c_str());
                    angle = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("pa22")][i]
                                    .c_str());
                    glon = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon22")][i]
                                    .c_str());
                    glat = std::atof(
                            m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat22")][i]
                                    .c_str());
                    AstroUtils::sky2xy(vtkwin->filenameWithPath, glon, glat, coord);
                    tmp_node = node;
                    node = new SEDNode();
                    node->setDesignation(designation + "_" + QString::number(b.first) + "um");
                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setWavelength(b.first);
                    node->setSky(glon, glat);
                    node->setXY(coord[0], coord[1]);
                    node->setEllipse(minAxes, maxAxes, angle, coord[2]);
                    sed->updateMaxFlux(flux);
                    if (!sed->hasRoot()) {
                        sed->setRootNode(node);
                    }

                    if (tmp_node) {
                        node->setParent(tmp_node);
                        tmp_node->setChild(node);
                    }
                }
            }

            sedlist.push_back(sed);
            distances = 1000
                    * atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("distance")][i]
                                   .c_str());
        }

        ViaLactea *vialactealWin = &Singleton<ViaLactea>::Instance();
        SEDVisualizerPlot *sedv = new SEDVisualizerPlot(sedlist, vtkwin, vialactealWin);
        if (distances == -9999)
            distances = 2000;
        sedv->setDistances(distances);
        sedv->show();
    }

    queue->editOperation(operation_queue_row, "Completed");
    queue->close();
}

SEDNode *VisIVOImporterDesktop::bmContainsNode(QString nodename)
{
    SEDNode *node;
    if (!nodelist.contains(nodename)) {
        node = new SEDNode();
        node->setDesignation(nodename);
        nodelist.insert(nodename, node);
    } else {
        node = nodelist.value(nodename);
    }
    return node;
}

VisIVOImporterDesktop::VisIVOImporterDesktop(QString t, QString f, TreeModel *m)
{
    m_impStatus = -1;
    /*  m_impStatus:
        1 : an error occurred
        0 : importing OK
    */

    model = m;
    fileName = f;
    infoFile = QFileInfo(fileName);
    type = t;
    isHigal = false;
    queue = &Singleton<OperationQueue>::Instance();
}

VisIVOImporterDesktop::VisIVOImporterDesktop(QString f, vtkwindow_new *v, bool isFilament,
                                             bool isBubble)
{
    QFileInfo infoFile = QFileInfo(f);

    headerFileName.append(QDir::homePath() + "/VisIVODesktopTemp/tmp_download/")
            .append(infoFile.baseName())
            .append(".bin.head");
    binaryFileName.append(QDir::homePath() + "/VisIVODesktopTemp/tmp_download/")
            .append(infoFile.baseName())
            .append(".bin");

    headerFile = new QFile(headerFileName);
    importedFile = new QFile(fileName);
    binaryFile = new QFile(binaryFileName);

    QString type = "ascii";
    QString outputPathString =
            QDir::homePath() + "/VisIVODesktopTemp/tmp_download/" + infoFile.baseName() + ".bin";

    VialacteaSource *vialactea_source = new VialacteaSource(f.toStdString());

    m_VisIVOTable = new VSTableDesktop();
    m_VisIVOTable->setLocator(outputPathString.toStdString());
    m_VisIVOTable->setNumberOfColumns(vialactea_source->getNumberOfColumns());
    m_VisIVOTable->setNumberOfRows(vialactea_source->getNumberOfRows());
    m_VisIVOTable->setColsNames(vialactea_source->getColumnsNames());
    m_VisIVOTable->readStatistics();
    vialactea_source->computeHistogram();

    m_VisIVOTable->setNumberOfBins(vialactea_source->getNumberOfBins());
    m_VisIVOTable->setHistogramArray(vialactea_source->getHistogramArray());
    m_VisIVOTable->setHistogramValueArray(vialactea_source->getHistogramValueArray());

    queue = &Singleton<OperationQueue>::Instance();
    operation_queue_row = queue->addOperation("Importing " + infoFile.fileName());

    m_VisIVOTable->setName(infoFile.absoluteFilePath().toStdString());
    m_VisIVOTable->setTableData(vialactea_source->getData());

    vtkwin = v;

    if (isFilament) {
        vtkwin->addFilaments(m_VisIVOTable);
    } else if (isBubble) {
        vtkwin->addBubble(m_VisIVOTable);
    } else {
        VisPoint *m_VisPointsObject = new VisPoint(m_VisIVOTable);
        m_VisPointsObject->setX("x");
        m_VisPointsObject->setY("y");
        m_VisPointsObject->setZ("z");
        m_VisPointsObject->setScale(true);
        vtkwindow_new *m_OldRenderingWindow = new vtkwindow_new(this, m_VisPointsObject);
    }
}

void VisIVOImporterDesktop::setVtkWin(vtkwindow_new *v)
{
    vtkwin = v;
}

void VisIVOImporterDesktop::setBm(bool bm)
{
    isBandMergedCatalogue = bm;
}

void VisIVOImporterDesktop::doImport(QString wavelen, bool usingAPI)
{

    wavelength = wavelen;
    operation_queue_row = queue->addOperation("Importing " + fileName);

    MainWindow *w = &Singleton<MainWindow>::Instance();
    if (!usingAPI) {
        if (type == "FITSIMG") {

            vtkSmartPointer<vtkFitsReader> fitsReader = vtkSmartPointer<vtkFitsReader>::New();
            fitsReader->SetFileName(fileName.toStdString());

            // Modify the TreeModel
            int rows = model->rowCount();
            model->insertRow(rows);
            model->setFITSIMG(model->index(rows, 0), fitsReader);
            model->setData(model->index(rows, 0), QIcon(":/icons/VME_IMAGE.bmp"));
            model->setData(model->index(rows, 1), fileName);

            queue->editOperation(operation_queue_row, "Completed");
            return;
        }
        if (type == "VTP") {
            queue->show();

            vtkSmartPointer<vtkXMLPolyDataReader> vtpReader =
                    vtkSmartPointer<vtkXMLPolyDataReader>::New();
            vtkSmartPointer<vtkPolyData> pPolyData = vtkSmartPointer<vtkPolyData>::New();
            vtkSmartPointer<vtkPolyDataMapper> pMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
            vtkSmartPointer<vtkLODActor> pActor = vtkSmartPointer<vtkLODActor>::New();

            vtpReader->SetFileName(fileName.toStdString().c_str());
            vtpReader->Update();
            pPolyData = vtpReader->GetOutput();
            pMapper->SetInputData(pPolyData);
            pActor->SetMapper(pMapper);
            return;
        }
        if (type == "VTI") {
            queue->show();

            vtkSmartPointer<vtkXMLImageDataReader> vtiReader =
                    vtkSmartPointer<vtkXMLImageDataReader>::New();
            vtkSmartPointer<vtkFixedPointVolumeRayCastMapper> volumeMapper =
                    vtkSmartPointer<vtkFixedPointVolumeRayCastMapper>::New();
            vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();

            vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction =
                    vtkSmartPointer<vtkColorTransferFunction>::New();
            vtkSmartPointer<vtkPiecewiseFunction> opacityTransferFunction =
                    vtkSmartPointer<vtkPiecewiseFunction>::New();
            vtkSmartPointer<vtkVolumeProperty> volumeProperty =
                    vtkSmartPointer<vtkVolumeProperty>::New();

            vtiReader->SetFileName(fileName.toStdString().c_str());
            vtiReader->Update();

            volumeMapper->SetInputConnection(vtiReader->GetOutputPort());

            volumeMapper->SetImageSampleDistance(0.5);
            volume->SetMapper(volumeMapper);
            colorTransferFunction->AddRGBPoint(0, 0, 0, 0);
            colorTransferFunction->AddRGBPoint(0.396415, 1, 0, 0);
            colorTransferFunction->AddRGBPoint(0.531872, 1, 0, 0);
            colorTransferFunction->AddRGBPoint(0.697211, 0.901961, 0, 0);
            colorTransferFunction->AddRGBPoint(0.76494, 0.901961, 0.831891, 0);
            colorTransferFunction->AddRGBPoint(0.824701, 0.901961, 0.831891, 0);
            colorTransferFunction->AddRGBPoint(0.888446, 0.901961, 0.901961, 0);
            colorTransferFunction->AddRGBPoint(1, 1, 1, 1);
            double step = 1.0 / 256;
            double opValue = 0;
            for (double i = 0; i <= 1; i += step) {
                opValue = (tanh(5 * i - 3) + 1) / 2.5;
                opacityTransferFunction->AddPoint(i, opValue);
            }
            volumeProperty->SetScalarOpacity(opacityTransferFunction);
            volumeProperty->ShadeOff();
            volumeProperty->SetInterpolationTypeToLinear();
            volumeProperty->SetColor(colorTransferFunction);
            volume->SetProperty(volumeProperty);
            int rows = model->rowCount();
            model->insertRow(rows);
            model->setVTI(model->index(rows, 0), volume);
            model->setData(model->index(rows, 0), QIcon(":/icons/VME_VOLUME.bmp"));
            model->setData(model->index(rows, 1), fileName);
            return;
        }

        headerFileName.append(w->workingPath)
                .append(QDir::separator())
                .append(infoFile.baseName())
                .append(".bin.head");
        binaryFileName.append(w->workingPath)
                .append(QDir::separator())
                .append(infoFile.baseName())
                .append(".bin");

        headerFile = new QFile(headerFileName);
        importedFile = new QFile(fileName);
        binaryFile = new QFile(binaryFileName);

        if (!importedFile->exists()) {
            m_impStatus = 1;
            return;
        } else {
            // We 're importing a supported VisIVO File by using the canonical importer
            // The result if m_impStatus==0 will be VBT (VisIVO Binary Table)
            QProcess *myProcess = new QProcess(this->parent());
            QStringList arguments;
            connect(myProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this,
                    SLOT(onProcessFinished(int, QProcess::ExitStatus)));

            arguments << "--fformat" << type << "--out" << binaryFileName << fileName;

            myProcess->setStandardErrorFile(QDir::homePath()
                                                    .append(QDir::separator())
                                                    .append("VisIVODesktopTemp")
                                                    .append("log_error.txt"),
                                            QIODevice::Append);
            myProcess->setStandardOutputFile(QDir::homePath()
                                                     .append(QDir::separator())
                                                     .append("VisIVODesktopTemp")
                                                     .append("log_output.txt"),
                                             QIODevice::Append);
            myProcess->start(w->importer, arguments);
        }

    } else {

        headerFileName.append(QDir::homePath() + "/VisIVODesktopTemp/tmp_download/")
                .append(infoFile.baseName())
                .append(".bin.head");
        binaryFileName.append(QDir::homePath() + "/VisIVODesktopTemp/tmp_download/")
                .append(infoFile.baseName())
                .append(".bin");

        headerFile = new QFile(headerFileName);
        importedFile = new QFile(fileName);
        binaryFile = new QFile(binaryFileName);

        if (!importedFile->exists()) {
            m_impStatus = 1;
            return;
        } else {
            int errorCode;

            char *filepath = new char[fileName.toStdString().length() + 1];
            strcpy(filepath, fileName.toStdString().c_str());

            char *fileformat = new char[type.toStdString().length() + 1];
            strcpy(fileformat, type.toStdString().c_str());
        }
    }
}

void VisIVOImporterDesktop::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_impStatus = exitStatus;
    if (!headerFile->exists()) {
        m_impStatus = 1;
        return;
    }
    if (!binaryFile->exists()) {
        m_impStatus = 1;
        return;
    }

    /*  m_impStatus:
                1 : an error occurred
                0 : OK
                */
    if (!m_impStatus) {

        std::string file = binaryFileName.toStdString();
        std::string name = infoFile.baseName().toStdString();
        m_VisIVOTable = new VSTableDesktop(file, name, "", true);
        // Modify the TreeModel
        QString tableName;
        bool result;
        QModelIndex parentIndex;
        tableName = QString(m_VisIVOTable->getName().c_str());

        parentIndex = QModelIndex();
        int rows = model->rowCount(parentIndex);
        if (model->insertRows(rows, 1, parentIndex)) {
            result = model->setTable(model->index(rows, 0, parentIndex),
                                     m_VisIVOTable); // We append a VSTable to the item
            if (!m_VisIVOTable->getIsVolume()) // The table is a Point Table
                result = model->setData(model->index(rows, 0, parentIndex),
                                        QIcon(":/icons/VBT_Table.bmp"));
            else // The table is a Volume Table VBT_Volume
                result = model->setData(model->index(rows, 0, parentIndex),
                                        QIcon(":/icons/VBT_Volume.bmp"));

            result = model->setData(model->index(rows, 1, parentIndex), tableName);
            model->setLastInsertItem(model->index(rows, 0, parentIndex));
            MainWindow *w = &Singleton<MainWindow>::Instance();
            w->ui->tabWidget->setCurrentWidget(w->ui->tabObjectTree);
        }

        // VIALACTEA
        if (isHigal) {
            if (!isBandMergedCatalogue)
                vtkwin->addSources(model->getTable(model->getLastInsertItem()));
            else {
                unsigned int *colList;
                colList = new unsigned int[5];
                std::string **m_tableData = NULL;
                m_tableData = new std::string *[5];

                for (int i = 0; i < 5; i++) {
                    m_tableData[i] = new std::string[m_VisIVOTable->getNumberOfRows()];
                }

                colList[0] = m_VisIVOTable->getColId("N70");
                colList[1] = m_VisIVOTable->getColId("N160");
                colList[2] = m_VisIVOTable->getColId("N250");
                colList[3] = m_VisIVOTable->getColId("N350");
                colList[4] = m_VisIVOTable->getColId("N500");

                m_VisIVOTable->getColumnString(colList, 5, 0, m_VisIVOTable->getNumberOfRows(),
                                               m_tableData);

                QHash<QString, int> bm500;
                QHash<QString, int> bm350;
                QHash<QString, int> bm250;
                QHash<QString, int> bm160;
                QHash<QString, int> bm70;

                for (int i = 0; i < m_VisIVOTable->getNumberOfRows(); i++) {

                    bm500.insert(QString::fromStdString(m_tableData[4][i]), i);
                    bm350.insert(QString::fromStdString(m_tableData[3][i]), i);
                    bm250.insert(QString::fromStdString(m_tableData[2][i]), i);
                    bm160.insert(QString::fromStdString(m_tableData[1][i]), i);
                    bm70.insert(QString::fromStdString(m_tableData[0][i]), i);
                }

                m_VisIVOTable->setBandMerged500(bm500);
                m_VisIVOTable->setBandMerged350(bm350);
                m_VisIVOTable->setBandMerged250(bm250);
                m_VisIVOTable->setBandMerged160(bm160);
                m_VisIVOTable->setBandMerged70(bm70);
            }
        }

        queue->editOperation(operation_queue_row, "Completed");
    }
}

VSTableDesktop *VisIVOImporterDesktop::getTable()
{
    return m_VisIVOTable;
}
int VisIVOImporterDesktop::getStatus()
{
    return m_impStatus;
}
