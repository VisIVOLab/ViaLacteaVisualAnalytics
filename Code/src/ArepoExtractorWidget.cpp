#include "ArepoExtractorWidget.h"
#include "ui_ArepoExtractorWidget.h"

#include "loadingwidget.h"
#include "vtkfitsreader.h"
#include "vtkwindow_new.h"

#include <QDebug>
#include <QDir>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QTreeWidgetItem>

#include <H5Cpp.h>

ArepoExtractorWidget::ArepoExtractorWidget(const QString &filepath, QWidget *parent)
    : QWidget(parent), ui(new Ui::ArepoExtractorWidget), filepath(filepath)
{
    ui->setupUi(this);
    ui->treeArepo->setHeaderLabel("Contents");
    this->setWindowTitle(this->filepath);
    this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowFlag(Qt::Window);

    this->loadSimulation();

    ui->lineLevel->setValidator(new QIntValidator(ui->lineLevel));
    ui->lineSizeX->setValidator(new QDoubleValidator(ui->lineSizeX));
    ui->lineSizeY->setValidator(new QDoubleValidator(ui->lineSizeY));
    ui->lineSizeZ->setValidator(new QDoubleValidator(ui->lineSizeZ));
    ui->lineOffsetX->setValidator(new QDoubleValidator(ui->lineOffsetX));
    ui->lineOffsetY->setValidator(new QDoubleValidator(ui->lineOffsetY));
    ui->lineOffsetZ->setValidator(new QDoubleValidator(ui->lineOffsetZ));

    QObject::connect(ui->lineLevel, &QLineEdit::editingFinished, this,
                     &ArepoExtractorWidget::updatePhysicalResolution);
    QObject::connect(ui->lineSizeX, &QLineEdit::editingFinished, this,
                     &ArepoExtractorWidget::updatePhysicalResolution);
    QObject::connect(ui->lineSizeY, &QLineEdit::editingFinished, this,
                     &ArepoExtractorWidget::updatePhysicalResolution);
    QObject::connect(ui->lineSizeZ, &QLineEdit::editingFinished, this,
                     &ArepoExtractorWidget::updatePhysicalResolution);
    QObject::connect(ui->btnExtract, &QPushButton::clicked, this, &ArepoExtractorWidget::extract);
}

ArepoExtractorWidget::~ArepoExtractorWidget()
{
    delete ui;
}

void ArepoExtractorWidget::loadSimulation()
{
    H5::H5File file(this->filepath.toStdString(), H5F_ACC_RDONLY);

    try {
        H5::Group root = file.openGroup("/");
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeArepo, { "/" });
        this->buildTreeItem(root, item);
        ui->treeArepo->topLevelItem(0)->setExpanded(true);
        this->updatePhysicalResolution();

        // Box Size
        H5::Group parameters = root.openGroup("/Parameters");
        H5::Attribute boxAttr = parameters.openAttribute("BoxSize");
        double boxSize = 0.;
        if (boxAttr.getDataType().getSize() == sizeof(float)) {
            float tmp = 0.0f;
            boxAttr.read(H5::PredType::NATIVE_FLOAT, &tmp);
            boxSize = static_cast<double>(tmp);
        } else {
            boxAttr.read(H5::PredType::NATIVE_DOUBLE, &boxSize);
        }

        H5::Attribute unitAttr = parameters.openAttribute("UnitLength_in_cm");
        double unitLength = 0.0;
        unitAttr.read(H5::PredType::NATIVE_DOUBLE, &unitLength);
        const double kpc_in_cm = 3.085677581e21;
        const double box_kpc = boxSize * unitLength / kpc_in_cm;
        ui->labelBoxSize->setText(QString::number(box_kpc));
    } catch (const H5::Exception &ex) {
        QMessageBox::critical(this, "Error", "Error while loading " + this->filepath);
    }

    file.close();
}

