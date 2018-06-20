#include "sed.h"
#include <QDebug>

SED::SED(QObject *parent) :
    QObject(parent)
{
    rootSetted=false;
}

void SED::setRootNode(SEDNode *node)
{
    rootNode=node;
    rootSetted=true;
}

void SED::updateMaxFlux(double f)
{
    if (f>maxFlux)
        maxFlux=f;
}

void SED::scaleFlux()
{
    /*
     * Alle tre lunghezze d’onda (250, 350, 500) si calcola il beam di Herschel come
     *
     *  beam(lambda)=lambda/14.
     * - Alle tre lunghezze d’onda si “circolarizza” la gaussiana come segue:
     *
     * fwhm_circ(lambda) = SQRT(fwhm_1(lambda)*fwhm_2(lambda))
     *
     * - Alle tre lunghezze d’onda si calcola il size deconvoluto della sorgente:
     *  fwhm_decon(lambda) = SQRT(fwhm_circ(lambda)^2-beam(lambda)^2).
     *  Occhio, noi questo passaggio lo facciamo in genere solo quando fwhm_circ(lambda) > SQRT(2.)*beam(lambda)
     *   altrimenti diciamo che la sorgente non è completamente risolta (e non facciamo lo scaling).
     *   - Infine, alle ultime due lunghezze d’onda (350, 500) operiamo lo scaling,
     *   a partire dal flusso originale dei cataloghi a singola banda:
     *    F_scal(lambda)= fwhm_decon(250)/fwhm_decon(lambda)*F_orig(lambda).
     *   In pratica, il fattore di scaling è il rapporto tra i size
     *   (circolarizzati e deconvoluti) a 250 e a 350-oppure-500.
     *   Questo rapporto è atteso essere < 1, ma in Hi-GAL capita di trovare di tutto,
     *   per cui quando il rapporto è > 1 lasciamo il flusso invariato e amen.
     */

    if(setted250)
    {

    }
}

/*
 void SED::printAll(SEDNode* node){
    if(node->getChild().count()!=0){
        qDebug()<<node->getDesignation();
        for(int i=0;i<node->getChild().count();i++)
        {
            printAll(node->getChild().values()[i]);
        }
    }else{
        qDebug()<<node->getDesignation();
    }
}
*/

void SED::printAll(SEDNode* node){
    QList<SEDNode *> stack;
    stack.push_back(node);
    while(!stack.isEmpty()){
        SEDNode * curr=stack.at(0);
        stack.removeFirst();
        qDebug()<<curr->getDesignation();
        for(int i=0;i<curr->getChild().count();i++)
        {
            stack.push_back(curr->getChild().values()[i]);
        }
    }
}


void SED::printSelf()
{
    printAll(rootNode);
    /*
    qDebug()<<"**********";
    qDebug()<<rootNode->getDesignation();
    for(int i=0;i<rootNode->getChild().count();i++ )
    {
        qDebug()<<"\t"<<rootNode->getChild().values()[i]->getDesignation();
        for(int j=0;j<rootNode->getChild().values()[i]->getChild().count();j++ )
        {
            qDebug()<<"\t\t"<<rootNode->getChild().values()[i]->getChild().values()[j]->getDesignation();

        }
    }*/
}

QDataStream &operator<<(QDataStream &out, SED* sed)
{
    qDebug() << "printing SED";
    /*
    out << sed->getRootNode();
    out << sed->getRootNode()->getChild().count();
    qDebug()<<"RootNode has childs: "<<QString::number(sed->getRootNode()->getChild().count());

     for(int i=0;i<sed->getRootNode()->getChild().count();i++ )
    {
        out<<sed->getRootNode()->getChild().values()[i];

        out<<sed->getRootNode()->getChild().values()[i]->getChild().count();

        qDebug()<<"Child "<<QString::number(i)<<" of RootNode has childs: "<<QString::number(sed->getRootNode()->getChild().values()[i]->getChild().count());
        for(int j=0;j<sed->getRootNode()->getChild().values()[i]->getChild().count();j++ )
        {
            out<<sed->getRootNode()->getChild().values()[i]->getChild().values()[j];
        }
    }
    */


    QList<SEDNode *> stack;
    stack.push_back(sed->getRootNode());
    out<<sed->getRootNode();

    while(!stack.isEmpty()){
        SEDNode * curr=stack.at(0);
        qDebug()<<curr->getDesignation();
        out<<curr->getChild().count();
        qDebug()<<"child: "<<curr->getChild().count();
        stack.removeFirst();

        for(int i=0;i<curr->getChild().count();i++)
        {
            out<<curr->getChild().values()[i];
            stack.push_back(curr->getChild().values()[i]);
        }
    }
    return out;
}


QDataStream &operator>>(QDataStream &in, SED* sed)
{
    qDebug()<<"reading SED";
    SEDNode *rootNode=new SEDNode();
    //sed=new SED();
    in >> rootNode;
    sed->setRootNode(rootNode);
    int count, count2;
    /*
    qDebug()<<sed->getRootNode()->getDesignation();

    in >> count;
    qDebug()<<"RootNode has childs: "<<QString::number(count);
    for(int i=0;i<count;i++ )
    {
        SEDNode *childNode=new SEDNode();
        in>>childNode;
        sed->getRootNode()->setChild(childNode);
        in >> count2;
        qDebug()<<"Child "<<QString::number(i)<<" of RootNode has childs: "<<QString::number(count2);
        for(int j=0;j<count2;j++ )
        {
            SEDNode *childNode=new SEDNode();
            in>>childNode;
            sed->getRootNode()->getChild().values()[i]->setChild(childNode);
        }
    }
    */

    QList<SEDNode *> stack;
    stack.push_back(sed->getRootNode());
    while(!stack.isEmpty()){
        SEDNode * curr=stack.at(0);
        in>>count;
        stack.removeFirst();
        for(int i=0;i<count;i++)
        {
            SEDNode* temp=new SEDNode();
            in >> temp;
            curr->setChild(temp);
            stack.push_back(temp);
        }
    }
    return in;
}

