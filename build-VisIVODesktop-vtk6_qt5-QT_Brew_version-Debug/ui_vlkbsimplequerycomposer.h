/********************************************************************************
** Form generated from reading UI file 'vlkbsimplequerycomposer.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VLKBSIMPLEQUERYCOMPOSER_H
#define UI_VLKBSIMPLEQUERYCOMPOSER_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_VLKBSimpleQueryComposer
{
public:
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *vlkbUrlLineEdit;
    QPushButton *connectPushButton;
    QGroupBox *tableGroupBox;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_2;
    QComboBox *tableComboBox;
    QGroupBox *tableListGroupBox;
    QVBoxLayout *verticalLayout;
    QLabel *label_5;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_4;
    QLineEdit *longMinLineEdit;
    QLabel *label_3;
    QLineEdit *longMaxLineEdit;
    QLabel *label_6;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_8;
    QLineEdit *latMinLineEdit;
    QLabel *label_7;
    QLineEdit *latMaxLineEdit;
    QCheckBox *savedatasetCheckBox;
    QHBoxLayout *horizontalLayout_5;
    QLabel *outputNameLabel;
    QLineEdit *outputNameLineEdit;
    QPushButton *navigateFSButtono;
    QSpacerItem *verticalSpacer;
    QPushButton *queryPushButton;

    void setupUi(QWidget *VLKBSimpleQueryComposer)
    {
        if (VLKBSimpleQueryComposer->objectName().isEmpty())
            VLKBSimpleQueryComposer->setObjectName(QString::fromUtf8("VLKBSimpleQueryComposer"));
        VLKBSimpleQueryComposer->resize(485, 395);
        verticalLayout_2 = new QVBoxLayout(VLKBSimpleQueryComposer);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(VLKBSimpleQueryComposer);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        vlkbUrlLineEdit = new QLineEdit(VLKBSimpleQueryComposer);
        vlkbUrlLineEdit->setObjectName(QString::fromUtf8("vlkbUrlLineEdit"));

        horizontalLayout->addWidget(vlkbUrlLineEdit);

        connectPushButton = new QPushButton(VLKBSimpleQueryComposer);
        connectPushButton->setObjectName(QString::fromUtf8("connectPushButton"));

        horizontalLayout->addWidget(connectPushButton);


        verticalLayout_2->addLayout(horizontalLayout);

        tableGroupBox = new QGroupBox(VLKBSimpleQueryComposer);
        tableGroupBox->setObjectName(QString::fromUtf8("tableGroupBox"));
        horizontalLayout_6 = new QHBoxLayout(tableGroupBox);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_2 = new QLabel(tableGroupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_6->addWidget(label_2);

        tableComboBox = new QComboBox(tableGroupBox);
        tableComboBox->setObjectName(QString::fromUtf8("tableComboBox"));
        tableComboBox->setEnabled(false);

        horizontalLayout_6->addWidget(tableComboBox);


        verticalLayout_2->addWidget(tableGroupBox);

        tableListGroupBox = new QGroupBox(VLKBSimpleQueryComposer);
        tableListGroupBox->setObjectName(QString::fromUtf8("tableListGroupBox"));
        verticalLayout = new QVBoxLayout(tableListGroupBox);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        label_5 = new QLabel(tableListGroupBox);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        verticalLayout->addWidget(label_5);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_4 = new QLabel(tableListGroupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_3->addWidget(label_4);

        longMinLineEdit = new QLineEdit(tableListGroupBox);
        longMinLineEdit->setObjectName(QString::fromUtf8("longMinLineEdit"));
        longMinLineEdit->setEnabled(false);

        horizontalLayout_3->addWidget(longMinLineEdit);

        label_3 = new QLabel(tableListGroupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_3->addWidget(label_3);

        longMaxLineEdit = new QLineEdit(tableListGroupBox);
        longMaxLineEdit->setObjectName(QString::fromUtf8("longMaxLineEdit"));
        longMaxLineEdit->setEnabled(false);

        horizontalLayout_3->addWidget(longMaxLineEdit);


        verticalLayout->addLayout(horizontalLayout_3);

        label_6 = new QLabel(tableListGroupBox);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        verticalLayout->addWidget(label_6);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_8 = new QLabel(tableListGroupBox);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        horizontalLayout_4->addWidget(label_8);

        latMinLineEdit = new QLineEdit(tableListGroupBox);
        latMinLineEdit->setObjectName(QString::fromUtf8("latMinLineEdit"));
        latMinLineEdit->setEnabled(false);

        horizontalLayout_4->addWidget(latMinLineEdit);

        label_7 = new QLabel(tableListGroupBox);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        horizontalLayout_4->addWidget(label_7);

        latMaxLineEdit = new QLineEdit(tableListGroupBox);
        latMaxLineEdit->setObjectName(QString::fromUtf8("latMaxLineEdit"));
        latMaxLineEdit->setEnabled(false);

        horizontalLayout_4->addWidget(latMaxLineEdit);


        verticalLayout->addLayout(horizontalLayout_4);


        verticalLayout_2->addWidget(tableListGroupBox);

        savedatasetCheckBox = new QCheckBox(VLKBSimpleQueryComposer);
        savedatasetCheckBox->setObjectName(QString::fromUtf8("savedatasetCheckBox"));

        verticalLayout_2->addWidget(savedatasetCheckBox);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        outputNameLabel = new QLabel(VLKBSimpleQueryComposer);
        outputNameLabel->setObjectName(QString::fromUtf8("outputNameLabel"));
        outputNameLabel->setEnabled(false);

        horizontalLayout_5->addWidget(outputNameLabel);

        outputNameLineEdit = new QLineEdit(VLKBSimpleQueryComposer);
        outputNameLineEdit->setObjectName(QString::fromUtf8("outputNameLineEdit"));
        outputNameLineEdit->setEnabled(false);

        horizontalLayout_5->addWidget(outputNameLineEdit);

        navigateFSButtono = new QPushButton(VLKBSimpleQueryComposer);
        navigateFSButtono->setObjectName(QString::fromUtf8("navigateFSButtono"));
        navigateFSButtono->setEnabled(false);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/FILE_OPEN.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        navigateFSButtono->setIcon(icon);

        horizontalLayout_5->addWidget(navigateFSButtono);


        verticalLayout_2->addLayout(horizontalLayout_5);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);

        queryPushButton = new QPushButton(VLKBSimpleQueryComposer);
        queryPushButton->setObjectName(QString::fromUtf8("queryPushButton"));
        queryPushButton->setEnabled(false);

        verticalLayout_2->addWidget(queryPushButton);


        retranslateUi(VLKBSimpleQueryComposer);

        QMetaObject::connectSlotsByName(VLKBSimpleQueryComposer);
    } // setupUi

    void retranslateUi(QWidget *VLKBSimpleQueryComposer)
    {
        VLKBSimpleQueryComposer->setWindowTitle(QCoreApplication::translate("VLKBSimpleQueryComposer", "Add compact sources to visualization", nullptr));
        label->setText(QCoreApplication::translate("VLKBSimpleQueryComposer", "VLKB URL:", nullptr));
        vlkbUrlLineEdit->setText(QCoreApplication::translate("VLKBSimpleQueryComposer", "http://ia2-vialactea.oats.inaf.it:8080/vlkb/", nullptr));
        connectPushButton->setText(QCoreApplication::translate("VLKBSimpleQueryComposer", "Connect", nullptr));
        tableGroupBox->setTitle(QCoreApplication::translate("VLKBSimpleQueryComposer", "Table selection:", nullptr));
        label_2->setText(QCoreApplication::translate("VLKBSimpleQueryComposer", "Table", nullptr));
        tableListGroupBox->setTitle(QCoreApplication::translate("VLKBSimpleQueryComposer", "Coordinates", nullptr));
        label_5->setText(QCoreApplication::translate("VLKBSimpleQueryComposer", "Longitude", nullptr));
        label_4->setText(QCoreApplication::translate("VLKBSimpleQueryComposer", "min:", nullptr));
        label_3->setText(QCoreApplication::translate("VLKBSimpleQueryComposer", "max:", nullptr));
        label_6->setText(QCoreApplication::translate("VLKBSimpleQueryComposer", "Latitude", nullptr));
        label_8->setText(QCoreApplication::translate("VLKBSimpleQueryComposer", "min:", nullptr));
        label_7->setText(QCoreApplication::translate("VLKBSimpleQueryComposer", "max:", nullptr));
        savedatasetCheckBox->setText(QCoreApplication::translate("VLKBSimpleQueryComposer", "Save dataset to disk", nullptr));
        outputNameLabel->setText(QCoreApplication::translate("VLKBSimpleQueryComposer", "Output name prefix:", nullptr));
        navigateFSButtono->setText(QString());
        queryPushButton->setText(QCoreApplication::translate("VLKBSimpleQueryComposer", "Query", nullptr));
    } // retranslateUi

};

namespace Ui {
    class VLKBSimpleQueryComposer: public Ui_VLKBSimpleQueryComposer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VLKBSIMPLEQUERYCOMPOSER_H