void ArepoExtractorWidget::buildTreeItem(const H5::Group &group, QTreeWidgetItem *parent)
{
    this->buildTreeItemAttributes(group, parent);

    for (hsize_t i = 0; i < group.getNumObjs(); ++i) {
        std::string name = group.getObjnameByIdx(i);
        H5G_obj_t type = group.getObjTypeByIdx(i);

        QTreeWidgetItem *item =
                new QTreeWidgetItem(parent, QStringList(QString::fromStdString(name)));

        switch (type) {
        case H5G_GROUP: {
            H5::Group subgroup = group.openGroup(name);
            this->buildTreeItem(subgroup, item);
            break;
        }
        case H5G_DATASET: {
            H5::DataSet dataset = group.openDataSet(name);
            this->buildTreeItemAttributes(dataset, item);
            break;
        }
        default:
            item->setText(0, QString::fromStdString(name + " (not supported)"));
        }
    }
}

void ArepoExtractorWidget::buildTreeItemAttributes(const H5::H5Object &obj, QTreeWidgetItem *parent)
{
    for (int i = 0; i < obj.getNumAttrs(); ++i) {
        try {
            H5::Attribute attr = obj.openAttribute(i);
            std::string name = attr.getName();
            H5::DataType type = attr.getDataType();
            H5::DataSpace space = attr.getSpace();

            QString attrValue = QString::fromStdString(name) + " = ";

            if (type.getClass() == H5T_INTEGER) {
                size_t size = type.getSize();

                if (H5Tget_sign(type.getId()) == H5T_SGN_NONE) {
                    // Unsigned
                    if (size == 1) {
                        uint8_t value;
                        attr.read(type, &value);
                        attrValue += QString::number(value);
                    } else if (size == 4) {
                        uint32_t value;
                        attr.read(type, &value);
                        attrValue += QString::number(value);
                    } else if (size == 8) {
                        uint64_t value;
                        attr.read(type, &value);
                        attrValue += QString::number(value);
                    } else {
                        attrValue +=
                                "(unsigned int: size " + QString::number(size) + " not supported)";
                    }
                } else {
                    // Signed
                    if (size == 4) {
                        int32_t value;
                        attr.read(type, &value);
                        attrValue += QString::number(value);
                    } else if (size == 8) {
                        int64_t value;
                        attr.read(type, &value);
                        attrValue += QString::number(value);
                    } else {
                        attrValue +=
                                "(signed int: size " + QString::number(size) + " not supported)";
                    }
                }
            } else if (type.getClass() == H5T_FLOAT) {
                if (type.getSize() == sizeof(float)) {
                    float value = 0.0;
                    attr.read(type, &value);
                    attrValue += QString::number(value);
                } else {
                    double value = 0.0;
                    attr.read(type, &value);
                    attrValue += QString::number(value);
                }
            } else if (type.getClass() == H5T_STRING) {
                H5std_string value;
                attr.read(type, value);
                attrValue += "\"" + QString::fromStdString(value) + "\"";

            } else if (type.getClass() == H5T_ARRAY) {
                int ndims = space.getSimpleExtentNdims();
                hsize_t dims[10];
                space.getSimpleExtentDims(dims);

                H5::DataType baseType = ((H5::ArrayType &)type).getSuper();

                if (baseType.getClass() == H5T_FLOAT && ndims == 1) {
                    std::vector<double> data(dims[0]);
                    attr.read(type, data.data());
                    attrValue += "[";
                    for (hsize_t j = 0; j < dims[0]; ++j) {
                        attrValue += QString::number(data[j]);
                        if (j < dims[0] - 1)
                            attrValue += ", ";
                    }
                    attrValue += "]";
                } else if (baseType.getClass() == H5T_INTEGER && ndims == 1) {
                    std::vector<int> data(dims[0]);
                    attr.read(type, data.data());
                    attrValue += "[";
                    for (hsize_t j = 0; j < dims[0]; ++j) {
                        attrValue += QString::number(data[j]);
                        if (j < dims[0] - 1)
                            attrValue += ", ";
                    }
                    attrValue += "]";
                } else {
                    attrValue += "(array not supported)";
                }

            } else {
                attrValue += "(type not supported)";
            }

            new QTreeWidgetItem(parent, QStringList(attrValue));
        } catch (const H5::Exception &e) {
            new QTreeWidgetItem(parent, QStringList("! Error while reading attribute"));
        }
    }
}
void ArepoExtractorWidget::openSimulation(const QString &filepath)
{
    auto fitsReader_dc = vtkSmartPointer<vtkFitsReader>::New();
    fitsReader_dc->SetFileName(filepath.toStdString());
    new vtkwindow_new(this, fitsReader_dc, 1, nullptr);
}

