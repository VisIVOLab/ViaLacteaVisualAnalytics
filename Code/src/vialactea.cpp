#include "vialactea.h"
#include "ui_vialactea.h"

#include "aboutform.h"
#include "ArepoExtractorWidget.h"
#include "astroutils.h"
#include "fitsheadermodifierdialog.h"
#include "HiPS2FITSForm.h"
#include "mcutoutsummary.h"
#include "sed.h"
#include "sedvisualizerplot.h"
#include "sessionloader.h"
#include "settingform.h"
#include "simpleconesearchform.h"
#include "singleton.h"
#include "usertablewindow.h"
#include "vialacteainitialquery.h"
#include "vialacteastringdictwidget.h"
#include "VLKBInventoryTree.h"
#include "vlkbsimplequerycomposer.h"
#include "vtkwindowcube.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QWebChannel>

WebProcess::WebProcess(QObject *parent) : QObject(parent) { }

void WebProcess::jsCall(const QString &point, const QString &radius)
{
    emit processJavascript(point, radius);
}

const QString ViaLactea::VLKB_BASE_URL = "https://vlkb.ia2.inaf.it";

ViaLactea::ViaLactea(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::ViaLactea),
      settingsFile(QDir::homePath().append("/VisIVODesktopTemp/setting.ini")),
      settings(settingsFile, QSettings::IniFormat)
{

    ui->setupUi(this);

    ui->saveToDiskCheckBox->setVisible(false);
    ui->fileNameLineEdit->setVisible(false);
    ui->selectFsPushButton->setVisible(false);

    // Cleanup previous run tmp
    QDir dir_tmp(
            QDir::homePath().append(QDir::separator()).append("VisIVODesktopTemp/tmp_download"));
    foreach (QString dirFile, dir_tmp.entryList()) {
        dir_tmp.remove(dirFile);
    }

    updateVLKBSetting();

    tilePath =
            settings.value("tilepath", ViaLactea::VLKB_BASE_URL + "/panoramicview/openlayers.html")
                    .toString();
    ui->webView->load(QUrl::fromUserInput(tilePath));
    ui->webView->setContextMenuPolicy(Qt::CustomContextMenu);

    // create an object for javascript communication
    webobj = new WebProcess(this);
    connect(webobj, &WebProcess::processJavascript, this, &ViaLactea::on_webViewRegionSelected);

    QWebChannel *channel = new QWebChannel(this);
    channel->registerObject("webobj", webobj);
    ui->webView->page()->setWebChannel(channel);

    mapSurvey.insert(0, QPair<QString, QString>("MIPSGAL", "24 um"));
    mapSurvey.insert(1, QPair<QString, QString>("GLIMPSE I", "8.0 um"));
    mapSurvey.insert(2, QPair<QString, QString>("GLIMPSE I", "5.8 um"));
    mapSurvey.insert(3, QPair<QString, QString>("GLIMPSE I", "4.5 um"));
    mapSurvey.insert(4, QPair<QString, QString>("GLIMPSE I", "3.6 um"));
    mapSurvey.insert(5, QPair<QString, QString>("Hi-GAL Tiles", "500 um"));
    mapSurvey.insert(6, QPair<QString, QString>("Hi-GAL Tiles", "350 um"));
    mapSurvey.insert(7, QPair<QString, QString>("Hi-GAL Tiles", "250 um"));
    mapSurvey.insert(8, QPair<QString, QString>("Hi-GAL Tiles", "70 um"));
    mapSurvey.insert(9, QPair<QString, QString>("Hi-GAL Tiles", "160 um"));
    mapSurvey.insert(10, QPair<QString, QString>("WISE", "22 um"));
    mapSurvey.insert(11, QPair<QString, QString>("WISE", "12 um"));
    mapSurvey.insert(12, QPair<QString, QString>("WISE", "4.6 um"));
    mapSurvey.insert(13, QPair<QString, QString>("WISE", "3.4 um"));
    mapSurvey.insert(14, QPair<QString, QString>("ATLASGAL", "870 um"));
    mapSurvey.insert(15, QPair<QString, QString>("CSO BGPS", "1.1 mm"));
    mapSurvey.insert(16, QPair<QString, QString>("CORNISH", "5 GHz"));
}

