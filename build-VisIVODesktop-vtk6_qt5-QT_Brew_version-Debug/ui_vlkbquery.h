/********************************************************************************
** Form generated from reading UI file 'vlkbquery.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VLKBQUERY_H
#define UI_VLKBQUERY_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_VLKBQuery
{
public:

    void setupUi(QWidget *VLKBQuery)
    {
        if (VLKBQuery->objectName().isEmpty())
            VLKBQuery->setObjectName(QString::fromUtf8("VLKBQuery"));
        VLKBQuery->resize(400, 300);

        retranslateUi(VLKBQuery);

        QMetaObject::connectSlotsByName(VLKBQuery);
    } // setupUi

    void retranslateUi(QWidget *VLKBQuery)
    {
        VLKBQuery->setWindowTitle(QCoreApplication::translate("VLKBQuery", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class VLKBQuery: public Ui_VLKBQuery {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VLKBQUERY_H
