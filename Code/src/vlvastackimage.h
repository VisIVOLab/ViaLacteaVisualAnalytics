#ifndef VLVASTACKIMAGE_H
#define VLVASTACKIMAGE_H

#include "subsetselectordialog.h"

#include "pqObjectBuilder.h"
#include "pqPipelineSource.h"
#include "pqRenderView.h"
#include "vtkSMTransferFunctionProxy.h"

#include <QMap>
#include <QString>

class vlvaStackImage
{
    using FitsHeaderMap = QMap<QString, QString>;

public:
        vlvaStackImage(QString f, int i, bool log, pqObjectBuilder* bldr, pqRenderView *viewImage, vtkSMSessionProxyManager* spm);
        ~vlvaStackImage();
        const vtkSMProxy* getProxy() const;
        const vtkSMTransferFunctionProxy* getLutProxy() const;
        pqPipelineSource* getSource();

        int setActive(bool act);
        const QString getColourMap() const;
        int changeColorMap(const QString &name);
        const bool getLogScale() const;
        int setLogScale(bool logScale);
        const double getOpacity() const;
        int setOpacity(float value, bool updateVal = true);
        int setPosition();
        int init(QString f, CubeSubset subset);

        const std::string getFitsHeaderPath() const;
        const QString getFitsFileName() const;
    private:
        int index;
        bool logScale, active;
        bool initialised;
        QString colourMap;
        double opacity;

        vtkSMSessionProxyManager* serverProxyManager;
        pqObjectBuilder* builder;
        pqPipelineSource* imageSource;
        vtkSMProxy* imageProxy;
        vtkSMTransferFunctionProxy* lutProxy;
        pqRenderView* viewImage;

        QString FitsFileName;
        QString fitsHeaderPath;
        FitsHeaderMap fitsHeader;
        double bounds[6];
        double dataMin;
        double dataMax;
        double rms;
        double lowerBound;
        double upperBound;

        int setSubsetProperties(CubeSubset& subset);
        QString createFitsHeaderFile(const FitsHeaderMap &fitsHeader);
        void readInfoFromSource();
        void readHeaderFromSource();
};

#endif // VLVASTACKIMAGE_H
