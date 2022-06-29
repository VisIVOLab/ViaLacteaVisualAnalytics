#include "sfilterdialog.h"
#include "ui_sfilterdialog.h"

#include "catalogue.h"
#include "vtkwindow_new.h"

#include <QDir>
#include <QStringListModel>

SFilterDialog::SFilterDialog(Catalogue *c, QWidget *parent)
    : QDialog(parent), ui(new Ui::SFilterDialog), catalogue(c)
{
    ui->setupUi(this);

    QFont font("monospace");
    font.setStyleHint(QFont::Monospace);
    ui->lineEdit->setFont(font);

    fields.sort(Qt::CaseInsensitive);

    ui->comboBox->addItems(fields);

    model = new QStringListModel(this);
    ui->listView->setModel(model);

    ui->lineEdit->clear();
}

SFilterDialog::~SFilterDialog()
{
    delete ui;
}

void SFilterDialog::on_pushButton_clicked()
{
    QProcess *py3 = new QProcess(this);
    py3->setProcessChannelMode(QProcess::MergedChannels);
    connect(py3, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this, py3](int exitCode, QProcess::ExitStatus exitStatus) {
                Q_UNUSED(exitStatus);
                QStringList ids = QString::fromUtf8(py3->readAll()).trimmed().split('\n');
                model->setStringList(ids);
                if (exitCode == 0 && !ids.isEmpty()) {
                    auto win = qobject_cast<vtkwindow_new *>(this->parent());
                    win->showFilteredSources(ids);
                }
                py3->deleteLater();
            });

    QDir wd = QApplication::applicationDirPath();

    QStringList args;
    args << "sfilter.py" << catalogue->getFilepath() << ui->lineEdit->text();
    py3->setWorkingDirectory(wd.absolutePath());
    py3->start("python3", args);
}

void SFilterDialog::on_comboBox_currentTextChanged(const QString &arg1)
{
    QString text = ui->lineEdit->text();
    text.append(arg1);
    ui->lineEdit->setText(text);
}
