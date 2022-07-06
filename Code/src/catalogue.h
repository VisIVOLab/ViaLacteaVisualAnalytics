#ifndef CATALOGUE_H
#define CATALOGUE_H

#include <QJsonObject>
#include <QObject>

#include <vtkSmartPointer.h>

class Source;
class vtkLODActor;
class vtkPoints;
class vtkPolyDataAlgorithm;
class vtkPolyData;
class vtkRenderWindow;

class Catalogue : public QObject
{
    Q_OBJECT
public:
    explicit Catalogue(QObject *parent, const QString &fn, vtkRenderWindow *renWin);

    const QHash<QString, Source *> &getSources() const;
    const Source *getSource(QString iau_name) const;

    vtkSmartPointer<vtkLODActor> GetActor() const;
    void SetActor(vtkSmartPointer<vtkLODActor> actor);

    vtkSmartPointer<vtkPolyDataAlgorithm> GetPolyDataFilter() const;
    void SetPolyDataFilter(vtkSmartPointer<vtkPolyDataAlgorithm> PolyDataFilter);

    void ExtractSourceInsideRect(double rect[4], double arcsec);

    const QMap<QString, QPair<vtkSmartPointer<vtkPoints>, vtkSmartPointer<vtkLODActor>>> &
    getExtractedNames() const;

    void updatePoints(const QString &iau_name, vtkSmartPointer<vtkPoints> points);
    void removeSource(const QString &iau_name);
    void renameSource(const QString &iau_name, const QString &new_iau_name);
    void updateMorphLabel(const QString &iau_name, const QString &morph_label);

    void save();

    void filterSources(const QStringList &ids);
    void removeFilteredSources();

    const QString &getFilepath() const;

    const QStringList &getFilteredIds() const;

signals:
    void SourcesExtracted();
    void ExtractedSourcesUpdated();
    void SourcesFiltered(const QStringList &ids);

private:
    QString filepath;
    vtkRenderWindow *renWin;
    QJsonObject root;
    QHash<QString, Source *> sources;

    int indexOf(const QString &iau_name) const;

    // Initial dataset
    vtkSmartPointer<vtkPolyDataAlgorithm> PolyDataFilter;
    vtkSmartPointer<vtkLODActor> Actor;

    // Extracted dataset
    QMap<QString, QPair<vtkSmartPointer<vtkPoints>, vtkSmartPointer<vtkLODActor>>> extractedNames;

    QStringList filteredIds;
    vtkSmartPointer<vtkLODActor> FilteredActor;

    void drawExtractedSources(const QSet<QString> &names, double arcsec);
};

#endif // CATALOGUE_H
