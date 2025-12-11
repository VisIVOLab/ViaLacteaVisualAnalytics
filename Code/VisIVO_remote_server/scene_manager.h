#pragma once

#include <QString>
#include <QJsonObject>
#include <QJsonArray>

class vtkRenderWindow;
class vtkRenderer;
class vtkImageActor;
class vtkImageData;
class vtkImageShiftScale;

class SceneManager {
public:
    SceneManager();
    ~SceneManager();

    QJsonObject loadFits(const QString &path);
    QJsonObject setCamera(const QJsonObject &params);
    QJsonObject renderPng(int width, int height);

private:
    bool ensureScene();
    void resetScene();

    QString m_loadedSource;
    vtkRenderWindow *m_window;
    vtkRenderer *m_renderer;
    vtkImageActor *m_imageActor;
    vtkImageData *m_sliceImage;
    vtkImageShiftScale *m_shift;
    int m_numSlices{0};
    double m_range[2]{0.0, 1.0};
};