void ArepoExtractorWidget::updatePhysicalResolution()
{
    ui->labelResX->setText(QString::number(ui->lineSizeX->text().toDouble()
                                           / std::pow(2., ui->lineLevel->text().toDouble()) * 1e3));
    ui->labelResY->setText(QString::number(ui->lineSizeY->text().toDouble()
                                           / std::pow(2., ui->lineLevel->text().toDouble()) * 1e3));
    ui->labelResZ->setText(QString::number(ui->lineSizeZ->text().toDouble()
                                           / std::pow(2., ui->lineLevel->text().toDouble()) * 1e3));
}

void ArepoExtractorWidget::extract()
{
    if (ui->lineLevel->text().toInt() > 8) {
        auto res =
                QMessageBox::question(this, "Arepo Extractor",
                                      "Level 9+ may take minutes to extract and results in a "
                                      "uniform cube of about 1+ GB.\n\nDo you want to continue?");
        if (res == QMessageBox::No) {
            return;
        }
    }

    if (ui->lineFields->text().isEmpty()) {
        QMessageBox::warning(this, "Arepo Extractor", "Type at least one field to extract.");
        return;
    }

    QString settingsFile = QDir::homePath().append("/VisIVODesktopTemp/setting.ini");
    QSettings settings(settingsFile, QSettings::IniFormat);
    QString pythonExe = settings.value("python.exe").toString();

    auto process = new QProcess(this);
    process->setProcessChannelMode(QProcess::MergedChannels);
    const QDir dir(QApplication::applicationDirPath());

    QStringList args;
    args << dir.absoluteFilePath("arepo-extractor.py") << "--level" << ui->lineLevel->text()
         << "--fields" << ui->lineFields->text() << "--particle" << ui->lineParticleType->text()
         << "--win_size" << ui->lineSizeX->text() << ui->lineSizeY->text() << ui->lineSizeZ->text()
         << "--offset" << ui->lineOffsetX->text() << ui->lineOffsetY->text()
         << ui->lineOffsetZ->text() << this->filepath;

    qDebug() << "args" << args;

    process->setWorkingDirectory(dir.absolutePath());

    auto loading = new LoadingWidget;
    loading->setText("Please wait...");
    loading->setLoadingProcess(process);
    loading->show();

    QObject::connect(
            process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [=](int exitCode, QProcess::ExitStatus exitStatus) {
                process->deleteLater();
                loading->deleteLater();
                qDebug() << exitStatus << exitCode;

                if (exitStatus != QProcess::NormalExit) {
                    QMessageBox::critical(this, "Arepo Extractor", process->errorString());
                    return;
                }

                if (exitCode != 0) {
                    QMessageBox::warning(this, "Arepo Extractor", process->readAll().trimmed());
                    return;
                }

                const QString filepath = process->readAll().trimmed().split('\n').last();

                auto res = QMessageBox::information(this, "Arepo Extractor",
                                                    "Success, output file\n: " + filepath,
                                                    QMessageBox::Open | QMessageBox::Ignore);
                if (res == QMessageBox::Open) {
                    this->openSimulation(filepath);
                }
            });
    process->start(pythonExe, args);
}