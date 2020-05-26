/********************************************************************************
** Form generated from reading UI file 'vtkfitstoolswidget.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VTKFITSTOOLSWIDGET_H
#define UI_VTKFITSTOOLSWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_vtkfitstoolswidget
{
public:
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QPushButton *selectButton;
    QPushButton *noSelectButton;
    QFrame *line;
    QLabel *label_2;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *addLocalSourcesPushButton;
    QPushButton *addRemoteSourcesPushButton;
    QLabel *label_4;
    QHBoxLayout *horizontalLayout_6;
    QPushButton *datacubeButton;
    QSpacerItem *horizontalSpacer;
    QTableWidget *addedSourcesListWidget;
    QFrame *line_2;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QComboBox *lutComboBox;
    QLabel *label_5;
    QComboBox *scaleComboBox;
    QFrame *line_3;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_3;
    QGroupBox *groupBox;
    QHBoxLayout *horizontalLayout_5;
    QRadioButton *galacticRadioButton;
    QRadioButton *eclipticRadioButton;
    QSpacerItem *verticalSpacer;

    void setupUi(QWidget *vtkfitstoolswidget)
    {
        if (vtkfitstoolswidget->objectName().isEmpty())
            vtkfitstoolswidget->setObjectName(QString::fromUtf8("vtkfitstoolswidget"));
        vtkfitstoolswidget->resize(564, 713);
        verticalLayout_2 = new QVBoxLayout(vtkfitstoolswidget);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        selectButton = new QPushButton(vtkfitstoolswidget);
        selectButton->setObjectName(QString::fromUtf8("selectButton"));

        horizontalLayout->addWidget(selectButton);

        noSelectButton = new QPushButton(vtkfitstoolswidget);
        noSelectButton->setObjectName(QString::fromUtf8("noSelectButton"));

        horizontalLayout->addWidget(noSelectButton);


        verticalLayout_2->addLayout(horizontalLayout);

        line = new QFrame(vtkfitstoolswidget);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        verticalLayout_2->addWidget(line);

        label_2 = new QLabel(vtkfitstoolswidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        verticalLayout_2->addWidget(label_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        addLocalSourcesPushButton = new QPushButton(vtkfitstoolswidget);
        addLocalSourcesPushButton->setObjectName(QString::fromUtf8("addLocalSourcesPushButton"));

        horizontalLayout_3->addWidget(addLocalSourcesPushButton);

        addRemoteSourcesPushButton = new QPushButton(vtkfitstoolswidget);
        addRemoteSourcesPushButton->setObjectName(QString::fromUtf8("addRemoteSourcesPushButton"));

        horizontalLayout_3->addWidget(addRemoteSourcesPushButton);


        verticalLayout_2->addLayout(horizontalLayout_3);

        label_4 = new QLabel(vtkfitstoolswidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        verticalLayout_2->addWidget(label_4);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        datacubeButton = new QPushButton(vtkfitstoolswidget);
        datacubeButton->setObjectName(QString::fromUtf8("datacubeButton"));

        horizontalLayout_6->addWidget(datacubeButton);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer);


        verticalLayout_2->addLayout(horizontalLayout_6);

        addedSourcesListWidget = new QTableWidget(vtkfitstoolswidget);
        if (addedSourcesListWidget->columnCount() < 3)
            addedSourcesListWidget->setColumnCount(3);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        addedSourcesListWidget->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        addedSourcesListWidget->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        addedSourcesListWidget->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        addedSourcesListWidget->setObjectName(QString::fromUtf8("addedSourcesListWidget"));

        verticalLayout_2->addWidget(addedSourcesListWidget);

        line_2 = new QFrame(vtkfitstoolswidget);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        verticalLayout_2->addWidget(line_2);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label = new QLabel(vtkfitstoolswidget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_2->addWidget(label);

        lutComboBox = new QComboBox(vtkfitstoolswidget);
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->addItem(QString());
        lutComboBox->setObjectName(QString::fromUtf8("lutComboBox"));

        horizontalLayout_2->addWidget(lutComboBox);

        label_5 = new QLabel(vtkfitstoolswidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        horizontalLayout_2->addWidget(label_5);

        scaleComboBox = new QComboBox(vtkfitstoolswidget);
        scaleComboBox->addItem(QString());
        scaleComboBox->addItem(QString());
        scaleComboBox->setObjectName(QString::fromUtf8("scaleComboBox"));

        horizontalLayout_2->addWidget(scaleComboBox);


        verticalLayout_2->addLayout(horizontalLayout_2);

        line_3 = new QFrame(vtkfitstoolswidget);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);

        verticalLayout_2->addWidget(line_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_3 = new QLabel(vtkfitstoolswidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_4->addWidget(label_3);

        groupBox = new QGroupBox(vtkfitstoolswidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        horizontalLayout_5 = new QHBoxLayout(groupBox);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        galacticRadioButton = new QRadioButton(groupBox);
        galacticRadioButton->setObjectName(QString::fromUtf8("galacticRadioButton"));
        galacticRadioButton->setChecked(true);

        horizontalLayout_5->addWidget(galacticRadioButton);

        eclipticRadioButton = new QRadioButton(groupBox);
        eclipticRadioButton->setObjectName(QString::fromUtf8("eclipticRadioButton"));

        horizontalLayout_5->addWidget(eclipticRadioButton);


        horizontalLayout_4->addWidget(groupBox);


        verticalLayout_2->addLayout(horizontalLayout_4);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);


        retranslateUi(vtkfitstoolswidget);

        lutComboBox->setCurrentIndex(3);
        scaleComboBox->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(vtkfitstoolswidget);
    } // setupUi

    void retranslateUi(QWidget *vtkfitstoolswidget)
    {
        vtkfitstoolswidget->setWindowTitle(QCoreApplication::translate("vtkfitstoolswidget", "Form", nullptr));
        selectButton->setText(QCoreApplication::translate("vtkfitstoolswidget", "start select", nullptr));
        noSelectButton->setText(QCoreApplication::translate("vtkfitstoolswidget", "stop select", nullptr));
        label_2->setText(QCoreApplication::translate("vtkfitstoolswidget", "Sources", nullptr));
        addLocalSourcesPushButton->setText(QCoreApplication::translate("vtkfitstoolswidget", "Add Local ", nullptr));
        addRemoteSourcesPushButton->setText(QCoreApplication::translate("vtkfitstoolswidget", "Add Remote", nullptr));
        label_4->setText(QCoreApplication::translate("vtkfitstoolswidget", "Datacube", nullptr));
        datacubeButton->setText(QCoreApplication::translate("vtkfitstoolswidget", "Add Remote", nullptr));
        QTableWidgetItem *___qtablewidgetitem = addedSourcesListWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("vtkfitstoolswidget", "Selected", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = addedSourcesListWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("vtkfitstoolswidget", "Name", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = addedSourcesListWidget->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("vtkfitstoolswidget", "Opacity", nullptr));
        label->setText(QCoreApplication::translate("vtkfitstoolswidget", "Palette", nullptr));
        lutComboBox->setItemText(0, QCoreApplication::translate("vtkfitstoolswidget", "Default", nullptr));
        lutComboBox->setItemText(1, QCoreApplication::translate("vtkfitstoolswidget", "Default Step", nullptr));
        lutComboBox->setItemText(2, QCoreApplication::translate("vtkfitstoolswidget", "EField", nullptr));
        lutComboBox->setItemText(3, QCoreApplication::translate("vtkfitstoolswidget", "Glow", nullptr));
        lutComboBox->setItemText(4, QCoreApplication::translate("vtkfitstoolswidget", "Gray", nullptr));
        lutComboBox->setItemText(5, QCoreApplication::translate("vtkfitstoolswidget", "MinMax", nullptr));
        lutComboBox->setItemText(6, QCoreApplication::translate("vtkfitstoolswidget", "PhysicsContour", nullptr));
        lutComboBox->setItemText(7, QCoreApplication::translate("vtkfitstoolswidget", "PureRed", nullptr));
        lutComboBox->setItemText(8, QCoreApplication::translate("vtkfitstoolswidget", "PureGreen", nullptr));
        lutComboBox->setItemText(9, QCoreApplication::translate("vtkfitstoolswidget", "PureBlue", nullptr));
        lutComboBox->setItemText(10, QCoreApplication::translate("vtkfitstoolswidget", "Run1", nullptr));
        lutComboBox->setItemText(11, QCoreApplication::translate("vtkfitstoolswidget", "Run2", nullptr));
        lutComboBox->setItemText(12, QCoreApplication::translate("vtkfitstoolswidget", "Sar", nullptr));
        lutComboBox->setItemText(13, QCoreApplication::translate("vtkfitstoolswidget", "Temperature", nullptr));
        lutComboBox->setItemText(14, QCoreApplication::translate("vtkfitstoolswidget", "TenStep", nullptr));
        lutComboBox->setItemText(15, QCoreApplication::translate("vtkfitstoolswidget", "VolRenGlow", nullptr));
        lutComboBox->setItemText(16, QCoreApplication::translate("vtkfitstoolswidget", "VolRenGreen", nullptr));
        lutComboBox->setItemText(17, QCoreApplication::translate("vtkfitstoolswidget", "VolRenRGB", nullptr));
        lutComboBox->setItemText(18, QCoreApplication::translate("vtkfitstoolswidget", "VolRenTwoLev", nullptr));
        lutComboBox->setItemText(19, QCoreApplication::translate("vtkfitstoolswidget", "AllYellow", nullptr));
        lutComboBox->setItemText(20, QCoreApplication::translate("vtkfitstoolswidget", "AllCyan", nullptr));
        lutComboBox->setItemText(21, QCoreApplication::translate("vtkfitstoolswidget", "AllViolet", nullptr));
        lutComboBox->setItemText(22, QCoreApplication::translate("vtkfitstoolswidget", "AllWhite", nullptr));
        lutComboBox->setItemText(23, QCoreApplication::translate("vtkfitstoolswidget", "AllBlack", nullptr));
        lutComboBox->setItemText(24, QCoreApplication::translate("vtkfitstoolswidget", "AllRed", nullptr));
        lutComboBox->setItemText(25, QCoreApplication::translate("vtkfitstoolswidget", "AllGreen", nullptr));
        lutComboBox->setItemText(26, QCoreApplication::translate("vtkfitstoolswidget", "AllBlu", nullptr));

        label_5->setText(QCoreApplication::translate("vtkfitstoolswidget", "Scale", nullptr));
        scaleComboBox->setItemText(0, QCoreApplication::translate("vtkfitstoolswidget", "Linear", nullptr));
        scaleComboBox->setItemText(1, QCoreApplication::translate("vtkfitstoolswidget", "Log", nullptr));

        scaleComboBox->setCurrentText(QCoreApplication::translate("vtkfitstoolswidget", "Linear", nullptr));
        label_3->setText(QCoreApplication::translate("vtkfitstoolswidget", "Wcs", nullptr));
        groupBox->setTitle(QString());
        galacticRadioButton->setText(QCoreApplication::translate("vtkfitstoolswidget", "Galactic", nullptr));
        eclipticRadioButton->setText(QCoreApplication::translate("vtkfitstoolswidget", "Ecliptic", nullptr));
    } // retranslateUi

};

namespace Ui {
    class vtkfitstoolswidget: public Ui_vtkfitstoolswidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VTKFITSTOOLSWIDGET_H
