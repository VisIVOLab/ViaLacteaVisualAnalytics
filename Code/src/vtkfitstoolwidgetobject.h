#ifndef VTKFITSTOOLWIDGETOBJECT_H
#define VTKFITSTOOLWIDGETOBJECT_H

#include <QString>
#include <QTreeWidgetItem>
#include "vtkSmartPointer.h"
#include "vtkLODActor.h"
#include "vtkfitsreader.h"

class vtkfitstoolwidgetobject
{
public:
    vtkfitstoolwidgetobject(int t);
    int getType() {return type;}
    vtkfitstoolwidgetobject* getParent() {return parent;}
    void setTreeWidgetItem( QTreeWidgetItem *i);
    QTreeWidgetItem* getTreeWidgetItem( ){return item;}
    void setParentItem( vtkfitstoolwidgetobject *p);
    void setName( QString n);
    QString getName() {return name;}
    void setWavelength(QString w);
    QString getWavelength(){return wavelength;}
    void setActor(vtkSmartPointer<vtkLODActor> a);
    void setFitsReader(vtkSmartPointer<vtkFitsReader> f){fits=f;}
    vtkSmartPointer<vtkLODActor> getActor(){return actor;}
    vtkSmartPointer<vtkFitsReader> getFits(){return fits;}
    QString getLutScale() {return lutScale;}
    QString getLutType() {return lutType;}
    int getLayerNumber(){return layerNumber;}
    void setLutScale( QString l) {lutScale=l;}
    void setLutType( QString l) {lutType=l;}
    void setLayerNumber( int l) {layerNumber=l;}
    void setSpecies( QString l) {species=l;}
    void setTransition( QString l) {transition=l;}
    void setSurvey(QString l) {survey=l;}


    QString getSpecies( ) {return species;}
    QString getTransition( ) {return transition;}
    QString getSurvey() {return survey;}

private:
    int type;
    int layerNumber;
    QString name;
    QString species;
    QString transition;
    QString survey;
    QString path;
    QString wavelength;
    QTreeWidgetItem *item;
    vtkfitstoolwidgetobject *parent;
    vtkSmartPointer<vtkLODActor> actor;
    vtkSmartPointer<vtkFitsReader> fits;
    QString lutType;
    QString lutScale;

};

#endif // VTKFITSTOOLWIDGETOBJECT_H
