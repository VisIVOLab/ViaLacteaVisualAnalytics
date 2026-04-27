#ifndef WebViewProcess_h
#define WebViewProcess_h

#include <QObject>

/**
 * QObject needed to allow communication between the Panoramic View and VisIVO
 */
class WebViewProcess : public QObject
{
    Q_OBJECT
public:
    explicit WebViewProcess(QObject *parent = nullptr);

    static const QString ActivatePointSelection;
    static const QString ActivateRectangularSelection;

public slots:
    /**
   * From Panoramic View
   * @param point Comma-separated World Coordinates
   * @param area Comma-separated rectangular size, or empty string for Point
   * Selection
   */
    void jsCall(const QString &point, const QString &area);

signals:
    /**
      * Signal to notify VisIVO of the selection
      * @param point Comma-separated World Coordinates
      * @param area Comma-separated rectangular size, or empty string for Point
     */
    void processJavascript(const QString &point, const QString &area);
};

#endif