ViaLactea::~ViaLactea()
{
    delete ui;
}

void ViaLactea::quitApp()
{
    // Problem not only in this
    QWebEnginePage *p = ui->webView->page();
    p->disconnect(ui->webView);
    delete p;
}

void ViaLactea::updateVLKBSetting()
{
    QString vlkburl = settings.value("vlkburl", "").toString();

    if (vlkburl.isEmpty()) {
        settings.setValue("vlkburl", VLKB_BASE_URL);
        settings.setValue("vlkbtableurl", VLKB_BASE_URL + "/tap");
    }

    settings.sync();
    VialacteaStringDictWidget *stringDictWidget = &Singleton<VialacteaStringDictWidget>::Instance();
    stringDictWidget->buildDict();
}

void ViaLactea::on_queryPushButton_clicked()
{
    double lon = ui->glonLineEdit->text().toDouble();
    double lat = ui->glatLineEdit->text().toDouble();
    double rad = ui->radiumLineEdit->text().toDouble();
    double dlon = ui->dlLineEdit->text().toDouble() * .5;
    double dlat = ui->dbLineEdit->text().toDouble() * .5;

    QList<QPair<QString, QString>> selectedSurvey;
    QList<QCheckBox *> allButtons = ui->surveySelectorGroupBox->findChildren<QCheckBox *>();
    for (int i = 0; i < allButtons.size(); ++i) {
        if (allButtons.at(i)->isChecked()) {
            selectedSurvey.append(mapSurvey.value(i));
        }
    }

    auto vq = new VialacteaInitialQuery();
    vq->setL(ui->glonLineEdit->text());
    vq->setB(ui->glatLineEdit->text());
    vq->setSpecies("Continuum");
    vq->setSurveyname(selectedSurvey.at(0).first);
    vq->setTransition(selectedSurvey.at(0).second);
    vq->setSelectedSurveyMap(selectedSurvey);

    QString pos;
    if (ui->radiumLineEdit->text() != "") {
        // circle
        pos = VialacteaInitialQuery::posCutoutString(lon, lat, rad);

        vq->setR(ui->radiumLineEdit->text());
        vq->searchRequest(lon, lat, rad);
    } else {
        // range
        double lon1 = lon - dlon;
        double lon2 = lon + dlon;
        double lat1 = lat - dlat;
        double lat2 = lat + dlat;
        pos = VialacteaInitialQuery::posCutoutString(lon1, lon2, lat1, lat2);

        vq->setDeltaRect(ui->dlLineEdit->text(), ui->dbLineEdit->text());
        vq->searchRequest(lon, lat, dlon, dlat);
    }

    connect(vq, &VialacteaInitialQuery::searchDoneVO, this,
            [this, selectedSurvey, vq, pos](const QByteArray &votable) {
                auto tree = new VLKBInventoryTree(votable, pos);
                tree->show();
                auto cutouts = tree->getList();

                connect(vq, &VialacteaInitialQuery::cutoutDone, this,
                        [this, tree](const QString &filepath, const Cutout &src) {
                            auto fits = vtkSmartPointer<vtkFitsReader>::New();
                            fits->SetFileName(filepath.toStdString());
                            fits->setSurvey(src.survey);
                            fits->setSpecies(src.species);
                            fits->setTransition(src.transition);
                            if (!tree->getLinkedWindow()) {
                                tree->setLinkedWindow(new vtkwindow_new(this, fits));
                            } else {
                                tree->getLinkedWindow()->addLayerImage(fits, src.survey,
                                                                       src.species, src.transition);
                            }
                        });

                for (const auto surveyList : selectedSurvey) {
                    QString reqSurvey = surveyList.first;
                    QString reqTransition = surveyList.second;
                    int bestOverlap = 4;
                    Cutout bestCutout;

                    for (const Cutout &cutout : cutouts) {
                        if (cutout.survey == reqSurvey && cutout.transition == reqTransition
                            && cutout.overlap < bestOverlap) {
                            bestCutout = cutout;
                        }
                    }

                    if (!bestCutout.ivoID.isEmpty()) {
                        vq->cutoutRequest(
                                bestCutout.ivoID,
                                QDir::home().absoluteFilePath("VisIVODesktopTemp/tmp_download"),
                                bestCutout.pos_cutout, bestCutout);
                    }
                }
            });
}

