#ifndef RemoteFileBrowserDialog_h
#define RemoteFileBrowserDialog_h

#include <QDialog>

#include <memory>

class BackendClient;
class QLineEdit;
class QListWidget;
class QPushButton;

class RemoteFileBrowserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoteFileBrowserDialog(const QString &backendUrl, QWidget *parent = nullptr);
    ~RemoteFileBrowserDialog() override;

    QString selectedFilePath() const;

private:
    void loadPath(const QString &path);
    void updateSelectionState();

    std::unique_ptr<BackendClient> client;
    QLineEdit *pathEdit;
    QListWidget *entriesList;
    QPushButton *openButton;
    QString selectedPath;
};

#endif
