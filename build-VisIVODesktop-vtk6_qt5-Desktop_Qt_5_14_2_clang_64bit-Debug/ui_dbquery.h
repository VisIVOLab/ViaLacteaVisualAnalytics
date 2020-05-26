/********************************************************************************
** Form generated from reading UI file 'dbquery.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DBQUERY_H
#define UI_DBQUERY_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_dbquery
{
public:
    QVBoxLayout *verticalLayout;
    QGroupBox *SpaceGroupBox;
    QVBoxLayout *verticalLayout_4;
    QLabel *label_9;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_5;
    QLineEdit *lineEdit_l;
    QLineEdit *lineEdit_b;
    QLabel *label_6;
    QSpacerItem *horizontalSpacer_4;
    QSpacerItem *horizontalSpacer_5;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_7;
    QLineEdit *lineEdit_r;
    QLabel *label_8;
    QSpacerItem *horizontalSpacer;
    QSpacerItem *horizontalSpacer_2;
    QSpacerItem *horizontalSpacer_3;
    QGroupBox *SpectralGroupBox;
    QVBoxLayout *verticalLayout_3;
    QLabel *label_10;
    QHBoxLayout *horizontalLayout;
    QLabel *label_11;
    QLineEdit *lineEdit_vl;
    QLineEdit *lineEdit_vu;
    QLabel *label_12;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *SurveyHorizontalLayout;
    QLabel *label;
    QComboBox *comboBox_surveys;
    QHBoxLayout *SpeciesHorizontalLayout;
    QLabel *label_2;
    QComboBox *comboBox_species;
    QHBoxLayout *TransitionHorizontalLayout;
    QLabel *label_3;
    QComboBox *comboBox_transitions;
    QPushButton *queryPushButton;
    QHBoxLayout *horizontalLayout_4;
    QTableWidget *datacube_tableWidget;
    QPushButton *pushButton_map;
    QSpacerItem *verticalSpacer;

    void setupUi(QDialog *dbquery)
    {
        if (dbquery->objectName().isEmpty())
            dbquery->setObjectName(QString::fromUtf8("dbquery"));
        dbquery->resize(495, 592);
        verticalLayout = new QVBoxLayout(dbquery);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        SpaceGroupBox = new QGroupBox(dbquery);
        SpaceGroupBox->setObjectName(QString::fromUtf8("SpaceGroupBox"));
        verticalLayout_4 = new QVBoxLayout(SpaceGroupBox);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        label_9 = new QLabel(SpaceGroupBox);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        verticalLayout_4->addWidget(label_9);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_5 = new QLabel(SpaceGroupBox);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        horizontalLayout_2->addWidget(label_5);

        lineEdit_l = new QLineEdit(SpaceGroupBox);
        lineEdit_l->setObjectName(QString::fromUtf8("lineEdit_l"));
        lineEdit_l->setReadOnly(false);

        horizontalLayout_2->addWidget(lineEdit_l);

        lineEdit_b = new QLineEdit(SpaceGroupBox);
        lineEdit_b->setObjectName(QString::fromUtf8("lineEdit_b"));
        lineEdit_b->setReadOnly(false);

        horizontalLayout_2->addWidget(lineEdit_b);

        label_6 = new QLabel(SpaceGroupBox);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        horizontalLayout_2->addWidget(label_6);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_4);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_5);


        verticalLayout_4->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_7 = new QLabel(SpaceGroupBox);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        horizontalLayout_3->addWidget(label_7);

        lineEdit_r = new QLineEdit(SpaceGroupBox);
        lineEdit_r->setObjectName(QString::fromUtf8("lineEdit_r"));
        lineEdit_r->setReadOnly(false);

        horizontalLayout_3->addWidget(lineEdit_r);

        label_8 = new QLabel(SpaceGroupBox);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        horizontalLayout_3->addWidget(label_8);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_2);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_3);


        verticalLayout_4->addLayout(horizontalLayout_3);


        verticalLayout->addWidget(SpaceGroupBox);

        SpectralGroupBox = new QGroupBox(dbquery);
        SpectralGroupBox->setObjectName(QString::fromUtf8("SpectralGroupBox"));
        verticalLayout_3 = new QVBoxLayout(SpectralGroupBox);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        label_10 = new QLabel(SpectralGroupBox);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        verticalLayout_3->addWidget(label_10);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_11 = new QLabel(SpectralGroupBox);
        label_11->setObjectName(QString::fromUtf8("label_11"));

        horizontalLayout->addWidget(label_11);

        lineEdit_vl = new QLineEdit(SpectralGroupBox);
        lineEdit_vl->setObjectName(QString::fromUtf8("lineEdit_vl"));
        lineEdit_vl->setReadOnly(false);

        horizontalLayout->addWidget(lineEdit_vl);

        lineEdit_vu = new QLineEdit(SpectralGroupBox);
        lineEdit_vu->setObjectName(QString::fromUtf8("lineEdit_vu"));
        lineEdit_vu->setReadOnly(false);

        horizontalLayout->addWidget(lineEdit_vu);

        label_12 = new QLabel(SpectralGroupBox);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        horizontalLayout->addWidget(label_12);


        verticalLayout_3->addLayout(horizontalLayout);


        verticalLayout->addWidget(SpectralGroupBox);

        groupBox_3 = new QGroupBox(dbquery);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        verticalLayout_2 = new QVBoxLayout(groupBox_3);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        SurveyHorizontalLayout = new QHBoxLayout();
        SurveyHorizontalLayout->setObjectName(QString::fromUtf8("SurveyHorizontalLayout"));
        label = new QLabel(groupBox_3);
        label->setObjectName(QString::fromUtf8("label"));

        SurveyHorizontalLayout->addWidget(label);

        comboBox_surveys = new QComboBox(groupBox_3);
        comboBox_surveys->setObjectName(QString::fromUtf8("comboBox_surveys"));

        SurveyHorizontalLayout->addWidget(comboBox_surveys);


        verticalLayout_2->addLayout(SurveyHorizontalLayout);

        SpeciesHorizontalLayout = new QHBoxLayout();
        SpeciesHorizontalLayout->setObjectName(QString::fromUtf8("SpeciesHorizontalLayout"));
        label_2 = new QLabel(groupBox_3);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        SpeciesHorizontalLayout->addWidget(label_2);

        comboBox_species = new QComboBox(groupBox_3);
        comboBox_species->setObjectName(QString::fromUtf8("comboBox_species"));

        SpeciesHorizontalLayout->addWidget(comboBox_species);


        verticalLayout_2->addLayout(SpeciesHorizontalLayout);

        TransitionHorizontalLayout = new QHBoxLayout();
        TransitionHorizontalLayout->setObjectName(QString::fromUtf8("TransitionHorizontalLayout"));
        label_3 = new QLabel(groupBox_3);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        TransitionHorizontalLayout->addWidget(label_3);

        comboBox_transitions = new QComboBox(groupBox_3);
        comboBox_transitions->setObjectName(QString::fromUtf8("comboBox_transitions"));

        TransitionHorizontalLayout->addWidget(comboBox_transitions);


        verticalLayout_2->addLayout(TransitionHorizontalLayout);

        queryPushButton = new QPushButton(groupBox_3);
        queryPushButton->setObjectName(QString::fromUtf8("queryPushButton"));

        verticalLayout_2->addWidget(queryPushButton);


        verticalLayout->addWidget(groupBox_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));

        verticalLayout->addLayout(horizontalLayout_4);

        datacube_tableWidget = new QTableWidget(dbquery);
        datacube_tableWidget->setObjectName(QString::fromUtf8("datacube_tableWidget"));

        verticalLayout->addWidget(datacube_tableWidget);

        pushButton_map = new QPushButton(dbquery);
        pushButton_map->setObjectName(QString::fromUtf8("pushButton_map"));

        verticalLayout->addWidget(pushButton_map);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout->addItem(verticalSpacer);


        retranslateUi(dbquery);

        QMetaObject::connectSlotsByName(dbquery);
    } // setupUi

    void retranslateUi(QDialog *dbquery)
    {
        dbquery->setWindowTitle(QCoreApplication::translate("dbquery", "Datacube query", nullptr));
        SpaceGroupBox->setTitle(QCoreApplication::translate("dbquery", "Space axis", nullptr));
        label_9->setText(QCoreApplication::translate("dbquery", "Galactic Coordinates", nullptr));
        label_5->setText(QCoreApplication::translate("dbquery", "[l,b]", nullptr));
        lineEdit_l->setText(QCoreApplication::translate("dbquery", "180", nullptr));
        lineEdit_b->setText(QCoreApplication::translate("dbquery", "1", nullptr));
        label_6->setText(QCoreApplication::translate("dbquery", "[deg]", nullptr));
        label_7->setText(QCoreApplication::translate("dbquery", "[r]", nullptr));
        lineEdit_r->setText(QCoreApplication::translate("dbquery", "0.1", nullptr));
        label_8->setText(QCoreApplication::translate("dbquery", "[deg]", nullptr));
        SpectralGroupBox->setTitle(QCoreApplication::translate("dbquery", "Spectral axis", nullptr));
        label_10->setText(QCoreApplication::translate("dbquery", "Velocity bounds", nullptr));
        label_11->setText(QCoreApplication::translate("dbquery", "[vlow,vup]", nullptr));
        lineEdit_vl->setText(QCoreApplication::translate("dbquery", "-300.0", nullptr));
        lineEdit_vu->setText(QCoreApplication::translate("dbquery", "300.0", nullptr));
        label_12->setText(QCoreApplication::translate("dbquery", "Km/s", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("dbquery", "Select", nullptr));
        label->setText(QCoreApplication::translate("dbquery", "Survey", nullptr));
        label_2->setText(QCoreApplication::translate("dbquery", "Species", nullptr));
        label_3->setText(QCoreApplication::translate("dbquery", "Transition", nullptr));
        queryPushButton->setText(QCoreApplication::translate("dbquery", "Query", nullptr));
        pushButton_map->setText(QCoreApplication::translate("dbquery", "Datacubes List", nullptr));
    } // retranslateUi

};

namespace Ui {
    class dbquery: public Ui_dbquery {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DBQUERY_H