void ViaLactea::on_noneRadioButton_clicked(bool checked)
{
    if (checked) {
        ui->webView->page()->runJavaScript("activatePointSelection(false)");
        ui->webView->page()->runJavaScript("activateRectangularSelection(false)");
    }
}

void ViaLactea::on_saveToDiskCheckBox_clicked(bool checked)
{
    ui->fileNameLineEdit->setEnabled(checked);
    ui->selectFsPushButton->setEnabled(checked);
}

void ViaLactea::on_selectFsPushButton_clicked()
{
    QString fn =
            QFileDialog::getSaveFileName(this, "Save as...", QString(), "Fits images (*.fits)");

    if (!fn.isEmpty() && !fn.endsWith(".fits", Qt::CaseInsensitive))
        fn += ".fits"; // default
    ui->fileNameLineEdit->setText(fn);
}

void ViaLactea::on_webViewRegionSelected(const QString &point, const QString &area)
{
    if (!point.isEmpty()) {
        QStringList pieces = point.split(",");
        ui->glonLineEdit->setText(QString::number(pieces[0].toDouble(), 'f', 4));
        ui->glatLineEdit->setText(QString::number(pieces[1].toDouble(), 'f', 4));
        if (ui->radiumLineEdit->text() == "")
            ui->radiumLineEdit->setText("0.1");
        ui->dlLineEdit->setText("");
        ui->dbLineEdit->setText("");

        ui->noneRadioButton->setChecked(true);
        on_noneRadioButton_clicked(true);
    }

    if (!area.isEmpty()) {
        QStringList pieces = area.split(",");
        QString dl = QString::number(pieces[0].toDouble(), 'f', 4);
        if (dl.toDouble() > 4.0)
            dl = QString::number(4.0, 'f', 4);
        QString db = QString::number(pieces[1].toDouble(), 'f', 4);
        if (db.toDouble() > 4.0)
            db = QString::number(4.0, 'f', 4);
        ui->dlLineEdit->setText(dl);
        ui->dbLineEdit->setText(db);
        ui->radiumLineEdit->setText("");
    }
}

void ViaLactea::on_glonLineEdit_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    queryButtonStatusOnOff();
}

void ViaLactea::on_glatLineEdit_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    queryButtonStatusOnOff();
}

void ViaLactea::on_radiumLineEdit_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    queryButtonStatusOnOff();
}

void ViaLactea::queryButtonStatusOnOff()
{
    if (ui->glatLineEdit->text() != "" && ui->glonLineEdit->text() != ""
        && (ui->radiumLineEdit->text() != ""
            || (ui->dlLineEdit->text() != "" && ui->dbLineEdit->text() != "")))
        ui->queryPushButton->setEnabled(true);
    else
        ui->queryPushButton->setEnabled(false);
}

