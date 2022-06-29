#include "catalogue.h"

#include "source.h"

#include <vtkCellArray.h>
#include <vtkLODActor.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

#include <QtConcurrent>

Catalogue::Catalogue(QObject *parent, const QString &fn, vtkRenderWindow *renWin)
    : QObject(parent), filepath(fn), renWin(renWin)
{
    QFile file(fn);
    file.open(QFile::ReadOnly);
    root = QJsonDocument::fromJson(file.readAll()).object();
    file.close();

    QJsonArray sources = root["sources"].toArray();
    foreach (auto &&it, sources) {
        Source *s = Source::fromJson(it.toObject(), this);
        this->sources.insert(s->getIauName(), s);
    }
}

void Catalogue::ExtractSourceInsideRect(double rect[], double arcsec)
{
    double x1 = fmin(rect[0], rect[2]);
    double y1 = fmin(rect[1], rect[3]);

    double x3 = fmax(rect[0], rect[2]);
    double y3 = fmax(rect[1], rect[3]);

    double x2 = x3;
    double y2 = y1;

    double x4 = x1;
    double y4 = y3;

    double w = fabs(x3 - x1);
    double h = fabs(y3 - y1);
    double area = w * h;

    QSet<QString> extracted;

    QtConcurrent::blockingMap(sources, [&](Source *s) -> void {
        double cx = s->getX0();
        double cy = s->getY0();

        double area1 = fabs(0.5 * (x1 * (cy - y4) + cx * (y4 - y1) + x4 * (y1 - cy)));
        double area2 = fabs(0.5 * (x4 * (cy - y3) + cx * (y3 - y4) + x3 * (y4 - cy)));
        double area3 = fabs(0.5 * (x3 * (cy - y2) + cx * (y2 - y3) + x2 * (y3 - cy)));
        double area4 = fabs(0.5 * (cx * (y2 - y1) + x2 * (y1 - cy) + x1 * (cy - y2)));
        double sum = area1 + area2 + area3 + area4;

        if (fabs((area - sum) / area) < 0.1) {
            extracted.insert(s->getIauName());
        }
    });

    if (!extracted.isEmpty()) {
        drawExtractedSources(extracted, arcsec);
        emit SourcesExtracted();
    }
}

const QMap<QString, QPair<vtkSmartPointer<vtkPoints>, vtkSmartPointer<vtkLODActor>>> &
Catalogue::getExtractedNames() const
{
    return extractedNames;
}

void Catalogue::updatePoints(const QString &iau_name, vtkSmartPointer<vtkPoints> points)
{
    auto source = sources.value(iau_name);
    source->updatePoints(points);

    auto sources = root["sources"].toArray();
    sources[indexOf(iau_name)] = source->getObj();
    root["sources"] = sources;
}

void Catalogue::removeSource(const QString &iau_name)
{
    if (!extractedNames.contains(iau_name)) {
        return;
    }

    auto pair = extractedNames.value(iau_name);
    extractedNames.remove(iau_name);
    auto source = sources.value(iau_name);
    sources.remove(iau_name);

    auto tmp = root["sources"].toArray();
    tmp.removeAt(indexOf(iau_name));
    root["sources"] = tmp;

    renWin->GetRenderers()->GetFirstRenderer()->RemoveActor(pair.second);
    renWin->Render();

    source->deleteLater();
    emit ExtractedSourcesUpdated();
}

void Catalogue::renameSource(const QString &iau_name, const QString &new_iau_name)
{
    if (!extractedNames.contains(iau_name) || iau_name == new_iau_name || new_iau_name.isEmpty()) {
        return;
    }

    auto pair = extractedNames.value(iau_name);
    extractedNames.remove(iau_name);
    extractedNames.insert(new_iau_name, pair);

    auto source = sources.value(iau_name);
    source->setIauName(new_iau_name);
    sources.remove(iau_name);
    sources.insert(new_iau_name, source);

    auto tmp = root["sources"].toArray();
    tmp[indexOf(iau_name)] = source->getObj();
    root["sources"] = tmp;

    emit ExtractedSourcesUpdated();
}

void Catalogue::save()
{
    QString timestamp = QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd_hh-mm-ss");
    QString fn = QString("%1_%2.json").arg(QFileInfo(filepath).baseName(), timestamp);
    QString out = QFileInfo(filepath).absoluteDir().absoluteFilePath(fn);

    QFile f(out);
    f.open(QIODevice::WriteOnly | QIODevice::Text);
    QJsonDocument doc(root);
    f.write(doc.toJson());
    f.close();
    QMessageBox::information(nullptr, "Catalogue saved",
                             "Catalogue saved in " + QFileInfo(f).absoluteFilePath());
}

const QString &Catalogue::getFilepath() const
{
    return filepath;
}

int Catalogue::indexOf(const QString &iau_name) const
{
    auto sources = root["sources"].toArray();
    for (int i = 0; i < sources.count(); ++i) {
        auto obj = sources[i].toObject();
        if (obj["iau_name"].toString() == iau_name) {
            return i;
        }
    }
    return -1;
}

void Catalogue::drawExtractedSources(const QSet<QString> &names, double arcsec)
{
    foreach (auto &&iau_name, names) {
        if (extractedNames.contains(iau_name))
            continue;

        auto s = getSource(iau_name);
        foreach (auto &&island, s->getIslands()) {
            auto vertices = island->getVertices();
            if (vertices.isEmpty()) {
                continue;
            }

            auto points = vtkSmartPointer<vtkPoints>::New();
            points->SetNumberOfPoints(vertices.count());
            auto cells = vtkSmartPointer<vtkCellArray>::New();
            cells->InsertNextCell(vertices.count() + 1);
            for (long long i = 0; i < vertices.count(); ++i) {
                auto point = vertices.at(i);
                points->SetPoint(i, point.first + 1, point.second + 1, arcsec);
                cells->InsertCellPoint(i);
            }
            cells->InsertCellPoint(0);

            auto polyData = vtkSmartPointer<vtkPolyData>::New();
            polyData->SetPoints(points);
            polyData->SetLines(cells);

            auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
            mapper->SetInputData(polyData);

            auto actor = vtkSmartPointer<vtkLODActor>::New();
            actor->SetMapper(mapper);
            actor->GetProperty()->SetColor(0, 1, 1);

            renWin->GetRenderers()->GetFirstRenderer()->AddActor(actor);

            extractedNames.insert(
                    iau_name,
                    QPair<vtkSmartPointer<vtkPoints>, vtkSmartPointer<vtkLODActor>>(points, actor));
        }
    }

    renWin->GetInteractor()->Render();
}

const QHash<QString, Source *> &Catalogue::getSources() const
{
    return sources;
}

const Source *Catalogue::getSource(QString iau_name) const
{
    if (this->sources.contains(iau_name)) {
        return this->sources.value(iau_name);
    }

    return nullptr;
}

vtkSmartPointer<vtkLODActor> Catalogue::GetActor() const
{
    return Actor;
}

void Catalogue::SetActor(vtkSmartPointer<vtkLODActor> actor)
{
    this->Actor = actor;
}

vtkSmartPointer<vtkPolyDataAlgorithm> Catalogue::GetPolyDataFilter() const
{
    return PolyDataFilter;
}

void Catalogue::SetPolyDataFilter(vtkSmartPointer<vtkPolyDataAlgorithm> PolyDataFilter)
{
    this->PolyDataFilter = PolyDataFilter;
}
