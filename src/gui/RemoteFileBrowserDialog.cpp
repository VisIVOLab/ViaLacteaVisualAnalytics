#include "RemoteFileBrowserDialog.h"

#include "app/BackendClient.h"

#include <QDir>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace Qt::StringLiterals;

namespace {
constexpr int PathRole = Qt::UserRole + 1;
constexpr int TypeRole = Qt::UserRole + 2;
}

RemoteFileBrowserDialog::RemoteFileBrowserDialog(const QString &backendUrl, QWidget *parent)
    : QDialog(parent),
      client(std::make_unique<BackendClient>(backendUrl)),
      pathEdit(new QLineEdit(this)),
      entriesList(new QListWidget(this)),
      openButton(new QPushButton(u"Open"_s, this))
{
    this->setWindowTitle(u"Browse Remote FITS Files"_s);
    this->resize(720, 480);

    auto *layout = new QVBoxLayout(this);
    auto *pathRow = new QHBoxLayout;
    auto *upButton = new QPushButton(u"Up"_s, this);
    auto *refreshButton = new QPushButton(u"Refresh"_s, this);

    pathRow->addWidget(new QLabel(u"Path"_s, this));
    pathRow->addWidget(this->pathEdit, 1);
    pathRow->addWidget(upButton);
    pathRow->addWidget(refreshButton);
    layout->addLayout(pathRow);
    layout->addWidget(this->entriesList, 1);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, this);
    buttons->addButton(this->openButton, QDialogButtonBox::AcceptRole);
    layout->addWidget(buttons);

    this->openButton->setEnabled(false);

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
    QObject::connect(upButton, &QPushButton::clicked, this, [this]() {
        const QDir dir(this->pathEdit->text());
        this->loadPath(dir.absoluteFilePath(u".."_s));
    });
    QObject::connect(this->entriesList, &QListWidget::currentItemChanged, this,
                     [this]() { this->updateSelectionState(); });
    QObject::connect(this->entriesList, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        if (!item) {
            return;
        }

        const QString itemType = item->data(TypeRole).toString();
        const QString itemPath = item->data(PathRole).toString();
        if (itemType == u"directory"_s) {
            this->loadPath(itemPath);
            return;
        }

        this->selectedPath = itemPath;
        this->accept();
    });

    this->loadPath(u"/"_s);
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
    this->pathEdit->setText(path);
    this->entriesList->clear();
    for (const auto &entry : result.entries) {
        auto *item = new QListWidgetItem(entry.name, this->entriesList);
        item->setData(PathRole, entry.path);
        item->setData(TypeRole, entry.type);
        if (entry.type == u"directory"_s) {
            item->setText(u"[DIR] %1"_s.arg(entry.name));
        }
    }
    this->updateSelectionState();
}

void RemoteFileBrowserDialog::updateSelectionState()
{
    const auto *item = this->entriesList->currentItem();
    const bool isFile = item && item->data(TypeRole).toString() == u"file"_s;
    this->selectedPath = isFile ? item->data(PathRole).toString() : QString();
    this->openButton->setEnabled(isFile);
}
