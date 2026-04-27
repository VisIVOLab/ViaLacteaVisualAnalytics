#ifndef MainWindow_h
#define MainWindow_h

#include <QMainWindow>

class AuthWrapper;
class Settings;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    /**
   * Set the application theme
   */
    void setApplicationTheme();

    /**
   * Show a QFileDialog to select and open local files
   */
    void openLocalData();

    /**
   * Show the Settings Dialog
   */
    void showSettingsDialog();

    /**
   * Show the About Dialog
   */
    void showAboutDialog();

    /**
   * Reload the Panoramic View
   */
    void loadPanoramicView();

    /**
     * Change the selection mode for the Panoramic View
     * @param id Selected mode.
     */
    void changeViewSelectionMode(int id);

    /**
   * Update the coordinates and the region for the search request
   * @param point Center of selection (comma separated values)
   * @param area Area of selection (comma separated values), empty string if
   * selection = point
   */
    void skyRegionSelected(const QString &point, const QString &area);

private:
    Ui::MainWindow *ui;
    Settings *settings;
    AuthWrapper *auth;

    enum class ViewSelectionMode { None, Point, Rectangle };
};

#endif
