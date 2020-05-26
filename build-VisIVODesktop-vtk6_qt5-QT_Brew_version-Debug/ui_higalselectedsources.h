/********************************************************************************
** Form generated from reading UI file 'higalselectedsources.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_HIGALSELECTEDSOURCES_H
#define UI_HIGALSELECTEDSOURCES_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_HigalSelectedSources
{
public:
    QVBoxLayout *verticalLayout_2;
    QTabWidget *tabWidget;
    QHBoxLayout *horizontalLayout;
    QPushButton *datasetButton;
    QPushButton *sedButton;
    QPushButton *plotButton;

    void setupUi(QWidget *HigalSelectedSources)
    {
        if (HigalSelectedSources->objectName().isEmpty())
            HigalSelectedSources->setObjectName(QString::fromUtf8("HigalSelectedSources"));
        HigalSelectedSources->resize(402, 416);
        verticalLayout_2 = new QVBoxLayout(HigalSelectedSources);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        tabWidget = new QTabWidget(HigalSelectedSources);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));

        verticalLayout_2->addWidget(tabWidget);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        datasetButton = new QPushButton(HigalSelectedSources);
        datasetButton->setObjectName(QString::fromUtf8("datasetButton"));

        horizontalLayout->addWidget(datasetButton);

        sedButton = new QPushButton(HigalSelectedSources);
        sedButton->setObjectName(QString::fromUtf8("sedButton"));

        horizontalLayout->addWidget(sedButton);

        plotButton = new QPushButton(HigalSelectedSources);
        plotButton->setObjectName(QString::fromUtf8("plotButton"));

        horizontalLayout->addWidget(plotButton);


        verticalLayout_2->addLayout(horizontalLayout);


        retranslateUi(HigalSelectedSources);

        tabWidget->setCurrentIndex(-1);


        QMetaObject::connectSlotsByName(HigalSelectedSources);
    } // setupUi

    void retranslateUi(QWidget *HigalSelectedSources)
    {
        HigalSelectedSources->setWindowTitle(QCoreApplication::translate("HigalSelectedSources", "Form", nullptr));
        datasetButton->setText(QCoreApplication::translate("HigalSelectedSources", "Dataset", nullptr));
        sedButton->setText(QCoreApplication::translate("HigalSelectedSources", "SED", nullptr));
        plotButton->setText(QCoreApplication::translate("HigalSelectedSources", "Plot", nullptr));
    } // retranslateUi

};

namespace Ui {
    class HigalSelectedSources: public Ui_HigalSelectedSources {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HIGALSELECTEDSOURCES_H
