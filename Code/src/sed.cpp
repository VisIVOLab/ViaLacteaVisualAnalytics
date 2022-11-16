#include "sed.h"
#include <QDebug>

SED::SED(QObject *parent) : QObject(parent)
{
    rootSetted = false;
}

void SED::setRootNode(SEDNode *node)
{
    rootNode = node;
    rootSetted = true;
}

void SED::updateMaxFlux(double f)
{
    if (f > maxFlux)
        maxFlux = f;
}

void SED::scaleFlux()
{
    if (setted250) { }
}

void SED::printAll(SEDNode *node)
{
    QList<SEDNode *> stack;
    stack.push_back(node);
    while (!stack.isEmpty()) {
        SEDNode *curr = stack.at(0);
        stack.removeFirst();
        qDebug() << curr->getDesignation();
        for (int i = 0; i < curr->getChild().count(); i++) {
            stack.push_back(curr->getChild().values()[i]);
        }
    }
}

void SED::printSelf()
{
    printAll(rootNode);
}

QDataStream &operator<<(QDataStream &out, SED *sed)
{
    QList<SEDNode *> stack;
    stack.push_back(sed->getRootNode());
    out << sed->getRootNode();

    while (!stack.isEmpty()) {
        SEDNode *curr = stack.at(0);
        qDebug() << curr->getDesignation();
        out << curr->getChild().count();
        stack.removeFirst();

        for (int i = 0; i < curr->getChild().count(); i++) {
            out << curr->getChild().values()[i];
            stack.push_back(curr->getChild().values()[i]);
        }
    }
    return out;
}

QDataStream &operator>>(QDataStream &in, SED *sed)
{
    SEDNode *rootNode = new SEDNode();
    in >> rootNode;
    sed->setRootNode(rootNode);
    int count;

    QList<SEDNode *> stack;
    stack.push_back(sed->getRootNode());
    while (!stack.isEmpty()) {
        SEDNode *curr = stack.at(0);
        in >> count;
        stack.removeFirst();
        for (int i = 0; i < count; i++) {
            SEDNode *temp = new SEDNode();
            in >> temp;
            curr->setChild(temp);
            stack.push_back(temp);
        }
    }
    return in;
}