void ViaLactea::openLocalImage(const QString &fn)
{
    auto fits = vtkSmartPointer<vtkFitsReader>::New();
    fits->SetFileName(fn.toStdString());

    bool doSearch = settings.value("vlkb.search", false).toBool();

    if (doSearch) {
        double coords[2], rectSize[2];
        AstroUtils::GetCenterCoords(fn.toStdString(), coords);
        AstroUtils::GetRectSize(fn.toStdString(), rectSize);

        VialacteaInitialQuery *vq = new VialacteaInitialQuery;
        QString pos = VialacteaInitialQuery::posCutoutString(coords[0], coords[1], rectSize[0]);
        connect(vq, &VialacteaInitialQuery::searchDoneVO, this,
                [vq, fn, fits, pos, this](const QByteArray &votable) {
                    auto tree = new VLKBInventoryTree(votable, pos);
                    tree->show();

                    auto win = new vtkwindow_new(this, fits);
                    tree->setLinkedWindow(win);
                    vq->deleteLater();
                });

        vq->searchRequest(coords[0], coords[1], rectSize[0]);
    } else {
        new vtkwindow_new(this, fits);
    }
}

void ViaLactea::on_actionSettings_triggered()
{
    if (!settingForm) {
        settingForm = new SettingForm(this);
    }
    settingForm->show();
    settingForm->activateWindow();
    settingForm->raise();
}

void ViaLactea::reload()
{
    tilePath = settings.value("tilepath", "").toString();
    ui->webView->load(QUrl::fromUserInput(tilePath));
    updateVLKBSetting();
}

void ViaLactea::sessionScan(const QString &currentDir, const QDir &rootDir, QStringList &results)
{
    QDir dir(currentDir);

    foreach (auto &&file, dir.entryInfoList({ "*.fit", "*.fits" }, QDir::Files)) {
        results << rootDir.relativeFilePath(file.absoluteFilePath());
    }

    foreach (auto &&subDir, dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        sessionScan(subDir.absoluteFilePath(), rootDir, results);
    }
}

void ViaLactea::openLocalDC(const QString &fn)
{
    // Check if the fits is a simcube
    auto fitsReader_dc = vtkSmartPointer<vtkFitsReader>::New();
    fitsReader_dc->SetFileName(fn.toStdString());
    if (fitsReader_dc->ctypeXY) {
        // Check if the keywords in the header are present
        std::list<std::string> missing;
        try {
            if (!AstroUtils::checkSimCubeHeader(fn.toStdString(), missing)) {
                auto res = QMessageBox::warning(this, "Warning",
                                                "The header does not contain all the necessary "
                                                "keywords!\nDo you want to add them?",
                                                QMessageBox::Yes | QMessageBox::No);

                if (res == QMessageBox::Yes) {
                    QStringList qMissing;
                    std::for_each(missing.cbegin(), missing.cend(), [&](const std::string &key) {
                        qMissing << QString::fromStdString(key);
                    });
                    auto dialog = new FitsHeaderModifierDialog(fn, qMissing);
                    dialog->show();
                    return;
                }
            }
        } catch (const std::exception &e) {
            QMessageBox::critical(this, "Error", QString::fromUtf8(e.what()));
            return;
        }

        new vtkwindow_new(this, fitsReader_dc, 1, nullptr);
        return;
    }

    long maxSize = settings.value("downscaleSize", 1024).toInt() * 1024; // MB->KB
    long size = QFileInfo(fn).size() / 1024; // B -> KB
    int ScaleFactor = AstroUtils::calculateResizeFactor(size, maxSize);

    vtkWindowCube *win = new vtkWindowCube(nullptr, fn, ScaleFactor);
    win->show();
    win->activateWindow();
    win->raise();

    bool doSearch = settings.value("vlkb.search", false).toBool();

    if (doSearch) {
        double coords[2], rectSize[2];
        AstroUtils::GetCenterCoords(fn.toStdString(), coords);
        AstroUtils::GetRectSize(fn.toStdString(), rectSize);

        VialacteaInitialQuery *vq = new VialacteaInitialQuery;
        QString pos = VialacteaInitialQuery::posCutoutString(coords[0], coords[1], rectSize[0]);
        connect(vq, &VialacteaInitialQuery::searchDoneVO, this,
                [vq, pos, this](const QByteArray &votable) {
                    auto tree = new VLKBInventoryTree(votable, pos);
                    tree->show();

                    vq->deleteLater();
                });

        vq->searchRequest(coords[0], coords[1], rectSize[0]);
    }
}

