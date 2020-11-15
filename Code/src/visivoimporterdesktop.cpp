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
#include <QDebug>
#include <QDialog>
#include <QFileDialog>
#include <QTreeView>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QFileInfo>
#include "vtkwindow_new.h"
#include "mainwindow.h"
#include "treemodel.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkXMLImageDataReader.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkSmartPointer.h"
#include "vtkFixedPointVolumeRayCastMapper.h"
#include "vtkColorTransferFunction.h"
#include "vtkVolume.h"
#include "vtkPiecewiseFunction.h"
#include "vtkVolumeProperty.h"
#include "singleton.h"
#include "vtkfitsreader.h"
#include "vtkImageViewer2.h"
#include "vialactea.h"
#include "astroutils.h"
#include "base64.h"

#include "vialacteasource.h"
#include "sed.h"
#include "sednode.h"
#include "vlkbquery.h"
#include <math.h>

#include "sedvisualizerplot.h"

#include <QtConcurrent>

/*
extern "C" {
#include "visivo.h"
}

*/


VisIVOImporterDesktop::VisIVOImporterDesktop(QString f, TreeModel * m, bool isBandMergedCatalogue, vtkwindow_new *v, QString wavelen)
{

    VialacteaSource *vialactea_source= new VialacteaSource(f.toStdString());

    m_VisIVOTable = new VSTableDesktop();

    m_VisIVOTable->setNumberOfColumns(vialactea_source->getNumberOfColumns());
    m_VisIVOTable->setNumberOfRows(vialactea_source->getNumberOfRows());
    m_VisIVOTable->setColsNames(vialactea_source->getColumnsNames());

    m_VisIVOTable->setWavelength(wavelen.toInt());

    QFileInfo infoFile = QFileInfo(f);

    queue = &Singleton<OperationQueue>::Instance();
    operation_queue_row=queue->addOperation("Importing "+infoFile.fileName());
    // queue->show();


    m_VisIVOTable->setName(infoFile.fileName().toStdString());
    m_VisIVOTable->setTableData(vialactea_source->getData());

    vtkwin=v;

    QString m_sSettingsFile = QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp").append("/setting.ini");


    QSettings settings(m_sSettingsFile, QSettings::NativeFormat);
    settings.setValue("vlkburl","http://vialactea:secret@ia2-vialactea.oats.inaf.it:8080/libjnifitsdb-1.0.2/");
    //    settings.setValue("vlkburl","http://vialactea:secret@ia2-vialactea.oats.inaf.it:8080/libjnifitsdb-0.22.0/");
    float  flux22Multiplier= 1;

    if(!isBandMergedCatalogue)
    {
        if(wavelen.compare("all")==0)
            vtkwin->addSourcesFromBM(m_VisIVOTable);
        else
            vtkwin->addSources(m_VisIVOTable);
    }
    else
    {
        firstSEDNode=true;
        double distances;

//        for(int  i = 0; i < 3; i++)

        for(int  i = 0; i < m_VisIVOTable->getNumberOfRows()/2; i++)
        {

            firstSEDNode=true;


            SED *sed=new SED();

            SEDNode *node;
            SEDNode *tmp_node;

            QString tmp;
            double flux,e_flux;

            double minAxes, maxAxes, angle, arcpix;

            double glon, glat;
            double *coord= new double[3];

            tmp=QString::fromStdString( m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("designation1100")][i]);
            //qDebug()<<"tmp 1100: "<<tmp;
            if(tmp!="missing")
            {
                firstSEDNode=false;
                flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux1100")][i].c_str());
                e_flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("err_flux1100")][i].c_str());
                if (flux>0)
                {
                    node=new SEDNode();
                    node->setDesignation(tmp);
                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setWavelength(1100);
                    sed->updateMaxFlux(node->getFlux());
                    sed->setRootNode(node);

                    glon=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon1100")][i].c_str());
                    glat=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat1100")][i].c_str());
                    node->setSky( glon , glat );
                    AstroUtils().sky2xy(vtkwin->filenameWithPath,glon,glat,coord);
                    node->setXY( coord[0] , coord[1] );
                    minAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhma1100")][i].c_str());
                    maxAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhmb1100")][i].c_str());
                    angle=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("pa1100")][i].c_str());




                    node->setEllipse(minAxes,maxAxes,angle,coord[2]);
                }
            }

            tmp=QString::fromStdString( m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("designation870")][i]);
            //qDebug()<<"tmp 870: "<<tmp;
            if(tmp.compare("missing")!=0)
            {
                //qDebug()<<"iun";
                flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux870")][i].c_str());
                e_flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("err_flux870")][i].c_str());
                tmp_node=node;
                if (flux>0)
                {
                    node=new SEDNode();
                    node->setDesignation(tmp);
                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setWavelength(870);
                    sed->updateMaxFlux(node->getFlux());



                    if(!firstSEDNode)
                    {

                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<node->getDesignation()<< " ha come padre "<<tmp_node->getDesignation();
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<tmp_node->getDesignation()<< " ha come figlio "<<node->getDesignation();
                        //qDebug ()<<" *****";

                        node->setParent(tmp_node);
                        tmp_node->setChild(node);
                    }
                    else
                    {
                        firstSEDNode=false;
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<node->getDesignation()<<" è radice";
                    }


                    glon=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon870")][i].c_str());
                    glat=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat870")][i].c_str());
                    node->setSky( glon , glat );

                    AstroUtils().sky2xy(vtkwin->filenameWithPath,glon,glat,coord);
                    node->setXY( coord[0] , coord[1] );

                    minAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhma870")][i].c_str());
                    maxAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhmb870")][i].c_str());
                    angle=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("pa870")][i].c_str());
                    node->setEllipse(minAxes,maxAxes,angle,coord[2]);

                    ////qDebug()<<"870_ flux:"<<flux;

                    if(!sed->hasRoot())
                        sed->setRootNode(node);
                }

            }

            tmp=QString::fromStdString( m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("designation500")][i]);
            //qDebug()<<"tmp 500: "<<tmp;
            if(tmp!="missing")
            {
                //firstSEDNode=false;
                flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux500")][i].c_str());
                e_flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("err_flux500")][i].c_str());
                tmp_node=node;

                if (flux>0)
                {
                    node=new SEDNode();
                    node->setDesignation(tmp);
                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setWavelength(500);
                    sed->updateMaxFlux(node->getFlux());
                    //sed->setRootNode(node);
                    if(!firstSEDNode)
                    {
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<node->getDesignation()<< " ha come padre "<<tmp_node->getDesignation();
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<tmp_node->getDesignation()<< " ha come figlio "<<node->getDesignation();
                        //qDebug ()<<" *****";
                        node->setParent(tmp_node);
                        tmp_node->setChild(node);
                    }
                    else
                        firstSEDNode=false;


                    glon=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon500")][i].c_str());
                    glat=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat500")][i].c_str());
                    node->setSky( glon , glat );
                    AstroUtils().sky2xy(vtkwin->filenameWithPath,glon,glat,coord);
                    node->setXY( coord[0] , coord[1] );
                    minAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhma500")][i].c_str());
                    maxAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhmb500")][i].c_str());
                    angle=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("pa500")][i].c_str());


                    ////qDebug()<<"500_  flux:"<<flux;

                    node->setEllipse(minAxes,maxAxes,angle,coord[2]);

                    if(!sed->hasRoot())
                        sed->setRootNode(node);
                }
            }

            tmp=QString::fromStdString( m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("designation350")][i]);
            //qDebug()<<"tmp 350: "<<tmp;
            if(tmp!="missing")
            {

                flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux350")][i].c_str());
                e_flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("err_flux350")][i].c_str());
                tmp_node=node;

                if (flux>0)
                {
                    node=new SEDNode();
                    node->setDesignation(tmp);
                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setWavelength(350);
                    sed->updateMaxFlux(node->getFlux());

                    if(!firstSEDNode)
                    {
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<node->getDesignation()<< " ha come padre "<<tmp_node->getDesignation();
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<tmp_node->getDesignation()<< " ha come figlio "<<node->getDesignation();
                        //qDebug ()<<" *****";
                        node->setParent(tmp_node);
                        tmp_node->setChild(node);

                    }
                    else
                        firstSEDNode=false;


                    glon=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon350")][i].c_str());
                    glat=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat350")][i].c_str());
                    node->setSky( glon , glat );

                    AstroUtils().sky2xy(vtkwin->filenameWithPath,glon,glat,coord);
                    node->setXY( coord[0] , coord[1] );

                    minAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhma350")][i].c_str());
                    maxAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhmb350")][i].c_str());
                    angle=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("pa350")][i].c_str());
                    node->setEllipse(minAxes,maxAxes,angle,coord[2]);

                    if(!sed->hasRoot())
                        sed->setRootNode(node);
                    ////qDebug()<<"350_  flux:"<<flux;
                }
            }

            tmp=QString::fromStdString( m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("designation250")][i]);
            //qDebug()<<"tmp 250: "<<tmp;
            if(tmp!="missing")
            {

                flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux250")][i].c_str());
                e_flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("err_flux250")][i].c_str());
                tmp_node=node;
                if (flux>0)
                {
                    node=new SEDNode();
                    node->setDesignation(tmp);

                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setWavelength(250);
                    sed->updateMaxFlux(node->getFlux());
                    sed->set250node();

                    if(!firstSEDNode)
                    {
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<node->getDesignation()<< " ha come padre "<<tmp_node->getDesignation();
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<tmp_node->getDesignation()<< " ha come figlio "<<node->getDesignation();
                        //qDebug ()<<" *****";
                        node->setParent(tmp_node);
                        tmp_node->setChild(node);
                    }
                    else
                        firstSEDNode=false;


                    glon=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon250")][i].c_str());
                    glat=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat250")][i].c_str());
                    node->setSky( glon , glat );
                    AstroUtils().sky2xy(vtkwin->filenameWithPath,glon,glat,coord);
                    node->setXY( coord[0] , coord[1] );

                    minAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhma250")][i].c_str());
                    maxAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhmb250")][i].c_str());
                    angle=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("pa250")][i].c_str());
                    node->setEllipse(minAxes,maxAxes,angle,coord[2]);

                    if(!sed->hasRoot())
                        sed->setRootNode(node);
                    ////qDebug()<<"250_  flux:"<<flux;
                }
            }

            tmp=QString::fromStdString( m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("designation160")][i]);
            //qDebug()<<"tmp 160: "<<tmp;
            if(tmp!="missing")
            {

                flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux160")][i].c_str());
                e_flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("err_flux160")][i].c_str());

                tmp_node=node;
                if (flux>0)
                {
                    node=new SEDNode();
                    node->setDesignation(tmp);

                    if(!firstSEDNode)
                    {
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<node->getDesignation()<< " ha come padre "<<tmp_node->getDesignation();
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<tmp_node->getDesignation()<< " ha come figlio "<<node->getDesignation();
                        //qDebug ()<<" *****";
                        node->setParent(tmp_node);
                        tmp_node->setChild(node);
                    }
                    else
                        firstSEDNode=false;

                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setWavelength(160);

                    sed->updateMaxFlux(node->getFlux());

                    glon=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon160")][i].c_str());
                    glat=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat160")][i].c_str());
                    node->setSky( glon , glat );
                    AstroUtils().sky2xy(vtkwin->filenameWithPath,glon,glat,coord);
                    node->setXY( coord[0] , coord[1] );

                    minAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhma160")][i].c_str());
                    maxAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhmb160")][i].c_str());
                    angle=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("pa160")][i].c_str());
                    node->setEllipse(minAxes,maxAxes,angle,coord[2]);

                    if(!sed->hasRoot())
                        sed->setRootNode(node);
                    ////qDebug()<<"160_  flux:"<<flux;
                }

            }

            tmp=QString::fromStdString( m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("designation70")][i]);
            //qDebug()<<"tmp 70: "<<tmp;
            if(tmp!="missing")
            {

                flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux70")][i].c_str());
                e_flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("err_flux70")][i].c_str());
                glon=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon70")][i].c_str());
                glat=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat70")][i].c_str());
                AstroUtils().sky2xy(vtkwin->filenameWithPath,glon,glat,coord);

                tmp_node=node;

                if (flux>0)
                {
                    node=new SEDNode();
                    node->setDesignation(tmp);

                    if(!firstSEDNode)
                    {
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<node->getDesignation()<< " ha come padre "<<tmp_node->getDesignation();
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<tmp_node->getDesignation()<< " ha come figlio "<<node->getDesignation();
                        //qDebug ()<<" *****";
                        node->setParent(tmp_node);
                        tmp_node->setChild(node);
                    }
                    else
                        firstSEDNode=false;

                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setSky( glon , glat );
                    node->setXY( coord[0] , coord[1] );
                    minAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhma70")][i].c_str());
                    maxAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhmb70")][i].c_str());
                    angle=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("pa70")][i].c_str());
                    node->setEllipse(minAxes,maxAxes,angle,coord[2]);
                    node->setWavelength(70);

                    if(!sed->hasRoot())
                        sed->setRootNode(node);

                    ////qDebug()<<"70_  flux:"<<flux;

                }

            }


            tmp=QString::fromStdString( m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("designation24")][i]);
            //qDebug()<<"tmp 24: "<<tmp;
            if(tmp!="missing")
            {

                flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux24")][i].c_str());
                e_flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("err_flux24")][i].c_str());
                glon=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon24")][i].c_str());
                glat=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat24")][i].c_str());
                AstroUtils().sky2xy(vtkwin->filenameWithPath,glon,glat,coord);

                tmp_node=node;
                if (flux>0)
                {
                    node=new SEDNode();
                    node->setDesignation(tmp);
                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setSky( glon , glat );
                    node->setXY( coord[0] , coord[1] );
                    minAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhma24")][i].c_str());
                    maxAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhmb24")][i].c_str());
                    angle=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("pa24")][i].c_str());
                    node->setEllipse(minAxes,maxAxes,angle,coord[2]);
                    node->setWavelength(24);
                    if(!firstSEDNode)
                    {
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<node->getDesignation()<< " ha come padre "<<tmp_node->getDesignation();
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<tmp_node->getDesignation()<< " ha come figlio "<<node->getDesignation();
                        //qDebug ()<<" *****";

                        node->setParent(tmp_node);
                        tmp_node->setChild(node);
                    }
                    else
                        firstSEDNode=false;




                    if(!sed->hasRoot())
                        sed->setRootNode(node);

                    ////qDebug()<<"24_  flux:"<<flux;
                }

            }


            tmp=QString::fromStdString( m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("designation22")][i]);
            //qDebug()<<"tmp 22: "<<tmp;
            if(tmp!="missing")
            {

                flux= flux22Multiplier*atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux22")][i].c_str());
                e_flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("err_flux22")][i].c_str());
                glon=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon22")][i].c_str());
                glat=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat22")][i].c_str());
                AstroUtils().sky2xy(vtkwin->filenameWithPath,glon,glat,coord);

                tmp_node=node;
                if (flux>0)
                {
                    node=new SEDNode();
                    node->setDesignation(tmp);
                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setSky( glon , glat );
                    node->setXY( coord[0] , coord[1] );
                    minAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhma22")][i].c_str());
                    maxAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhmb22")][i].c_str());
                    angle=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("pa22")][i].c_str());
                    node->setEllipse(minAxes,maxAxes,angle,coord[2]);
                    node->setWavelength(22);

                    if(!firstSEDNode)
                    {
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<node->getDesignation()<< " ha come padre "<<tmp_node->getDesignation();
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<tmp_node->getDesignation()<< " ha come figlio "<<node->getDesignation();
                        //qDebug ()<<" *****";

                        node->setParent(tmp_node);
                        tmp_node->setChild(node);
                    }
                    else
                        firstSEDNode=false;




                    if(!sed->hasRoot())
                        sed->setRootNode(node);

                    ////qDebug()<<"22_  flux:"<<flux;
                }
            }



            tmp=QString::fromStdString( m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("designation21")][i]);
            //qDebug()<<"tmp 21um: "<<tmp;
            if(tmp!="missing")
            {

                flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux21_e")][i].c_str());
                e_flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("err_flux21_e")][i].c_str());
                glon=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon21")][i].c_str());
                glat=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat21")][i].c_str());
                AstroUtils().sky2xy(vtkwin->filenameWithPath,glon,glat,coord);

                tmp_node=node;

                ////qDebug()<<"flux_e "<<flux;
                if (flux>0)
                {
                    node=new SEDNode();
                    node->setDesignation(tmp+"_e");

                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setSky( glon , glat );
                    node->setXY( coord[0] , coord[1] );
                    minAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhma21")][i].c_str());
                    maxAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhmb21")][i].c_str());
                    angle=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("pa21")][i].c_str());
                    node->setEllipse(minAxes,maxAxes,angle,coord[2]);
                    node->setWavelength(21);
                    if(!firstSEDNode)
                    {
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<node->getDesignation()<< " ha come padre "<<tmp_node->getDesignation();
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<tmp_node->getDesignation()<< " ha come figlio "<<node->getDesignation();
                        //qDebug ()<<" *****";

                        node->setParent(tmp_node);
                        tmp_node->setChild(node);
                    }
                    else
                        firstSEDNode=false;



                    if(!sed->hasRoot())
                        sed->setRootNode(node);

                    ////qDebug()<<"21_  flux:"<<flux;
                }

                flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux21_d")][i].c_str());
                e_flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("err_flux21_d")][i].c_str());

                ////qDebug()<<"flux_e "<<flux;

                tmp_node=node;
                if (flux>0)
                {

                    node=new SEDNode();
                    node->setDesignation(tmp+"_d");

                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setSky( glon , glat );
                    node->setXY( coord[0] , coord[1] );
                    node->setEllipse(minAxes,maxAxes,angle,coord[2]);
                    node->setWavelength(14.7);


                    if(!firstSEDNode)
                    {
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<node->getDesignation()<< " ha come padre "<<tmp_node->getDesignation();
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<tmp_node->getDesignation()<< " ha come figlio "<<node->getDesignation();
                        //qDebug ()<<" *****";

                        node->setParent(tmp_node);
                        tmp_node->setChild(node);
                    }
                    else
                        firstSEDNode=false;


                    if(!sed->hasRoot())
                        sed->setRootNode(node);

                    ////qDebug()<<"14.7_  flux:"<<flux;
                }
            }


            //12.1um
            tmp=QString::fromStdString( m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("designation21")][i]);
            //qDebug()<<"tmp 12.1: "<<tmp;

            if(tmp!="missing")
            {
                flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux21_c")][i].c_str());
                e_flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("err_flux21_c")][i].c_str());
                glon=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon21")][i].c_str());
                glat=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat21")][i].c_str());
                AstroUtils().sky2xy(vtkwin->filenameWithPath,glon,glat,coord);

                ////qDebug()<<"flux_c "<<flux;

                tmp_node=node;
                if (flux>0)
                {

                    node=new SEDNode();
                    node->setDesignation(tmp+"_c");
                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setSky( glon , glat );
                    node->setXY( coord[0] , coord[1] );
                    node->setEllipse(minAxes,maxAxes,angle,coord[2]);
                    node->setWavelength(12.1);
                    if(!firstSEDNode)
                    {

                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<node->getDesignation()<< " ha come padre "<<tmp_node->getDesignation();
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<tmp_node->getDesignation()<< " ha come figlio "<<node->getDesignation();
                        //qDebug ()<<" *****";

                        node->setParent(tmp_node);
                        tmp_node->setChild(node);
                    }
                    else
                        firstSEDNode=false;




                    if(!sed->hasRoot())
                        sed->setRootNode(node);

                    ////qDebug()<<"12.1_  flux:"<<flux;
                }
            }


            //12um
            tmp=QString::fromStdString( m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("designation22")][i]);
            //qDebug()<<"tmp 12: "<<tmp;

            if(tmp!="missing")
            {

                flux= flux22Multiplier*atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux12")][i].c_str());
                e_flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("err_flux12")][i].c_str());
                glon=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon22")][i].c_str());
                glat=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat22")][i].c_str());
                AstroUtils().sky2xy(vtkwin->filenameWithPath,glon,glat,coord);

                tmp_node=node;
                if (flux>0)
                {

                    node=new SEDNode();
                    node->setDesignation(tmp+"_12um");
                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setSky( glon , glat );
                    node->setXY( coord[0] , coord[1] );
                    minAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhma22")][i].c_str());
                    maxAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhmb22")][i].c_str());
                    angle=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("pa22")][i].c_str());
                    node->setEllipse(minAxes,maxAxes,angle,coord[2]);
                    node->setWavelength(12);

                    if(!firstSEDNode)
                    {
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<node->getDesignation()<< " ha come padre "<<tmp_node->getDesignation();
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<tmp_node->getDesignation()<< " ha come figlio "<<node->getDesignation();
                        //qDebug ()<<" *****";

                        node->setParent(tmp_node);
                        tmp_node->setChild(node);
                    }
                    else
                        firstSEDNode=false;






                    if(!sed->hasRoot())
                        sed->setRootNode(node);

                    ////qDebug()<<"12_  flux:"<<flux;
                }

            }


            //8.3um
            tmp=QString::fromStdString( m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("designation21")][i]);
            //qDebug()<<"tmp 8.3: "<<tmp;

            if(tmp!="missing")
            {

                flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux21_a")][i].c_str());
                e_flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("err_flux21_a")][i].c_str());
                glon=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon21")][i].c_str());
                glat=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat21")][i].c_str());
                AstroUtils().sky2xy(vtkwin->filenameWithPath,glon,glat,coord);

                ////qDebug()<<"flux_a "<<flux;

                tmp_node=node;
                if (flux>0)
                {

                    node=new SEDNode();
                    node->setDesignation(tmp+"_a");

                    if(!firstSEDNode)
                    {
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<node->getDesignation()<< " ha come padre "<<tmp_node->getDesignation();
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<tmp_node->getDesignation()<< " ha come figlio "<<node->getDesignation();
                        //qDebug ()<<" *****";

                        node->setParent(tmp_node);
                        tmp_node->setChild(node);
                    }
                    else
                        firstSEDNode=false;


                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setSky( glon , glat );
                    node->setXY( coord[0] , coord[1] );
                    minAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhma21")][i].c_str());
                    maxAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhmb21")][i].c_str());
                    angle=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("pa21")][i].c_str());

                    node->setEllipse(minAxes,maxAxes,angle,coord[2]);
                    node->setWavelength(8.3);

                    if(!sed->hasRoot())
                        sed->setRootNode(node);

                    ////qDebug()<<"8.3_  flux:"<<flux;
                }
            }

            //4.6um
            tmp=QString::fromStdString( m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("designation22")][i]);
            //qDebug()<<"tmp 4.6: "<<tmp;

            if(tmp!="missing")
            {

                flux= flux22Multiplier*atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux46")][i].c_str());
                //flux= flux22Multiplier*atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux12")][i].c_str());
                //  e_flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("err_flux12")][i].c_str());
                e_flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("err_flux46")][i].c_str());
                glon=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon22")][i].c_str());
                glat=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat22")][i].c_str());
                AstroUtils().sky2xy(vtkwin->filenameWithPath,glon,glat,coord);

                tmp_node=node;
                if (flux>0)
                {

                    node=new SEDNode();
                    node->setDesignation(tmp+"_4.6um");

                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setSky( glon , glat );
                    node->setXY( coord[0] , coord[1] );
                    minAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhma22")][i].c_str());
                    maxAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhmb22")][i].c_str());
                    angle=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("pa22")][i].c_str());

                    node->setEllipse(minAxes,maxAxes,angle,coord[2]);
                    node->setWavelength(4.6);

                    if(!firstSEDNode)
                    {
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<node->getDesignation()<< " ha come padre "<<tmp_node->getDesignation();
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<tmp_node->getDesignation()<< " ha come figlio "<<node->getDesignation();
                        //qDebug ()<<" *****";

                        node->setParent(tmp_node);
                        tmp_node->setChild(node);
                    }
                    else
                        firstSEDNode=false;



                    if(!sed->hasRoot())
                        sed->setRootNode(node);

                    ////qDebug()<<"4.6_  flux:"<<flux;
                }

            }

            //3.4um
            tmp=QString::fromStdString( m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("designation22")][i]);
            //qDebug()<<"tmp 3.4: "<<tmp;

            if(tmp!="missing")
            {

                flux= flux22Multiplier*atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux34")][i].c_str());
                e_flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("err_flux34")][i].c_str());
                // flux= flux22Multiplier*atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux22")][i].c_str());
                // e_flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("err_flux22")][i].c_str());
                glon=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon22")][i].c_str());
                glat=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat22")][i].c_str());
                AstroUtils().sky2xy(vtkwin->filenameWithPath,glon,glat,coord);


                tmp_node=node;
                if (flux>0)
                {

                    node=new SEDNode();
                    node->setDesignation(tmp+"_3.4um");
                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setSky( glon , glat );
                    node->setXY( coord[0] , coord[1] );
                    minAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhma22")][i].c_str());
                    maxAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhmb22")][i].c_str());
                    angle=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("pa22")][i].c_str());

                    node->setEllipse(minAxes,maxAxes,angle,coord[2]);
                    node->setWavelength(3.4);


                    if(!firstSEDNode)
                    {
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<node->getDesignation()<< " ha come padre "<<tmp_node->getDesignation();
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<tmp_node->getDesignation()<< " ha come figlio "<<node->getDesignation();
                        //qDebug ()<<" *****";
                        node->setParent(tmp_node);
                        tmp_node->setChild(node);
                    }
                    else
                        firstSEDNode=false;



                    if(!sed->hasRoot())
                        sed->setRootNode(node);

                    ////qDebug()<<"3.4_  flux:"<<flux;
                }

            }

            //2.2um
            tmp=QString::fromStdString( m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("designation22")][i]);
            //qDebug()<<"tmp 2.2: "<<tmp;

            if(tmp!="missing")
            {

                flux= flux22Multiplier*atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux2M_K")][i].c_str());
                e_flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux2M_K")][i].c_str());
                glon=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon22")][i].c_str());
                glat=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat22")][i].c_str());
                AstroUtils().sky2xy(vtkwin->filenameWithPath,glon,glat,coord);

                tmp_node=node;
                if (flux>0)
                {

                    node=new SEDNode();
                    node->setDesignation(tmp+"_2.2um");
                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setSky( glon , glat );
                    node->setXY( coord[0] , coord[1] );
                    minAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhma22")][i].c_str());
                    maxAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhmb22")][i].c_str());
                    angle=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("pa22")][i].c_str());

                    node->setEllipse(minAxes,maxAxes,angle,coord[2]);
                    node->setWavelength(2.2);
                    if(!firstSEDNode)
                    {
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<node->getDesignation()<< " ha come padre "<<tmp_node->getDesignation();
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<tmp_node->getDesignation()<< " ha come figlio "<<node->getDesignation();
                        //qDebug ()<<" *****";
                        node->setParent(tmp_node);
                        tmp_node->setChild(node);
                    }
                    else
                        firstSEDNode=false;




                    if(!sed->hasRoot())
                        sed->setRootNode(node);

                    ////qDebug()<<"2.2_  flux:"<<flux;
                }

            }


            //1.65um
            tmp=QString::fromStdString( m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("designation22")][i]);
            //qDebug()<<"tmp 1.65: "<<tmp;

            if(tmp!="missing")
            {

                flux= flux22Multiplier*atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux2M_H")][i].c_str());
                e_flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux2M_H")][i].c_str());
                glon=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon22")][i].c_str());
                glat=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat22")][i].c_str());
                AstroUtils().sky2xy(vtkwin->filenameWithPath,glon,glat,coord);

                tmp_node=node;
                if (flux>0)
                {

                    node=new SEDNode();
                    node->setDesignation(tmp+"_1.65um");
                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setSky( glon , glat );
                    node->setXY( coord[0] , coord[1] );
                    minAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhma22")][i].c_str());
                    maxAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhmb22")][i].c_str());
                    angle=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("pa22")][i].c_str());

                    node->setEllipse(minAxes,maxAxes,angle,coord[2]);
                    node->setWavelength(1.65);
                    if(!firstSEDNode)
                    {
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<node->getDesignation()<< " ha come padre "<<tmp_node->getDesignation();
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<tmp_node->getDesignation()<< " ha come figlio "<<node->getDesignation();
                        //qDebug ()<<" *****";
                        node->setParent(tmp_node);
                        tmp_node->setChild(node);
                    }
                    else
                        firstSEDNode=false;




                    if(!sed->hasRoot())
                        sed->setRootNode(node);

                    ////qDebug()<<"1.65_  flux:"<<flux;
                }

            }


            //1.25um
            tmp=QString::fromStdString( m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("designation22")][i]);
            //qDebug()<<"tmp 1.25: "<<tmp;

            if(tmp!="missing")
            {

                flux= flux22Multiplier*atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux2M_J")][i].c_str());
                e_flux= atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("flux2M_J")][i].c_str());
                glon=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glon22")][i].c_str());
                glat=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("glat22")][i].c_str());
                AstroUtils().sky2xy(vtkwin->filenameWithPath,glon,glat,coord);

                tmp_node=node;
                if (flux>0)
                {

                    node=new SEDNode();
                    node->setDesignation(tmp+"_1.25um");
                    node->setFlux(flux);
                    node->setErrFlux(e_flux);
                    node->setSky( glon , glat );
                    node->setXY( coord[0] , coord[1] );
                    minAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhma22")][i].c_str());
                    maxAxes=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("fwhmb22")][i].c_str());
                    angle=atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("pa22")][i].c_str());

                    node->setEllipse(minAxes,maxAxes,angle,coord[2]);
                    node->setWavelength(1.25);
                    if(!firstSEDNode)
                    {
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<node->getDesignation()<< " ha come padre "<<tmp_node->getDesignation();
                        //qDebug ()<<node->getWavelength()<<" Il nodo "<<tmp_node->getDesignation()<< " ha come figlio "<<node->getDesignation();
                        //qDebug ()<<" *****";
                        node->setParent(tmp_node);
                        tmp_node->setChild(node);
                    }
                    else
                        firstSEDNode=false;

                    if(!sed->hasRoot())
                        sed->setRootNode(node);
                }

            }


            sedlist.push_back(sed);
            distances= 1000*atof(m_VisIVOTable->getTableData()[m_VisIVOTable->getColId("distance")][i].c_str());



        }




        ViaLactea *vialactealWin = &Singleton<ViaLactea>::Instance();
        SEDVisualizerPlot *sedv= new SEDVisualizerPlot(sedlist,vtkwin,vialactealWin);
        //qDebug()<<"distances is "<<distances;
        if (distances==-9999)
            distances=2000;
        sedv->setDistances(distances);
        //SEDVisualizerPlot *sedv= new SEDVisualizerPlot(sed_list2,vtkwin,vialactealWin);
        sedv->show();


        // SEDVisualizer *sedv= new SEDVisualizer(sedlist);
        //  sedv->showMaximized();
    }

    queue->editOperation(operation_queue_row,"Completed");
    queue->close();
}

