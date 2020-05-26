/********************************************************************************
** Form generated from reading UI file 'vtkfitstoolwidget_new.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VTKFITSTOOLWIDGET_NEW_H
#define UI_VTKFITSTOOLWIDGET_NEW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBox>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_vtkfitstoolwidget_new
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QToolBox *toolBox;
    QWidget *page;
    QHBoxLayout *horizontalLayout;
    QTreeWidget *layerTreeWidget;
    QWidget *widget;
    QVBoxLayout *verticalLayout_2;
    QGroupBox *infoGroupBox;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label;
    QLineEdit *nameLineEdit;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QLineEdit *wavelenghtLineEdit;
    QGroupBox *regulationGroupBox;
    QVBoxLayout *verticalLayout_4;
    QCheckBox *checkBox;
    QHBoxLayout *horizontalLayout_6;
    QLabel *label_3;
    QSlider *horizontalSlider;
    QPushButton *pushButton_4;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *pushButton_2;
    QPushButton *savePushButton;
    QPushButton *deletePushButton;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *vtkfitstoolwidget_new)
    {
        if (vtkfitstoolwidget_new->objectName().isEmpty())
            vtkfitstoolwidget_new->setObjectName(QString::fromUtf8("vtkfitstoolwidget_new"));
        vtkfitstoolwidget_new->resize(800, 600);
        centralwidget = new QWidget(vtkfitstoolwidget_new);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        toolBox = new QToolBox(centralwidget);
        toolBox->setObjectName(QString::fromUtf8("toolBox"));
        page = new QWidget();
        page->setObjectName(QString::fromUtf8("page"));
        page->setGeometry(QRect(0, 0, 776, 499));
        horizontalLayout = new QHBoxLayout(page);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        layerTreeWidget = new QTreeWidget(page);
        QTreeWidgetItem *__qtreewidgetitem = new QTreeWidgetItem();
        __qtreewidgetitem->setText(0, QString::fromUtf8("color"));
        layerTreeWidget->setHeaderItem(__qtreewidgetitem);
        layerTreeWidget->setObjectName(QString::fromUtf8("layerTreeWidget"));
        layerTreeWidget->setHeaderHidden(true);

        horizontalLayout->addWidget(layerTreeWidget);

        widget = new QWidget(page);
        widget->setObjectName(QString::fromUtf8("widget"));
        verticalLayout_2 = new QVBoxLayout(widget);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        infoGroupBox = new QGroupBox(widget);
        infoGroupBox->setObjectName(QString::fromUtf8("infoGroupBox"));
        verticalLayout_3 = new QVBoxLayout(infoGroupBox);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label = new QLabel(infoGroupBox);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_4->addWidget(label);

        nameLineEdit = new QLineEdit(infoGroupBox);
        nameLineEdit->setObjectName(QString::fromUtf8("nameLineEdit"));

        horizontalLayout_4->addWidget(nameLineEdit);


        verticalLayout_3->addLayout(horizontalLayout_4);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_2 = new QLabel(infoGroupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_2->addWidget(label_2);

        wavelenghtLineEdit = new QLineEdit(infoGroupBox);
        wavelenghtLineEdit->setObjectName(QString::fromUtf8("wavelenghtLineEdit"));

        horizontalLayout_2->addWidget(wavelenghtLineEdit);


        verticalLayout_3->addLayout(horizontalLayout_2);


        verticalLayout_2->addWidget(infoGroupBox);

        regulationGroupBox = new QGroupBox(widget);
        regulationGroupBox->setObjectName(QString::fromUtf8("regulationGroupBox"));
        verticalLayout_4 = new QVBoxLayout(regulationGroupBox);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        checkBox = new QCheckBox(regulationGroupBox);
        checkBox->setObjectName(QString::fromUtf8("checkBox"));

        verticalLayout_4->addWidget(checkBox);

        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        label_3 = new QLabel(regulationGroupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_6->addWidget(label_3);

        horizontalSlider = new QSlider(regulationGroupBox);
        horizontalSlider->setObjectName(QString::fromUtf8("horizontalSlider"));
        horizontalSlider->setOrientation(Qt::Horizontal);

        horizontalLayout_6->addWidget(horizontalSlider);


        verticalLayout_4->addLayout(horizontalLayout_6);

        pushButton_4 = new QPushButton(regulationGroupBox);
        pushButton_4->setObjectName(QString::fromUtf8("pushButton_4"));

        verticalLayout_4->addWidget(pushButton_4);


        verticalLayout_2->addWidget(regulationGroupBox);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        verticalLayout_2->addItem(verticalSpacer);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        pushButton_2 = new QPushButton(widget);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));

        horizontalLayout_3->addWidget(pushButton_2);

        savePushButton = new QPushButton(widget);
        savePushButton->setObjectName(QString::fromUtf8("savePushButton"));

        horizontalLayout_3->addWidget(savePushButton);


        verticalLayout_2->addLayout(horizontalLayout_3);

        deletePushButton = new QPushButton(widget);
        deletePushButton->setObjectName(QString::fromUtf8("deletePushButton"));
        deletePushButton->setAutoFillBackground(false);

        verticalLayout_2->addWidget(deletePushButton);


        horizontalLayout->addWidget(widget);

        toolBox->addItem(page, QString::fromUtf8("Layers"));

        verticalLayout->addWidget(toolBox);

        vtkfitstoolwidget_new->setCentralWidget(centralwidget);
        menubar = new QMenuBar(vtkfitstoolwidget_new);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 22));
        vtkfitstoolwidget_new->setMenuBar(menubar);
        statusbar = new QStatusBar(vtkfitstoolwidget_new);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        vtkfitstoolwidget_new->setStatusBar(statusbar);

        retranslateUi(vtkfitstoolwidget_new);

        toolBox->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(vtkfitstoolwidget_new);
    } // setupUi

    void retranslateUi(QMainWindow *vtkfitstoolwidget_new)
    {
        vtkfitstoolwidget_new->setWindowTitle(QCoreApplication::translate("vtkfitstoolwidget_new", "MainWindow", nullptr));
        QTreeWidgetItem *___qtreewidgetitem = layerTreeWidget->headerItem();
        ___qtreewidgetitem->setText(1, QCoreApplication::translate("vtkfitstoolwidget_new", "name", nullptr));
        infoGroupBox->setTitle(QCoreApplication::translate("vtkfitstoolwidget_new", "Layer info", nullptr));
        label->setText(QCoreApplication::translate("vtkfitstoolwidget_new", "Name:", nullptr));
        label_2->setText(QCoreApplication::translate("vtkfitstoolwidget_new", "Wavelength:", nullptr));
        regulationGroupBox->setTitle(QCoreApplication::translate("vtkfitstoolwidget_new", "Regulation", nullptr));
        checkBox->setText(QCoreApplication::translate("vtkfitstoolwidget_new", "Visible", nullptr));
        label_3->setText(QCoreApplication::translate("vtkfitstoolwidget_new", "Transparency: ", nullptr));
        pushButton_4->setText(QCoreApplication::translate("vtkfitstoolwidget_new", "Change color", nullptr));
        pushButton_2->setText(QCoreApplication::translate("vtkfitstoolwidget_new", "Cancel", nullptr));
        savePushButton->setText(QCoreApplication::translate("vtkfitstoolwidget_new", "Save", nullptr));
        deletePushButton->setText(QCoreApplication::translate("vtkfitstoolwidget_new", "Delete Layer", nullptr));
        toolBox->setItemText(toolBox->indexOf(page), QCoreApplication::translate("vtkfitstoolwidget_new", "Layers", nullptr));
    } // retranslateUi

};

namespace Ui {
    class vtkfitstoolwidget_new: public Ui_vtkfitstoolwidget_new {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VTKFITSTOOLWIDGET_NEW_H
