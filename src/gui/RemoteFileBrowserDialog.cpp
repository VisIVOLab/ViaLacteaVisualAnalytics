#include "RemoteFileBrowserDialog.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSplitter>
#include <QStyle>
#include <QTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QtConcurrentRun>

using namespace Qt::StringLiterals;

namespace {
constexpr int PathRole = Qt::UserRole + 1;
constexpr int TypeRole = Qt::UserRole + 2;
constexpr int SizeRole = Qt::UserRole + 3;
constexpr int ModifiedRole = Qt::UserRole + 4;
constexpr int IsFitsRole = Qt::UserRole + 5;
}

RemoteFileBrowserDialog::RemoteFileBrowserDialog(const QString &backendUrl, QWidget *parent)
    : QDialog(parent),
      client(std::make_unique<BackendClient>(backendUrl)),
      pathEdit(new QLineEdit(this)),
      entriesTree(new QTreeWidget(this)),
      openButton(new QPushButton(u"Open"_s, this)),
      nameValue(new QLabel(this)),
      pathValue(new QLabel(this)),
      typeValue(new QLabel(this)),
      sizeValue(new QLabel(this)),
      modifiedValue(new QLabel(this)),
      headerView(new QTextEdit(this))
{
    this->setWindowTitle(u"Browse Remote FITS Files"_s);
    this->resize(980, 620);

    auto *layout = new QVBoxLayout(this);
    auto *pathRow = new QHBoxLayout;
    auto *homeButton = new QPushButton(u"Home"_s, this);
    auto *upButton = new QPushButton(u"Up"_s, this);
    auto *refreshButton = new QPushButton(u"Refresh"_s, this);

    pathRow->addWidget(new QLabel(u"Path"_s, this));
    pathRow->addWidget(this->pathEdit, 1);
    pathRow->addWidget(homeButton);
    pathRow->addWidget(upButton);
    pathRow->addWidget(refreshButton);
    layout->addLayout(pathRow);

    auto *splitter = new QSplitter(this);
    splitter->setChildrenCollapsible(false);
    layout->addWidget(splitter, 1);

    auto *browserPane = new QWidget(splitter);
    auto *browserLayout = new QVBoxLayout(browserPane);
    browserLayout->setContentsMargins(0, 0, 0, 0);
    this->entriesTree->setColumnCount(3);
    this->entriesTree->setHeaderLabels({ u"Name"_s, u"Size"_s, u"Modified"_s });
    this->entriesTree->setAlternatingRowColors(true);
    this->entriesTree->setRootIsDecorated(false);
    this->entriesTree->setSortingEnabled(false);
    this->entriesTree->header()->setStretchLastSection(false);
    this->entriesTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->entriesTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    this->entriesTree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    browserLayout->addWidget(this->entriesTree);

    auto *detailsPane = new QWidget(splitter);
    auto *detailsLayout = new QVBoxLayout(detailsPane);
    detailsLayout->setContentsMargins(0, 0, 0, 0);
    auto *detailsTitle = new QLabel(u"Details"_s, detailsPane);
    auto *detailsForm = new QFormLayout;
    detailsForm->setContentsMargins(0, 0, 0, 0);
    this->pathValue->setTextInteractionFlags(Qt::TextSelectableByMouse);
    this->headerView->setReadOnly(true);
    this->headerView->setLineWrapMode(QTextEdit::NoWrap);

    detailsForm->addRow(u"Name"_s, this->nameValue);
    detailsForm->addRow(u"Path"_s, this->pathValue);
    detailsForm->addRow(u"Type"_s, this->typeValue);
    detailsForm->addRow(u"Size"_s, this->sizeValue);
    detailsForm->addRow(u"Modified"_s, this->modifiedValue);
    detailsLayout->addWidget(detailsTitle);
    detailsLayout->addLayout(detailsForm);
    detailsLayout->addWidget(new QLabel(u"FITS Header"_s, detailsPane));
    detailsLayout->addWidget(this->headerView, 1);
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 2);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    buttons->addButton(this->openButton, QDialogButtonBox::AcceptRole);
    layout->addWidget(buttons);

    this->openButton->setEnabled(false);
    this->clearDetails();

    QObject::connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    QObject::connect(this->openButton, &QPushButton::clicked, this, [this]() {
        if (!this->selectedPath.isEmpty()) {
            this->accept();
        }
    });
    QObject::connect(this->pathEdit, &QLineEdit::returnPressed, this,
                     [this]() { this->loadPath(this->pathEdit->text()); });
    QObject::connect(refreshButton, &QPushButton::clicked, this,
                     [this]() { this->loadPath(this->pathEdit->text()); });
    QObject::connect(homeButton, &QPushButton::clicked, this, [this]() { this->loadPath({}); });
    QObject::connect(upButton, &QPushButton::clicked, this, [this]() {
        const QDir dir(this->pathEdit->text());
        this->loadPath(dir.absoluteFilePath(u".."_s));
    });
    QObject::connect(this->entriesTree, &QTreeWidget::currentItemChanged, this,
                     [this](QTreeWidgetItem *current) {
                         this->updateSelectionState();
                         if (!current) {
                             this->clearDetails();
                             return;
                         }

                         BackendFileEntry entry;
                         entry.name = current->text(0);
                         entry.path = current->data(0, PathRole).toString();
                         entry.type = current->data(0, TypeRole).toString();
                         entry.size = current->data(0, SizeRole).toLongLong();
                         entry.modifiedTime = current->data(0, ModifiedRole).toString();
                         entry.isFits = current->data(0, IsFitsRole).toBool();
                         this->updateDetails(&entry);
                     });
    QObject::connect(this->entriesTree, &QTreeWidget::itemDoubleClicked, this,
                     [this](QTreeWidgetItem *item) {
                         if (!item) {
                             return;
                         }

                         const QString itemType = item->data(0, TypeRole).toString();
                         const QString itemPath = item->data(0, PathRole).toString();
                         if (itemType == u"directory"_s) {
                             this->loadPath(itemPath);
                             return;
                         }

                         if (item->data(0, IsFitsRole).toBool()) {
                             this->selectedPath = itemPath;
                             this->accept();
                         }
                     });
    QObject::connect(&this->headerWatcher, &QFutureWatcher<BackendFileHeaderResult>::finished, this,
                     [this]() {
                         const QString expectedPath = this->headerWatcher.property("path").toString();
                         const auto result = this->headerWatcher.result();
                         auto *current = this->entriesTree->currentItem();
                         if (!current || current->data(0, PathRole).toString() != expectedPath) {
                             return;
                         }

                         if (!result.valid) {
                             this->headerView->setPlainText(result.error.isEmpty()
                                                                    ? u"Could not load FITS header."_s
                                                                    : result.error);
                             return;
                         }

                         this->headerView->setPlainText(result.cards.join(u"\n"_s));
                     });

    this->loadPath({});
}