void ViaLactea::on_actionExit_triggered()
{
    this->close();
}

void ViaLactea::closeEvent(QCloseEvent *event)
{
    auto res = QMessageBox::question(this, "Confirm exit",
                                     "Do you want to exit?.\nClosing this "
                                     "window will terminate ongoing processes.",
                                     QMessageBox::Yes | QMessageBox::No);

    if (res == QMessageBox::No) {
        event->ignore();
        return;
    }

    ui->webView->page()->deleteLater();
    QApplication::quit();
}

void ViaLactea::on_actionAbout_triggered()
{
    if (!aboutForm) {
        aboutForm = new AboutForm(this);
    }
    aboutForm->show();
    aboutForm->activateWindow();
    aboutForm->raise();
}

void ViaLactea::on_actionConeSearch_triggered()
{
    if (!coneForm) {
        coneForm = new SimpleConeSearchForm(this);
    }

    coneForm->show();
    coneForm->activateWindow();
    coneForm->raise();
}

void ViaLactea::on_actionHiPS2FITS_triggered()
{
    if (!hipsForm) {
        hipsForm = new HiPS2FITSForm;
    }

    hipsForm->show();
    hipsForm->activateWindow();
    hipsForm->raise();
}

void ViaLactea::on_select3dPushButton_clicked()
{
    VLKBSimpleQueryComposer *skyregionquery = new VLKBSimpleQueryComposer(NULL);
    skyregionquery->setIs3dSelections();

    skyregionquery->setLongitude(0, 360);
    skyregionquery->setLatitude(-1, 1);
    skyregionquery->show();
}

void ViaLactea::on_actionLoad_SED_2_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load SED fits"), QDir::homePath(),
                                                    tr("Archive (*.zip)"));
    if (fileName.isEmpty())
        return;

    QString sedZipPath = QDir::homePath() + "/VisIVODesktopTemp/tmp_download/SED.zip";
    QFile::copy(fileName, sedZipPath);
    QProcess process_unzip;
    process_unzip.start("unzip " + sedZipPath + " -d " + QDir::homePath()
                        + "/VisIVODesktopTemp/tmp_download");

    process_unzip.waitForFinished(); // sets current thread to sleep and waits for process end
    QString output_unzip(process_unzip.readAll());

    QDir tmp_download(QDir::homePath() + "/VisIVODesktopTemp/tmp_download");
    QStringList filters;
    filters << "SED*";
    tmp_download.setNameFilters(filters);
    QStringList dirList = tmp_download.entryList();

    QFile sedFile(QDir::homePath()
                          .append(QDir::separator())
                          .append("/VisIVODesktopTemp/tmp_download/SEDList.dat"));
    sedFile.open(QIODevice::ReadOnly);
    QDataStream in(&sedFile); // read the data serialized from the file
    QList<SED *> sed_list2;
    in >> sed_list2;

    ViaLactea *vialactealWin = &Singleton<ViaLactea>::Instance();
    SEDVisualizerPlot *sedv = new SEDVisualizerPlot(sed_list2, 0, vialactealWin);
    sedv->show();
    sedv->loadSavedSED(dirList);
}

void ViaLactea::on_pointRadioButton_clicked(bool checked)
{
    if (checked) {
        ui->webView->page()->runJavaScript("activatePointSelection(true)");
    }
}

void ViaLactea::on_rectRadioButton_clicked(bool checked)
{
    if (checked) {
        ui->webView->page()->runJavaScript("activateRectangularSelection(true)");
    }
}

void ViaLactea::on_dlLineEdit_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    queryButtonStatusOnOff();
}

