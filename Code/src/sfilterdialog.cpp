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
    ui->textPredicate->setFont(font);

    ui->comboFields->addItems(fields);
    ui->comboMorphLabel->addItems(morph_label_val);

    model = new QStringListModel(this);
    ui->listView->setModel(model);

    ui->textPredicate->clear();
}

SFilterDialog::~SFilterDialog()
{
    auto win = qobject_cast<vtkwindow_new *>(this->parent());
    win->hideFilteredSources();
    delete ui;
}

void SFilterDialog::on_btnFilter_clicked()
{
    QProcess *py3 = new QProcess(this);
    py3->setProcessChannelMode(QProcess::MergedChannels);
    connect(py3, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
            [this, py3](int exitCode, QProcess::ExitStatus exitStatus) {
                Q_UNUSED(exitStatus);
                QStringList ids = QString::fromUtf8(py3->readAll()).trimmed().split('\n');
                ids.removeAll(QString()); // Remove empty strings
                model->setStringList(ids);

                auto win = qobject_cast<vtkwindow_new *>(this->parent());
                if (exitCode == 0 && !ids.isEmpty()) {
                    win->showFilteredSources(ids);
                    ui->btnConfirm->setEnabled(true);
                } else {
                    win->hideFilteredSources();
                    ui->btnConfirm->setEnabled(false);
                }
                py3->deleteLater();
            });

    QDir wd = QApplication::applicationDirPath();

    QStringList args;
    args << "sfilter.py" << catalogue->getFilepath() << ui->textPredicate->text();
    py3->setWorkingDirectory(wd.absolutePath());
    py3->start("python3", args);
}

void SFilterDialog::append(const QString &arg)
{
    QString text = ui->textPredicate->text();
    text.append(arg);
    ui->textPredicate->setText(text);
}

void SFilterDialog::on_comboFields_currentTextChanged(const QString &arg1)
{
    append(arg1);
}

void SFilterDialog::on_comboMorphLabel_currentTextChanged(const QString &arg1)
{
    append("'" + arg1 + "'");
}

void SFilterDialog::on_comboSourcenessLabel_currentTextChanged(const QString &arg1)
{
    append("'" + arg1 + "'");
}

void SFilterDialog::on_comboClassLabel_currentTextChanged(const QString &arg1)
{
    append("'" + arg1 + "'");
}

void SFilterDialog::on_btnConfirm_clicked()
{
    catalogue->filterSources(model->stringList());
    emit this->accept();
}
