#include "ImageLayerLoadTask.h"

#include "AstroUtils.h"
#include "vtkFITSReader.h"

#include "wcs.h"

#include <vtkImageData.h>
#include <vtkMath.h>
#include <vtkNew.h>

#include <cmath>

ImageLayerLoadResult loadImageLayer(const ImageLayerLoadRequest &request)
{
    ImageLayerLoadResult result;
    result.filepath = request.layerFilepath;

    if (request.masterFilepath.empty() || request.layerFilepath.empty()) {
        result.errorMessage = "Empty dataset path.";
        return result;
    }

    vtkNew<vtkFITSReader> reader;
    reader->SetFileName(request.layerFilepath.c_str());
    reader->Update();

    result.imageData = vtkSmartPointer<vtkImageData>::New();
    result.imageData->DeepCopy(reader->GetOutput());
    result.scalarRange = { reader->GetMin(), reader->GetMax() };

    AstroUtils astroMaster(request.masterFilepath);
    AstroUtils astroNewLayer(request.layerFilepath);

    const double scalingFactor = astroNewLayer.getSecPix() / astroMaster.getSecPix();
    result.spacing = { scalingFactor, scalingFactor, 1. };

    double posNewLayer[2];
    double pix[2] = { 0., 0. };
    astroNewLayer.xy2sky(pix, posNewLayer, WCS_J2000);
    astroMaster.sky2xy(posNewLayer, pix, WCS_J2000);
    result.origin = { pix[0], pix[1], 0. };

    pix[0] = 0.;
    pix[1] = 100.;
    astroNewLayer.xy2sky(pix, posNewLayer, WCS_J2000);
    astroMaster.sky2xy(posNewLayer, pix, WCS_J2000);
    const double m = std::abs((pix[1] - result.origin[1]) / (pix[0] - result.origin[0]));
    result.rotationDegrees = 90. - std::atan(m) * 180. / vtkMath::Pi();

    result.valid = true;
    return result;
}
