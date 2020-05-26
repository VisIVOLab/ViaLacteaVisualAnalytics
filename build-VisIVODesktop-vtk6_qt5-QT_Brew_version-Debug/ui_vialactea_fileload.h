/********************************************************************************
** Form generated from reading UI file 'vialactea_fileload.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VIALACTEA_FILELOAD_H
#define UI_VIALACTEA_FILELOAD_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Vialactea_FileLoad
{
public:
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLabel *filenameLabel;
    QSpacerItem *horizontalSpacer;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout_2;
    QRadioButton *catalogueRadioButton;
    QRadioButton *bandmergedradioButton;
    QRadioButton *mapRadioButton;
    QHBoxLayout *wavelenLayout;
    QLabel *waveLabel;
    QLineEdit *waveLineEdit;
    QSpacerItem *horizontalSpacer_2;
    QSpacerItem *verticalSpacer;
    QPushButton *okPushButton;

    void setupUi(QWidget *Vialactea_FileLoad)
    {
        if (Vialactea_FileLoad->objectName().isEmpty())
            Vialactea_FileLoad->setObjectName(QString::fromUtf8("Vialactea_FileLoad"));
        Vialactea_FileLoad->resize(400, 300);
        verticalLayout_2 = new QVBoxLayout(Vialactea_FileLoad);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(Vialactea_FileLoad);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        filenameLabel = new QLabel(Vialactea_FileLoad);
        filenameLabel->setObjectName(QString::fromUtf8("filenameLabel"));

        horizontalLayout->addWidget(filenameLabel);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout_2->addLayout(horizontalLayout);

        groupBox = new QGroupBox(Vialactea_FileLoad);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        horizontalLayout_2 = new QHBoxLayout(groupBox);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        catalogueRadioButton = new QRadioButton(groupBox);
        catalogueRadioButton->setObjectName(QString::fromUtf8("catalogueRadioButton"));
        catalogueRadioButton->setChecked(true);

        horizontalLayout_2->addWidget(catalogueRadioButton);

        bandmergedradioButton = new QRadioButton(groupBox);
        bandmergedradioButton->setObjectName(QString::fromUtf8("bandmergedradioButton"));

        horizontalLayout_2->addWidget(bandmergedradioButton);

        mapRadioButton = new QRadioButton(groupBox);
        mapRadioButton->setObjectName(QString::fromUtf8("mapRadioButton"));

        horizontalLayout_2->addWidget(mapRadioButton);


        verticalLayout_2->addWidget(groupBox);

        wavelenLayout = new QHBoxLayout();
        wavelenLayout->setObjectName(QString::fromUtf8("wavelenLayout"));
        waveLabel = new QLabel(Vialactea_FileLoad);
        waveLabel->setObjectName(QString::fromUtf8("waveLabel"));

        wavelenLayout->addWidget(waveLabel);

        waveLineEdit = new QLineEdit(Vialactea_FileLoad);
        waveLineEdit->setObjectName(QString::fromUtf8("waveLineEdit"));

        wavelenLayout->addWidget(waveLineEdit);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        wavelenLayout->addItem(horizontalSpacer_2);


        verticalLayout_2->addLayout(wavelenLayout);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);

        okPushButton = new QPushButton(Vialactea_FileLoad);
        okPushButton->setObjectName(QString::fromUtf8("okPushButton"));
        okPushButton->setFocusPolicy(Qt::ClickFocus);

        verticalLayout_2->addWidget(okPushButton);


        retranslateUi(Vialactea_FileLoad);

        QMetaObject::connectSlotsByName(Vialactea_FileLoad);
    } // setupUi

    void retranslateUi(QWidget *Vialactea_FileLoad)
    {
        Vialactea_FileLoad->setWindowTitle(QCoreApplication::translate("Vialactea_FileLoad", "Form", nullptr));
        label->setText(QCoreApplication::translate("Vialactea_FileLoad", "Filename:", nullptr));
        filenameLabel->setText(QCoreApplication::translate("Vialactea_FileLoad", "TextLabel", nullptr));
        groupBox->setTitle(QCoreApplication::translate("Vialactea_FileLoad", "Type:", nullptr));
        catalogueRadioButton->setText(QCoreApplication::translate("Vialactea_FileLoad", "Catalogue", nullptr));
        bandmergedradioButton->setText(QCoreApplication::translate("Vialactea_FileLoad", "Band-merged", nullptr));
        mapRadioButton->setText(QCoreApplication::translate("Vialactea_FileLoad", "Map", nullptr));
        waveLabel->setText(QCoreApplication::translate("Vialactea_FileLoad", "Wavelength: ", nullptr));
        okPushButton->setText(QCoreApplication::translate("Vialactea_FileLoad", "Ok", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Vialactea_FileLoad: public Ui_Vialactea_FileLoad {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VIALACTEA_FILELOAD_H
