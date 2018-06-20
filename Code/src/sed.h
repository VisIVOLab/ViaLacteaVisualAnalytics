#ifndef SED_H
#define SED_H

#include <QObject>
#include <QHash>
#include <QDataStream>
#include "sednode.h"

class SED : public QObject
{
    Q_OBJECT
public:
    explicit SED(QObject *parent = 0);
    void set21a( QHash <QString, int> bm){bm21a=bm;}
    void set21c( QHash <QString, int> bm){bm21c=bm;}
    void set21d( QHash <QString, int> bm){bm21d=bm;}
    void set21e( QHash <QString, int> bm){bm21e=bm;}
    void set21( QHash <QString, int> bm){bm21=bm;}
    void set22( QHash <QString, int> bm){bm22=bm;}
    void set24( QHash <QString, int> bm){bm24=bm;}
    void set70 ( QHash <QString, int> bm){bm70=bm; }
    void set160( QHash <QString, int> bm){bm160=bm;}
    void set250( QHash <QString, int> bm){bm250=bm;}
    void set350( QHash <QString, int> bm){bm350=bm;}
    void set500( QHash <QString, int> bm){bm500=bm;}
    void set870( QHash <QString, int> bm){bm870=bm;}
    void set1100( QHash <QString, int> bm){bm1100=bm;}
    void setRootNode(SEDNode *node);
    bool hasRoot(){return rootSetted;}
    bool has250(){return setted250;}
    void set250node( ){setted250=true;}
    void updateMaxFlux(double f);
    void printSelf();
    double getMaxFlux(){return maxFlux;}
    void scaleFlux();
    void printAll(SEDNode* node);

    SEDNode *   getRootNode(){return rootNode;}
    SEDNode *rootNode;
signals:

public slots:

private:
    QHash <QString, int> bm1100;
    QHash <QString, int> bm870;
    QHash <QString, int> bm500;
    QHash <QString, int> bm350;
    QHash <QString, int> bm250;
    QHash <QString, int> bm160;
    QHash <QString, int> bm70;
    QHash <QString, int> bm24;
    QHash <QString, int> bm22;
    QHash <QString, int> bm21;
    QHash <QString, int> bm21e;
    QHash <QString, int> bm21d;
    QHash <QString, int> bm21c;
    QHash <QString, int> bm21a;



    bool rootSetted;
    bool setted250;
    double maxFlux;


};

QDataStream &operator<<(QDataStream& out, SED* sed);
QDataStream &operator>>(QDataStream& in, SED* sed);

#endif // SED_H