void ViaLactea::on_dbLineEdit_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    queryButtonStatusOnOff();
}

void ViaLactea::on_actionLoad_session_triggered()
{
    QString rootPath = QFileDialog::getExistingDirectory(this, "Load a session", QDir::homePath());
    if (rootPath.isEmpty()) {
        return;
    }

    QString fn = QDir(rootPath).absoluteFilePath("session.json");
    if (!QFile::exists(fn)) {
        // Scan the folder for fits files and create the session file on-the-fly
        QStringList fits;
        QDir rootDir(rootPath);
        sessionScan(rootPath, rootDir, fits);

        QJsonArray layers;
        QJsonArray datacubes;
        foreach (auto &&file, fits) {
            std::string fn = rootDir.absoluteFilePath(file).toStdString();
            int status = 0;
            fitsfile *fptr;

            if (fits_open_data(&fptr, fn.c_str(), READONLY, &status)) {
                fits_report_error(stderr, status);
                continue;
            }

            int naxis;
            if (fits_get_img_dim(fptr, &naxis, &status)) {
                fits_report_error(stderr, status);
                continue;
            }

            if (naxis == 3) {
                QJsonObject dc;
                dc["fits"] = file;
                dc["enabled"] = false;
                datacubes.append(dc);
            } else {
                QJsonObject layer;
                layer["fits"] = file;
                layer["type"] = "Continuum";
                layer["enabled"] = false;
                layers.append(layer);
            }

            fits_close_file(fptr, &status);
        }

        QJsonObject image;
        image["layers"] = layers;

        QJsonObject root;
        root["datacubes"] = datacubes;
        root["image"] = image;

        QJsonDocument doc(root);
        QFile tmp(fn);
        if (!tmp.open(QFile::WriteOnly)) {
            QMessageBox::critical(this, tr("Error"), tmp.errorString());
            return;
        }
        tmp.write(doc.toJson());
        tmp.close();
    }

    auto sessionLoader = new SessionLoader(this, fn);
    sessionLoader->show();
    sessionLoader->activateWindow();
    sessionLoader->raise();
}

void ViaLactea::on_loadTableButton_clicked()
{
    // Check if a job was already being polled
    QString pendingFile(QDir::homePath().append("/VisIVODesktopTemp/pending_mcutouts.dat"));
    if (QFileInfo::exists(pendingFile)) {
        auto res = QMessageBox::question(this, "MCutout",
                                         "A pending job has been found.\nWould you like to check "
                                         "it?\n\nIf you refuse, the job will be discarded!");

        if (res == QMessageBox::Yes) {
            auto mcutoutWin = new MCutoutSummary(this, pendingFile);
            mcutoutWin->show();
            mcutoutWin->activateWindow();
            mcutoutWin->raise();
            return;
        } else {
            QFile::remove(pendingFile);
        }
    }

    QString fn = QFileDialog::getOpenFileName(this, "Load user table", QDir::homePath());
    if (fn.isEmpty())
        return;

    auto win = new UserTableWindow(fn, settingsFile, this);
    win->showMaximized();
    win->activateWindow();
    win->raise();

    this->showMinimized();
}

void ViaLactea::on_openLoadDataPushButton_clicked()
{
    QString fn = QFileDialog::getOpenFileName(this, tr("Import an image file"), QString(),
                                              tr("FITS images (*.fit *.fits)"));
    if (fn.isEmpty()) {
        return;
    }

    if (AstroUtils::isFitsImage(fn.toStdString())) {
        openLocalImage(fn);
    } else {
        openLocalDC(fn);
    }
}

void ViaLactea::on_actionLoadArepo_triggered()
{
    QString filename = QFileDialog::getOpenFileName(nullptr, "Open Arepo HDF5", "",
                                                    "HDF5 Files (*.hdf5 *.h5)");

    if (!filename.isEmpty()) {
        auto win = new ArepoExtractorWidget(filename, this);
        win->show();
    }
}
