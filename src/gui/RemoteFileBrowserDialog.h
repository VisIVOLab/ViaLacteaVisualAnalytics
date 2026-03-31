#ifndef RemoteFileBrowserDialog_h
#define RemoteFileBrowserDialog_h

#include "app/BackendClient.h"

#include <QDialog>
#include <QFutureWatcher>

#include <memory>

class QLineEdit;
class QPushButton;
class QTextEdit;
class QTreeWidget;
class QTreeWidgetItem;
class QLabel;

class RemoteFileBrowserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RemoteFileBrowserDialog(const QString &backendUrl, QWidget *parent = nullptr);
    ~RemoteFileBrowserDialog() override;

    QString selectedFilePath() const;

private:
    void loadPath(const QString &path);
    void populateEntries(const std::vector<BackendFileEntry> &entries);
    void requestHeaderPreview(const QString &path);
    void clearDetails();
    void updateDetails(const BackendFileEntry *entry);
    void updateSelectionState();
    QString humanReadableSize(qint64 size) const;

    std::unique_ptr<BackendClient> client;
    QLineEdit *pathEdit;
    QTreeWidget *entriesTree;
    QPushButton *openButton;
    QLabel *nameValue;
    QLabel *pathValue;
    QLabel *typeValue;
    QLabel *sizeValue;
    QLabel *modifiedValue;
    QTextEdit *headerView;
    QFutureWatcher<BackendFileHeaderResult> headerWatcher;
    QString selectedPath;
};

#endif
