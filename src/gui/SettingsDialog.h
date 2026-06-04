#ifndef SettingsDialog_h
#define SettingsDialog_h

#include <QDialog>
#include <QPointer>

class Settings;

QT_BEGIN_NAMESPACE
namespace Ui {
class SettingsDialog;
}
QT_END_NAMESPACE

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(Settings *settings, QWidget *parent = nullptr);
    ~SettingsDialog() override;

public slots:
    void accept() override;

private slots:
    void selectPythonLocation();
    void selectPanoramicView();
    // void updateAuthStatus(AuthService service);
    void VLKBAuthTriggered();
    void restoreDefaults();

private:
    Ui::SettingsDialog *ui;
    QPointer<Settings> settings;

    void setVLKBAuthStatus();
};

#endif
