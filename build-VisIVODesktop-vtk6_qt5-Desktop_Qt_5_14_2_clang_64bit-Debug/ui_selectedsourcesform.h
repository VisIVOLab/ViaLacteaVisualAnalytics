/********************************************************************************
** Form generated from reading UI file 'selectedsourcesform.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SELECTEDSOURCESFORM_H
#define UI_SELECTEDSOURCESFORM_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SelectedSourcesForm
{
public:

    void setupUi(QWidget *SelectedSourcesForm)
    {
        if (SelectedSourcesForm->objectName().isEmpty())
            SelectedSourcesForm->setObjectName(QString::fromUtf8("SelectedSourcesForm"));
        SelectedSourcesForm->resize(400, 300);

        retranslateUi(SelectedSourcesForm);

        QMetaObject::connectSlotsByName(SelectedSourcesForm);
    } // setupUi

    void retranslateUi(QWidget *SelectedSourcesForm)
    {
        SelectedSourcesForm->setWindowTitle(QCoreApplication::translate("SelectedSourcesForm", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SelectedSourcesForm: public Ui_SelectedSourcesForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SELECTEDSOURCESFORM_H
