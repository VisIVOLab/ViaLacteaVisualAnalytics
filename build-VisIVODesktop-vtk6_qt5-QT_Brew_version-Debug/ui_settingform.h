/********************************************************************************
** Form generated from reading UI file 'settingform.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTINGFORM_H
#define UI_SETTINGFORM_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
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

class Ui_SettingForm
{
public:
    QVBoxLayout *verticalLayout_2;
    QGroupBox *IdkGroupBox;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QLineEdit *IdlLineEdit;
    QPushButton *IdlPushButton;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_2;
    QLineEdit *TileLineEdit;
    QPushButton *TilePushButton;
    QHBoxLayout *horizontalLayout_3;
    QCheckBox *checkBox;
    QLineEdit *urlLineEdit;
    QGroupBox *groupBox_2;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_3;
    QLineEdit *glyphLineEdit;
    QSpacerItem *horizontalSpacer;
    QGroupBox *groupBox_3;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_6;
    QRadioButton *publicVLKB_radioButton;
    QRadioButton *privateVLKB_radioButton;
    QHBoxLayout *userPassLayout;
    QLabel *userLabel;
    QLineEdit *username_LineEdit;
    QSpacerItem *horizontalSpacer_2;
    QLabel *passLabel;
    QLineEdit *password_LineEdit;
    QGroupBox *groupBox_4;
    QHBoxLayout *horizontalLayout_7;
    QLabel *label_4;
    QLineEdit *lineEdit_2;
    QPushButton *workdirButton;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout;
    QPushButton *pushButton;
    QPushButton *OkPushButton;

    void setupUi(QWidget *SettingForm)
    {
        if (SettingForm->objectName().isEmpty())
            SettingForm->setObjectName(QString::fromUtf8("SettingForm"));
        SettingForm->resize(727, 501);
        verticalLayout_2 = new QVBoxLayout(SettingForm);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        IdkGroupBox = new QGroupBox(SettingForm);
        IdkGroupBox->setObjectName(QString::fromUtf8("IdkGroupBox"));
        horizontalLayout_2 = new QHBoxLayout(IdkGroupBox);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label = new QLabel(IdkGroupBox);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_2->addWidget(label);

        IdlLineEdit = new QLineEdit(IdkGroupBox);
        IdlLineEdit->setObjectName(QString::fromUtf8("IdlLineEdit"));

        horizontalLayout_2->addWidget(IdlLineEdit);

        IdlPushButton = new QPushButton(IdkGroupBox);
        IdlPushButton->setObjectName(QString::fromUtf8("IdlPushButton"));
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/icons/FILE_OPEN.bmp"), QSize(), QIcon::Normal, QIcon::Off);
        IdlPushButton->setIcon(icon);

        horizontalLayout_2->addWidget(IdlPushButton);


        verticalLayout_2->addWidget(IdkGroupBox);

        groupBox = new QGroupBox(SettingForm);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_4->addWidget(label_2);

        TileLineEdit = new QLineEdit(groupBox);
        TileLineEdit->setObjectName(QString::fromUtf8("TileLineEdit"));

        horizontalLayout_4->addWidget(TileLineEdit);

        TilePushButton = new QPushButton(groupBox);
        TilePushButton->setObjectName(QString::fromUtf8("TilePushButton"));
        TilePushButton->setIcon(icon);

        horizontalLayout_4->addWidget(TilePushButton);


        verticalLayout->addLayout(horizontalLayout_4);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        checkBox = new QCheckBox(groupBox);
        checkBox->setObjectName(QString::fromUtf8("checkBox"));
        checkBox->setChecked(true);

        horizontalLayout_3->addWidget(checkBox);

        urlLineEdit = new QLineEdit(groupBox);
        urlLineEdit->setObjectName(QString::fromUtf8("urlLineEdit"));

        horizontalLayout_3->addWidget(urlLineEdit);


        verticalLayout->addLayout(horizontalLayout_3);


        verticalLayout_2->addWidget(groupBox);

        groupBox_2 = new QGroupBox(SettingForm);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        horizontalLayout_5 = new QHBoxLayout(groupBox_2);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_3 = new QLabel(groupBox_2);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_5->addWidget(label_3);

        glyphLineEdit = new QLineEdit(groupBox_2);
        glyphLineEdit->setObjectName(QString::fromUtf8("glyphLineEdit"));

        horizontalLayout_5->addWidget(glyphLineEdit);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer);


        verticalLayout_2->addWidget(groupBox_2);

        groupBox_3 = new QGroupBox(SettingForm);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        verticalLayout_3 = new QVBoxLayout(groupBox_3);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        publicVLKB_radioButton = new QRadioButton(groupBox_3);
        publicVLKB_radioButton->setObjectName(QString::fromUtf8("publicVLKB_radioButton"));
        publicVLKB_radioButton->setChecked(true);

        horizontalLayout_6->addWidget(publicVLKB_radioButton);

        privateVLKB_radioButton = new QRadioButton(groupBox_3);
        privateVLKB_radioButton->setObjectName(QString::fromUtf8("privateVLKB_radioButton"));

        horizontalLayout_6->addWidget(privateVLKB_radioButton);


        verticalLayout_3->addLayout(horizontalLayout_6);

        userPassLayout = new QHBoxLayout();
        userPassLayout->setObjectName(QString::fromUtf8("userPassLayout"));
        userLabel = new QLabel(groupBox_3);
        userLabel->setObjectName(QString::fromUtf8("userLabel"));

        userPassLayout->addWidget(userLabel);

        username_LineEdit = new QLineEdit(groupBox_3);
        username_LineEdit->setObjectName(QString::fromUtf8("username_LineEdit"));

        userPassLayout->addWidget(username_LineEdit);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        userPassLayout->addItem(horizontalSpacer_2);

        passLabel = new QLabel(groupBox_3);
        passLabel->setObjectName(QString::fromUtf8("passLabel"));

        userPassLayout->addWidget(passLabel);

        password_LineEdit = new QLineEdit(groupBox_3);
        password_LineEdit->setObjectName(QString::fromUtf8("password_LineEdit"));
        password_LineEdit->setEchoMode(QLineEdit::Password);

        userPassLayout->addWidget(password_LineEdit);


        verticalLayout_3->addLayout(userPassLayout);


        verticalLayout_2->addWidget(groupBox_3);

        groupBox_4 = new QGroupBox(SettingForm);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        horizontalLayout_7 = new QHBoxLayout(groupBox_4);
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        label_4 = new QLabel(groupBox_4);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_7->addWidget(label_4);

        lineEdit_2 = new QLineEdit(groupBox_4);
        lineEdit_2->setObjectName(QString::fromUtf8("lineEdit_2"));

        horizontalLayout_7->addWidget(lineEdit_2);

        workdirButton = new QPushButton(groupBox_4);
        workdirButton->setObjectName(QString::fromUtf8("workdirButton"));
        workdirButton->setIcon(icon);

        horizontalLayout_7->addWidget(workdirButton);


        verticalLayout_2->addWidget(groupBox_4);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        pushButton = new QPushButton(SettingForm);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        horizontalLayout->addWidget(pushButton);

        OkPushButton = new QPushButton(SettingForm);
        OkPushButton->setObjectName(QString::fromUtf8("OkPushButton"));

        horizontalLayout->addWidget(OkPushButton);


        verticalLayout_2->addLayout(horizontalLayout);


        retranslateUi(SettingForm);

        QMetaObject::connectSlotsByName(SettingForm);
    } // setupUi

    void retranslateUi(QWidget *SettingForm)
    {
        SettingForm->setWindowTitle(QCoreApplication::translate("SettingForm", "Form", nullptr));
        IdkGroupBox->setTitle(QCoreApplication::translate("SettingForm", "IDL/GDL executable", nullptr));
        label->setText(QCoreApplication::translate("SettingForm", "Path", nullptr));
        IdlPushButton->setText(QString());
        groupBox->setTitle(QCoreApplication::translate("SettingForm", "Tile", nullptr));
        label_2->setText(QCoreApplication::translate("SettingForm", "Path", nullptr));
        TilePushButton->setText(QString());
        checkBox->setText(QCoreApplication::translate("SettingForm", "Online", nullptr));
        urlLineEdit->setText(QCoreApplication::translate("SettingForm", "http://visivo.oact.inaf.it/vialacteatiles/openlayers.html", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("SettingForm", "Glyph", nullptr));
        label_3->setText(QCoreApplication::translate("SettingForm", "max", nullptr));
        glyphLineEdit->setText(QCoreApplication::translate("SettingForm", "1000", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("SettingForm", "VLKB", nullptr));
        publicVLKB_radioButton->setText(QCoreApplication::translate("SettingForm", "Public", nullptr));
        privateVLKB_radioButton->setText(QCoreApplication::translate("SettingForm", "Private", nullptr));
        userLabel->setText(QCoreApplication::translate("SettingForm", "username", nullptr));
        passLabel->setText(QCoreApplication::translate("SettingForm", "password", nullptr));
        groupBox_4->setTitle(QCoreApplication::translate("SettingForm", "Working directory", nullptr));
        label_4->setText(QCoreApplication::translate("SettingForm", "Path", nullptr));
        lineEdit_2->setText(QCoreApplication::translate("SettingForm", "~/VisIVODesktopTemp/tmp_download", nullptr));
        workdirButton->setText(QString());
        pushButton->setText(QCoreApplication::translate("SettingForm", "Cancel", nullptr));
        OkPushButton->setText(QCoreApplication::translate("SettingForm", "Ok", nullptr));
    } // retranslateUi

};

namespace Ui {
    class SettingForm: public Ui_SettingForm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTINGFORM_H
