#ifndef VLVASTACKIMAGE_H
#define VLVASTACKIMAGE_H

#include "src/astroutils.h"
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

        vtkSMProxy* getProxy() const { return imageProxy; };
        vtkSMTransferFunctionProxy* getLutProxy() const { return lutProxy; };
        pqPipelineSource* getSource() const { return imageSource; };

        int setActive(bool act);
        const bool isEnabled() const;

        const QString getColourMap() const;
        int changeColorMap(const QString &name);

        const bool getLogScale() const;
        int setLogScale(bool logScale);

        const double getOpacity() const;
        int setOpacity(float value, bool updateVal = true);

        int setXYPosition(double x, double y);
        const std::pair<double, double> getXYPosition() const;
        const std::pair<int, int> getPixCount() const;
        int setZPosition();
        int setScale(double x, double y, double z = 1);
        const std::tuple<double, double, double> getScale() const;
        int setOrientation(double phi, double theta, double psi);
        const std::tuple<double, double, double> getOrientation() const;
        void setIndex(const size_t val);
        size_t getIndex() const;

        int init(QString f, CubeSubset subset);

        const std::string getFitsHeaderPath() const;
        const QString getFitsFileName() const;

        int getType() const { return type; };

        bool operator<(const vlvaStackImage& other) const;
        bool operator==(const vlvaStackImage& other) const;

    private:
        int index;
        bool logScale, active;
        bool initialised;
        QString colourMap;
        double opacity;

        std::tuple<double, double, double> angle;
        std::pair<double, double> xyPosition;
        std::pair<int, int> pixCount;
        std::tuple<double, double, double> scale;

        vtkSMSessionProxyManager* serverProxyManager;
        pqObjectBuilder* builder;
        pqPipelineSource* imageSource;
        pqDataRepresentation* imageRep;
        vtkSMProxy* imageProxy;
        vtkSMTransferFunctionProxy* lutProxy;
        pqRenderView* viewImage;

        /*
         * type
         * 0 = image
         * 1 = sigleband
         * 2 = centroid
         * 3 = isocontour
         */
        int type;

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

static bool overlaps(const std::vector<vlvaStackImage*>& imgs, const vlvaStackImage* evalImg){
    for (auto i : imgs){
        if (i == evalImg)
            continue;
        if (AstroUtils().CheckOverlap(i->getFitsHeaderPath(), evalImg->getFitsHeaderPath()))
            return 1;
    }
    return 0;
}

#endif // VLVASTACKIMAGE_H