SEDNode* VisIVOImporterDesktop::bmContainsNode(QString nodename)
{
    SEDNode *node;
    if(!nodelist.contains(nodename))
    {
        node=new SEDNode();
        node->setDesignation(nodename);
        nodelist.insert(nodename, node);
    }
    else
    {
        node=nodelist.value(nodename);
    }
    return node;
}

VisIVOImporterDesktop::VisIVOImporterDesktop(QString t, QString f, TreeModel *m )
{
    m_impStatus = -1;
    /*  m_impStatus:
        1 : an error occurred
        0 : importing OK
    */

    model=m;
    fileName=f;
    infoFile = QFileInfo(fileName);
    type=t;
    isHigal=false;
    queue = &Singleton<OperationQueue>::Instance();

}

VisIVOImporterDesktop::VisIVOImporterDesktop(QString f, vtkwindow_new* v, bool isFilament, bool isBubble)
{


    QFileInfo infoFile = QFileInfo(f);


    //QUI


    headerFileName.append(QDir::homePath()+"/VisIVODesktopTemp/tmp_download/").append(infoFile.baseName()).append(".bin.head");
    binaryFileName.append(QDir::homePath()+"/VisIVODesktopTemp/tmp_download/").append(infoFile.baseName()).append(".bin");

    headerFile = new QFile(headerFileName);
    importedFile = new QFile(fileName);
    binaryFile = new QFile(binaryFileName);


    QString type="ascii";
    int errorCode;


    char *filepath = new char[f.toStdString().length() + 1];
    strcpy(filepath,f.toStdString().c_str());

    char *fileformat = new char[type.toStdString().length() + 1];
    strcpy(fileformat,type.toStdString().c_str());

 //   VisIVOImporter envVI1;
 //   VI_Init(&envVI1);

    QString outputPathString=QDir::homePath()+"/VisIVODesktopTemp/tmp_download/"+infoFile.baseName()+".bin";

    char *outputPath = new char[outputPathString.toStdString().length() + 1];
    strcpy(outputPath,outputPathString.toStdString().c_str());

 //   errorCode=VI_SetAtt(&envVI1,VI_SET_FFORMAT,fileformat);
 //   errorCode=VI_SetAtt(&envVI1,VI_SET_FILEPATH, filepath);
 //   errorCode=VI_SetAtt(&envVI1,VI_SET_OUTFILEVBT,outputPath);
 //   VI_Import(&envVI1);


    VialacteaSource *vialactea_source= new VialacteaSource(f.toStdString());
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
    operation_queue_row=queue->addOperation("Importing "+infoFile.fileName());


    m_VisIVOTable->setName(infoFile.fileName().toStdString());
    m_VisIVOTable->setTableData(vialactea_source->getData());

    vtkwin=v;
    ////qDebug()<<"num#### "<<m_VisIVOTable->getNumberOfRows();

    if (isFilament)
    {


        vtkwin->addFilaments(m_VisIVOTable);
    }
    else if (isBubble)
    {
        vtkwin->addBubble(m_VisIVOTable);
    }
    else
    {

        VisPoint *m_VisPointsObject = new VisPoint(m_VisIVOTable);

        m_VisPointsObject->setX("x");
        m_VisPointsObject->setY("y");
        m_VisPointsObject->setZ("z");
        m_VisPointsObject->setScale(true);

        vtkwindow_new *m_OldRenderingWindow = new vtkwindow_new(this, m_VisPointsObject);


    }

}

