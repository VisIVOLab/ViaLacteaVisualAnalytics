#include "Settings.h"

#include "Logging.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QSettings>

#include <limits>

using namespace Qt::StringLiterals;

Settings::Settings(const QString &appDir, QObject *parent) : QObject(parent), appDir(appDir)
{
    const QString settingsPath = QDir(appDir).absoluteFilePath(u"Settings.ini"_s);
    this->settings = new QSettings(settingsPath, QSettings::IniFormat, this);

    this->setPythonLocation();

    if (!QFileInfo::exists(settingsPath)) {
        qCWarning(logSettings) << settingsPath
                               << "does not exist. Creating one with default settings...";
        this->resetDefaults();
    }
}

Settings::~Settings() = default;

QString Settings::getApplicationDir() const
{
    return this->appDir;
}

void Settings::reload()
{
    this->settings->sync();
}

void Settings::resetDefaults()
{
    this->settings->setValue(Settings::ColorScheme.first, Settings::ColorScheme.second);
    this->settings->setValue(this->PythonLocation.first, this->PythonLocation.second);
    this->settings->setValue(Settings::PanoramicView.first, Settings::PanoramicView.second);
    this->settings->setValue(Settings::MaxGlyphs.first, Settings::MaxGlyphs.second);
    this->settings->setValue(Settings::VLKBUrl.first, Settings::VLKBUrl.second);
    this->settings->setValue(Settings::VLKBSearch.first, Settings::VLKBSearch.second);

    emit this->updated();
}

//----------------------------------------------------------------------------
// UI
//----------------------------------------------------------------------------
const std::pair<QString, QVariant> Settings::ColorScheme = {
    u"UI/theme"_s, static_cast<int>(Qt::ColorScheme::Unknown)
};

Qt::ColorScheme Settings::getColorScheme() const
{
    return static_cast<Qt::ColorScheme>(
            this->settings->value(Settings::ColorScheme.first, Settings::ColorScheme.second)
                    .toInt());
}

void Settings::setColorScheme(Qt::ColorScheme scheme)
{
    this->settings->setValue(Settings::ColorScheme.first, static_cast<int>(scheme));
}

//----------------------------------------------------------------------------
// Python
//----------------------------------------------------------------------------
void Settings::setPythonLocation()
{
    QProcess p;
    const QStringList args = { u"-c"_s, u"import sys; print(sys.executable, end='')"_s };
    p.start(u"python3"_s, args);
    p.waitForFinished();
    QString path = p.readAllStandardOutput();
    if (path.isEmpty()) {
        // Try again with python
        p.start(u"python"_s, args);
        p.waitForFinished();
        path = p.readAllStandardOutput();
    }

    this->PythonLocation = { u"Python/location"_s, path };
}

QString Settings::getPythonLocation() const
{
    const QString path = this->settings->value(this->PythonLocation.first).toString();
    return path.isEmpty() ? this->PythonLocation.second.toString() : path;
}

void Settings::setPythonLocation(const QString &location)
{
    this->settings->setValue(this->PythonLocation.first, location);
}

//----------------------------------------------------------------------------
// PanoramicView
//----------------------------------------------------------------------------
const std::pair<QString, QVariant> Settings::PanoramicView = {
    u"PanoramicView/url"_s, u"http://vlkb.ia2.inaf.it/panoramicview/openlayers.html"_s
};

QString Settings::getPanoramicView() const
{
    const QString url = this->settings->value(Settings::PanoramicView.first).toString();
    return url.isEmpty() ? Settings::PanoramicView.second.toString() : url;
}

void Settings::setPanoramicView(const QString &url)
{
    this->settings->setValue(Settings::PanoramicView.first, url);
}

//----------------------------------------------------------------------------
// Visualization
//----------------------------------------------------------------------------
const std::pair<QString, QVariant> Settings::MaxGlyphs = { u"Visualization/max_glyphs"_s,
                                                           std::numeric_limits<int>::max() };

int Settings::getMaxGlyphs() const
{
    return this->settings->value(Settings::MaxGlyphs.first, Settings::MaxGlyphs.second).toInt();
}

void Settings::setMaxGlyphs(int n)
{
    this->settings->setValue(Settings::MaxGlyphs.first, n);
}

//----------------------------------------------------------------------------
// VLKB
//----------------------------------------------------------------------------
const std::pair<QString, QVariant> Settings::VLKBUrl = { u"VLKB/url"_s,
                                                         u"https://vlkb.ia2.inaf.it"_s };

QString Settings::getVLKBUrl() const
{
    const QString url = this->settings->value(Settings::VLKBUrl.first).toString();
    return url.isEmpty() ? Settings::VLKBUrl.second.toString() : url;
}

void Settings::setVLKBUrl(const QString &url)
{
    this->settings->setValue(Settings::VLKBUrl.first, url);
}

const std::pair<QString, QVariant> Settings::VLKBSearch = { u"VLKB/search_on_import"_s, false };

bool Settings::getSearchOnImportFlag() const
{
    return this->settings->value(Settings::VLKBSearch.first, Settings::VLKBSearch.second).toBool();
}

void Settings::setSearchOnImportFlag(bool flag)
{
    this->settings->setValue(Settings::VLKBSearch.first, flag);
}