RemoteFileBrowserDialog::~RemoteFileBrowserDialog() = default;

QString RemoteFileBrowserDialog::selectedFilePath() const
{
    return this->selectedPath;
}

void RemoteFileBrowserDialog::loadPath(const QString &path)
{
    const auto result = this->client->listFiles(path);
    if (!result.valid) {
        QMessageBox::warning(this, u"Remote Files"_s,
                             result.error.isEmpty() ? u"Could not list remote directory."_s
                                                    : result.error);
        return;
    }

    this->selectedPath.clear();
    this->pathEdit->setText(result.currentPath);
    this->populateEntries(result.entries);
    this->clearDetails();
    this->updateSelectionState();
}

void RemoteFileBrowserDialog::populateEntries(const std::vector<BackendFileEntry> &entries)
{
    this->entriesTree->clear();

    const QIcon folderIcon = this->style()->standardIcon(QStyle::SP_DirIcon);
    const QIcon fitsIcon = this->style()->standardIcon(QStyle::SP_FileIcon);
    const QIcon fileIcon = this->style()->standardIcon(QStyle::SP_FileLinkIcon);

    for (const auto &entry : entries) {
        auto *item = new QTreeWidgetItem(this->entriesTree);
        item->setText(0, entry.name);
        item->setText(1, entry.type == u"directory"_s ? QString() : this->humanReadableSize(entry.size));
        item->setText(2, entry.modifiedTime);
        item->setData(0, PathRole, entry.path);
        item->setData(0, TypeRole, entry.type);
        item->setData(0, SizeRole, entry.size);
        item->setData(0, ModifiedRole, entry.modifiedTime);
        item->setData(0, IsFitsRole, entry.isFits);
        item->setIcon(0, entry.type == u"directory"_s ? folderIcon : (entry.isFits ? fitsIcon : fileIcon));
    }
}

void RemoteFileBrowserDialog::requestHeaderPreview(const QString &path)
{
    if (this->headerWatcher.isRunning()) {
        this->headerWatcher.cancel();
    }

    this->headerView->setPlainText(u"Loading FITS header..."_s);
    this->headerWatcher.setProperty("path", path);
    this->headerWatcher.setFuture(QtConcurrent::run([this, path]() { return this->client->fileHeader(path); }));
}

void RemoteFileBrowserDialog::clearDetails()
{
    this->nameValue->clear();
    this->pathValue->clear();
    this->typeValue->clear();
    this->sizeValue->clear();
    this->modifiedValue->clear();
    this->headerView->setPlainText(u"Select a file to preview its details."_s);
}

void RemoteFileBrowserDialog::updateDetails(const BackendFileEntry *entry)
{
    if (!entry) {
        this->clearDetails();
        return;
    }

    this->nameValue->setText(entry->name);
    this->pathValue->setText(entry->path);
    this->typeValue->setText(entry->type == u"directory"_s ? u"Directory"_s
                                                            : (entry->isFits ? u"FITS file"_s
                                                                             : u"File"_s));
    this->sizeValue->setText(entry->type == u"directory"_s ? u"_"_s : this->humanReadableSize(entry->size));
    this->modifiedValue->setText(entry->modifiedTime);

    if (entry->isFits) {
        this->requestHeaderPreview(entry->path);
    } else {
        this->headerView->setPlainText(entry->type == u"directory"_s
                                               ? u"Directory selected."_s
                                               : u"Header preview is available only for FITS files."_s);
    }
}

void RemoteFileBrowserDialog::updateSelectionState()
{
    const auto *item = this->entriesTree->currentItem();
    const bool isFitsFile =
            item && item->data(0, TypeRole).toString() == u"file"_s && item->data(0, IsFitsRole).toBool();
    this->selectedPath = isFitsFile ? item->data(0, PathRole).toString() : QString();
    this->openButton->setEnabled(isFitsFile);
}

QString RemoteFileBrowserDialog::humanReadableSize(qint64 size) const
{
    static const QStringList units = { u"B"_s, u"KB"_s, u"MB"_s, u"GB"_s, u"TB"_s };
    double value = static_cast<double>(size);
    int unitIndex = 0;
    while (value >= 1024.0 && unitIndex < units.size() - 1) {
        value /= 1024.0;
        ++unitIndex;
    }

    return QStringLiteral("%1 %2").arg(value, 0, 'f', unitIndex == 0 ? 0 : 1).arg(units[unitIndex]);
}
