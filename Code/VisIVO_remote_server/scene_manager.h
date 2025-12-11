#pragma once

#include <QString>
#include <QJsonObject>
#include <QJsonArray>

class vtkRenderWindow;
class vtkRenderer;
class vtkImageActor;
class vtkImageData;
class vtkImageShiftScale;
class vtkExtractVOI;

class SceneManager {
public:
    SceneManager();
    ~SceneManager();

    QJsonObject loadFits(const QString &path);
    QJsonObject setCamera(const QJsonObject &params);
    QJsonObject renderPng(int width, int height);
    QJsonObject renderVolumePng(int width, int height, const QJsonObject &volumeParams);
    QJsonObject renderRaw(int width, int height, QByteArray &rgba, QByteArray &depth);

private:
    bool ensureScene();
    void resetScene();

    QString m_loadedSource;
    vtkRenderWindow *m_window;
    vtkRenderer *m_renderer;
    vtkImageActor *m_imageActor;
    vtkImageData *m_sliceImage;
    vtkImageData *m_volumeImage;
    vtkImageShiftScale *m_shift;
    vtkExtractVOI *m_sliceExtract;
    int m_numSlices{0};
    double m_range[2]{0.0, 1.0};
};
