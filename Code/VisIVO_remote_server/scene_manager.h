#pragma once

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <vtkSmartPointer.h>
#include <limits>

class vtkRenderWindow;
class vtkRenderer;
class vtkImageActor;
class vtkImageData;
class vtkImageShiftScale;
class vtkExtractVOI;
class vtkMarchingCubes;
class vtkPolyDataNormals;
class vtkPolyDataMapper;
class vtkActor;

class SceneManager {
public:
    SceneManager();
    ~SceneManager();

    QJsonObject loadFits(const QString &path);
    QJsonObject setCamera(const QJsonObject &params);
    QJsonObject setSlice(int slice);
    QJsonObject setWindowLevel(double window, double level);
    QJsonObject setRange(double min, double max);
    QJsonObject renderPng(int width, int height, const QString &quality = QString());
    QJsonObject renderVolumePng(int width, int height, const QJsonObject &volumeParams, const QString &quality = QString());
    QJsonObject renderContourPng(int width, int height, const QJsonObject &params, const QString &quality = QString());
    QJsonObject renderH264(int width, int height, const QString &quality = QString());
    QJsonObject renderH264Volume(int width, int height, const QJsonObject &volumeParams, const QString &quality = QString());
    QJsonObject renderH264Contour(int width, int height, const QJsonObject &params, const QString &quality = QString());
    QJsonObject rotateCamera(double yawDeg, double pitchDeg);
    QJsonObject panCamera(double dx, double dy);
    QJsonObject zoomCamera(double factor);
    QJsonObject renderRaw(int width, int height, QByteArray &rgba, QByteArray &depth);
    QJsonObject renderRawVolume(int width, int height, const QJsonObject &volumeParams, QByteArray &rgba, QByteArray &depth);
    QJsonObject renderRawContour(int width, int height, const QJsonObject &params, QByteArray &rgba, QByteArray &depth);
    QJsonObject setLut(const QJsonObject &params);

private:
    bool ensureScene();
    void resetScene();
    vtkImageData *mapWithLut(vtkImageData *src);

    QString m_loadedSource;
    vtkRenderWindow *m_window;
    vtkRenderer *m_renderer;
    vtkImageActor *m_imageActor;
    vtkImageData *m_sliceImage;
    vtkImageData *m_volumeImage;
    vtkImageShiftScale *m_shift;
    vtkExtractVOI *m_sliceExtract;
    vtkSmartPointer<vtkMarchingCubes> m_mc;
    vtkSmartPointer<vtkPolyDataNormals> m_mcNormals;
    vtkSmartPointer<vtkPolyDataMapper> m_volumeMapper;
    vtkSmartPointer<vtkActor> m_volumeActor;
    double m_lastIso{std::numeric_limits<double>::quiet_NaN()};
    int m_lastLod{1};
    bool m_cameraInitialized{false};
    int m_numSlices{0};
    double m_range[2]{0.0, 1.0};
    int m_currentSlice{0};
    bool m_hasWindowLevel{false};
    double m_windowWL{0.0};
    double m_level{0.0};
    QString m_lutScale{"Log"};
    QString m_lutType{"Gray"};
};