void VisIVOImporterDesktop::setVtkWin(vtkwindow_new* v)
{
    vtkwin=v;
}

void VisIVOImporterDesktop::setBm(bool bm)
{
    isBandMergedCatalogue=bm;
}

void VisIVOImporterDesktop::doImport(QString wavelen, bool usingAPI)
{

    wavelength=wavelen;
    operation_queue_row=queue->addOperation("Importing "+fileName);

    MainWindow *w = &Singleton<MainWindow>::Instance();
    if(!usingAPI)
    {
        if(type=="FITSIMG")
        {


            vtkSmartPointer<vtkFitsReader> fitsReader = vtkSmartPointer<vtkFitsReader>::New();
            fitsReader->SetFileName(fileName.toStdString());



            // Modify the TreeModel
            int rows = model->rowCount();
            model->insertRow(rows);
            // model->setFITSIMG(model->index(rows,0),imageActor);
            model->setFITSIMG(model->index(rows,0),fitsReader);
            model->setData(model->index(rows,0),QIcon( ":/icons/VME_IMAGE.bmp" ));
            model->setData(model->index(rows,1),fileName);
            // end: Modify the TreeModel

            queue->editOperation(operation_queue_row,"Completed");

            /*
        if(isHigal)
        {
            HiGal *higalWin = &Singleton<HiGal>::Instance();

            QTableWidget *table=higalWin->getTable(3);

            // Do whatever you want with it...
            // table->setPlainText("Updated Plain Text Edit);

            int id= table->rowCount();
            table->insertRow(id);


            QTableWidgetItem* wavelen = new QTableWidgetItem(wavelength);
            QTableWidgetItem* nameItem = new QTableWidgetItem(infoFile.fileName());
            table->setItem(id, 0, nameItem);
            table->setItem(id, 1, wavelen);


            higalWin->mapsImageData.insert(id,fitsReader);

        }
*/

            return;
        }
        if (type=="VTP")
        {
            queue->show();

            vtkSmartPointer<vtkXMLPolyDataReader> vtpReader =    vtkSmartPointer<vtkXMLPolyDataReader>::New();
            vtkSmartPointer<vtkPolyData> pPolyData =  vtkSmartPointer<vtkPolyData>::New();
            vtkSmartPointer<vtkPolyDataMapper> pMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
            vtkSmartPointer<vtkLODActor> pActor = vtkSmartPointer<vtkLODActor>::New();

            vtpReader->SetFileName(fileName.toStdString().c_str());
            vtpReader->Update();
            pPolyData=vtpReader->GetOutput();
            //  pMapper->SetInput(pPolyData);
            pMapper->SetInputData(pPolyData);
            pActor->SetMapper(pMapper);
            // willing to be moved

            // end: willing to be moved
            return;
        }
        if (type=="VTI")
        {
            queue->show();

            vtkSmartPointer<vtkXMLImageDataReader> vtiReader =    vtkSmartPointer<vtkXMLImageDataReader>::New();
            vtkSmartPointer<vtkFixedPointVolumeRayCastMapper> volumeMapper = vtkSmartPointer<vtkFixedPointVolumeRayCastMapper>::New();
            vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();

            vtkSmartPointer<vtkColorTransferFunction> colorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
            vtkSmartPointer<vtkPiecewiseFunction> opacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
            vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();


            vtiReader->SetFileName(fileName.toStdString().c_str());
            vtiReader->Update();

            volumeMapper->SetInputConnection(vtiReader->GetOutputPort());


            volumeMapper->SetImageSampleDistance(0.5);
            volume->SetMapper(volumeMapper);
            // willing to be moved
            // A smart Color map ...8  points...
            colorTransferFunction->AddRGBPoint(0,0,0,0);
            colorTransferFunction->AddRGBPoint(0.396415,1,0,0);
            colorTransferFunction->AddRGBPoint(0.531872,1,0,0);
            colorTransferFunction->AddRGBPoint(0.697211,0.901961,0,0);
            colorTransferFunction->AddRGBPoint(0.76494,0.901961,0.831891,0);
            colorTransferFunction->AddRGBPoint(0.824701,0.901961,0.831891,0);
            colorTransferFunction->AddRGBPoint(0.888446,0.901961,0.901961,0);
            colorTransferFunction->AddRGBPoint(1,1,1,1);
            /*
                         opacityTransferFunction->AddPoint(0,0);
                         opacityTransferFunction->AddPoint(0.396415,0);
                         opacityTransferFunction->AddPoint(0.531872,0.06);
                         opacityTransferFunction->AddPoint(0.697211,0.2);
                         opacityTransferFunction->AddPoint(0.76494,0.5);
                         opacityTransferFunction->AddPoint(0.824701,0.8);
                         opacityTransferFunction->AddPoint(0.888446,0.9);
                         opacityTransferFunction->AddPoint(1,1);
                      */
            double step=1.0/256;
            double opValue=0;
            for (double i=0; i<=1; i+=step)
            {
                opValue=(tanh(5*i-3)+1)/2.5;
                opacityTransferFunction->AddPoint(i,opValue);
            }
            volumeProperty->SetScalarOpacity(opacityTransferFunction);
            volumeProperty->ShadeOff();
            volumeProperty->SetInterpolationTypeToLinear();
            volumeProperty->SetColor(colorTransferFunction);
            volume->SetProperty(volumeProperty);

            // Modify the TreeModel
            int rows = model->rowCount();
            model->insertRow(rows);
            model->setVTI(model->index(rows,0),volume);
            model->setData(model->index(rows,0),QIcon( ":/icons/VME_VOLUME.bmp" ));
            model->setData(model->index(rows,1),fileName);
            // end: Modify the TreeModel
            return;
        }

        headerFileName.append(w->workingPath).append(QDir::separator()).append(infoFile.baseName()).append(".bin.head");
        binaryFileName.append(w->workingPath).append(QDir::separator()).append(infoFile.baseName()).append(".bin");

        headerFile = new QFile(headerFileName);
        importedFile = new QFile(fileName);
        binaryFile = new QFile(binaryFileName);


        if (!importedFile->exists()) {
            ////qDebug() << "VisIVOImporterDesktop::VisIVOImporterDesktop:  I can't open file";
            m_impStatus = 1;
            return;
        }
        else
        {
            //We 're importing a supported VisIVO File by using the canonical importer
            //The result if m_impStatus==0 will be VBT (VisIVO Binary Table)

            QProcess *myProcess = new QProcess(this->parent());
            QStringList arguments;
            connect(myProcess, SIGNAL(finished(int, QProcess::ExitStatus)),this, SLOT(onProcessFinished(int, QProcess::ExitStatus)));

            arguments << "--fformat" << type << "--out"<< binaryFileName << fileName;

            ////qDebug()<< "--fformat" << type << "--out"<< binaryFileName << fileName;

            myProcess->setStandardErrorFile(QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp").append("log_error.txt"), QIODevice::Append);
            myProcess->setStandardOutputFile(QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp").append("log_output.txt"), QIODevice::Append);
            myProcess->start(w->importer,arguments);

        }

    }
    else
    {


        headerFileName.append(QDir::homePath()+"/VisIVODesktopTemp/tmp_download/").append(infoFile.baseName()).append(".bin.head");
        binaryFileName.append(QDir::homePath()+"/VisIVODesktopTemp/tmp_download/").append(infoFile.baseName()).append(".bin");

        headerFile = new QFile(headerFileName);
        importedFile = new QFile(fileName);
        binaryFile = new QFile(binaryFileName);


        if (!importedFile->exists()) {
            ////qDebug() << "VisIVOImporterDesktop::VisIVOImporterDesktop:  I can't open file";
            m_impStatus = 1;
            return;
        }
        else
        {
            int errorCode;


            char *filepath = new char[fileName.toStdString().length() + 1];
            strcpy(filepath,fileName.toStdString().c_str());

            char *fileformat = new char[type.toStdString().length() + 1];
            strcpy(fileformat,type.toStdString().c_str());
            /*
            VisIVOImporter envVI1;
            VI_Init(&envVI1);

            QString outputPathString=QDir::homePath()+"/VisIVODesktopTemp/tmp_download/"+infoFile.baseName()+".bin";

            char *outputPath = new char[outputPathString.toStdString().length() + 1];
            std::strcpy(outputPath,outputPathString.toStdString().c_str());

            errorCode=VI_SetAtt(&envVI1,VI_SET_FFORMAT,fileformat);
            errorCode=VI_SetAtt(&envVI1,VI_SET_FILEPATH, filepath);
            errorCode=VI_SetAtt(&envVI1,VI_SET_OUTFILEVBT,outputPath);
            VI_Import(&envVI1);
y
            onProcessFinished(0,QProcess::NormalExit);
*/

        }


    }
}



void VisIVOImporterDesktop::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_impStatus = exitStatus;

    ////qDebug()<<"dentro onProcessFinished m_impStatus: "<<m_impStatus;

    if (!headerFile->exists() ) {
        ////qDebug() << "VisIVOImporterDesktop::VisIVOImporterDesktop: The header file wasn't generate" << headerFile->fileName();
        m_impStatus = 1;
        return;
    }
    if (!binaryFile->exists() ) {
        ////qDebug() << "VisIVOImporterDesktop::VisIVOImporterDesktop: The binary file wasn't generate" << binaryFile->fileName();
        m_impStatus = 1;
        return;
    }


    /*  m_impStatus:
                1 : an error occurred
                0 : OK
                */
    if ( !m_impStatus)
    {


        std::string file = binaryFileName.toStdString();
        std::string name = infoFile.baseName().toStdString();
        //   m_VisIVOTable = new VSTableDesktop(file,name,"" ,!isHigal);
        m_VisIVOTable = new VSTableDesktop(file,name,"" ,true);
        //    VSTableDesktop(std::string locator, std::string name = "", std::string description = "", bool statistic=true);




        // Modify the TreeModel
        QString tableName;
        bool result;
        QModelIndex  parentIndex;
        tableName=QString(m_VisIVOTable->getName().c_str());
        //parentIndex = model->index(0,0);

        ////qDebug()<<tableName;
        parentIndex=QModelIndex();
        int rows = model->rowCount(parentIndex);
        if (model->insertRows(rows,1,parentIndex)){
            result = model->setTable(model->index(rows,0,parentIndex),m_VisIVOTable); // We append a VSTable to the item
            if (!m_VisIVOTable->getIsVolume())  // The table is a Point Table
                result = model->setData(model->index(rows,0,parentIndex),QIcon( ":/icons/VBT_Table.bmp" ));
            else                        // The table is a Volume Table VBT_Volume
                result = model->setData(model->index(rows,0,parentIndex),QIcon( ":/icons/VBT_Volume.bmp" ));

            result = model->setData(model->index(rows,1,parentIndex),tableName);


            model->setLastInsertItem(model->index(rows,0,parentIndex));

            MainWindow *w = &Singleton<MainWindow>::Instance();
            w->ui->tabWidget->setCurrentWidget(w->ui->tabObjectTree);


        }

        //VIALACTEA
        if(isHigal)
        {

            //  ViaLactea *vialacteaWindow = &Singleton<ViaLactea>::Instance();

            if(!isBandMergedCatalogue)
                vtkwin->addSources(model->getTable(model->getLastInsertItem()));
            else
            {
                //tabella bm!
                ////qDebug()<<"tabella bm!";

                unsigned int *colList;
                colList = new unsigned int [5];


                // char **m_tableData=NULL;
                std::string **m_tableData=NULL;

                m_tableData = new std::string*[5];


                for(int  i = 0; i < 5; i++)
                {
                    m_tableData[i] = new std::string[m_VisIVOTable->getNumberOfRows()];
                }



                colList[0]=m_VisIVOTable->getColId("N70");
                colList[1]=m_VisIVOTable->getColId("N160");
                colList[2]=m_VisIVOTable->getColId("N250");
                colList[3]=m_VisIVOTable->getColId("N350");
                colList[4]=m_VisIVOTable->getColId("N500");

                m_VisIVOTable->getColumnString(colList,5,0,m_VisIVOTable->getNumberOfRows(),m_tableData);


                QHash <QString, int> bm500;
                QHash <QString, int> bm350;
                QHash <QString, int> bm250;
                QHash <QString, int> bm160;
                QHash <QString, int> bm70;

                for(int  i = 0; i < m_VisIVOTable->getNumberOfRows(); i++)
                {

                    bm500.insert( QString::fromStdString( m_tableData[4][i] ), i);
                    bm350.insert( QString::fromStdString( m_tableData[3][i] ), i);
                    bm250.insert( QString::fromStdString( m_tableData[2][i] ), i);
                    bm160.insert( QString::fromStdString( m_tableData[1][i] ), i);
                    bm70.insert ( QString::fromStdString( m_tableData[0][i] ), i);

                }

                m_VisIVOTable->setBandMerged500(bm500);
                m_VisIVOTable->setBandMerged350(bm350);
                m_VisIVOTable->setBandMerged250(bm250);
                m_VisIVOTable->setBandMerged160(bm160);
                m_VisIVOTable->setBandMerged70(bm70);


                //ViaLactea *vialactealWin = &Singleton<ViaLactea>::Instance();
                //vialactealWin->setBmTable(m_VisIVOTable);




            }



        }


        queue->editOperation(operation_queue_row,"Completed");

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
