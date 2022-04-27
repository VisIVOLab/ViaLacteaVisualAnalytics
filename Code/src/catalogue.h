#ifndef CATALOGUE_H
#define CATALOGUE_H

#include <QJsonDocument>
#include <QObject>

#include <vtkSmartPointer.h>

class Source;
class vtkLODActor;
class vtkPolyDataAlgorithm;
class vtkPolyData;
class vtkRenderWindow;

class Catalogue : public QObject
{
    Q_OBJECT
public:
    explicit Catalogue(QObject *parent, const QString &fn);

    const QHash<QString, Source *> &getSources() const;
    const Source *getSource(QString iau_name) const;

    vtkSmartPointer<vtkLODActor> GetActor() const;
    void SetActor(vtkSmartPointer<vtkLODActor> actor);

    vtkSmartPointer<vtkPolyDataAlgorithm> GetPolyDataFilter() const;
    void SetPolyDataFilter(vtkSmartPointer<vtkPolyDataAlgorithm> PolyDataFilter);

    // void AddExtractedSource(vtkSmartPointer<vtkPolyDataAlgorithm>, vtkSmartPointer<vtkLODActor>);
    void ExtractSourceInsideRect(double rect[4], vtkRenderWindow *renWin, double arcsec);

    const QMap<QString, QPair<vtkSmartPointer<vtkPolyData>, vtkSmartPointer<vtkLODActor>>> &
    getExtractedNames() const;

signals:
    void SourceExtracted();

private:
    QString filepath;
    QJsonDocument doc;
    QHash<QString, Source *> sources;

    // Initial dataset
    vtkSmartPointer<vtkPolyDataAlgorithm> PolyDataFilter;
    vtkSmartPointer<vtkLODActor> Actor;

    // Extracted dataset
    QMap<QString, QPair<vtkSmartPointer<vtkPolyData>, vtkSmartPointer<vtkLODActor>>> extractedNames;

    void drawExtractedSources(const QSet<QString> &names, vtkRenderWindow *renWin, double arcsec);
};

#endif // CATALOGUE_H
