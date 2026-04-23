#ifndef Settings_h
#define Settings_h

#include <QObject>
#include <QPointer>
#include <QVariant>

class QSettings;

/**
 * Application-level Settings class
 */
class Settings : public QObject
{
    Q_OBJECT

public:
    /**
   * Application-level Settings class
   * @param appDir Absolute directory path to store settings
   */
    explicit Settings(const QString &appDir, QObject *parent = nullptr);
    ~Settings() override;

    /**
   * @return Absolute directory path where settings are stored
   */
    QString getApplicationDir() const;

    /**
   * @return The application color scheme
   */
    Qt::ColorScheme getColorScheme() const;

    /**
   * Set the application color scheme
   * @param scheme The color scheme to apply
   */
    void setColorScheme(Qt::ColorScheme scheme);

    /**
   * @return Python absolute path
   */
    QString getPythonLocation() const;

    /**
   * Set Python absolute path
   * @param location Python3 absolute path
   */
    void setPythonLocation(const QString &location);

    /**
   * @return Panoramic View URL
   */
    QString getPanoramicView() const;

    /**
   * Set Panoramic View URL
   * @param url Panoramic View URL
   */
    void setPanoramicView(const QString &url);

    /**
   * @return Maximum number of glyphs to visualize in vtkWindowSources
   */
    int getMaxGlyphs() const;

    /**
   * Set the maximum number of glyphs to visualize in vtkWindowSources
   * @param n Max number of glyphs
   */
    void setMaxGlyphs(int n);

    /**
   * @return Current VLKB services endpoint
   */
    QString getVLKBUrl() const;

    /**
   * Set VLKB endpoint
   * @param url VLKB endpoint URL
   */
    void setVLKBUrl(const QString &url);

    /**
   * @return true if VisIVO should search the Knowledge Base when loading local
   * data
   */
    bool getSearchOnImportFlag() const;

    /**
   * Enable or disable the Search on Import functionality
   * @param flag
   */
    void setSearchOnImportFlag(bool flag);

public slots:
    /**
   * Reload settings from file
   */
    void reload();

    /**
   * Reset settings to default values
   */
    void resetDefaults();

signals:
    /**
   * Signal to notify consumer that they need to reload their settings
   */
    void updated();

private:
    const QString appDir;
    QPointer<QSettings> settings;

    // Platform dependent
    std::pair<QString, QVariant> PythonLocation;
    void setPythonLocation();

    // Platform independent
    static const std::pair<QString, QVariant> ColorScheme;
    static const std::pair<QString, QVariant> PanoramicView;
    static const std::pair<QString, QVariant> MaxGlyphs;
    static const std::pair<QString, QVariant> VLKBUrl;
    static const std::pair<QString, QVariant> VLKBSearch;
};

#endif
