#include "vlvastackimage.h"
#include "errorMessage.h"

#include <pqDataRepresentation.h>
#include <pqLoadDataReaction.h>
#include <pqOutputPort.h>
#include <vtkPVArrayInformation.h>
#include <vtkPVDataInformation.h>
#include <vtkPVDataMover.h>
#include <vtkPVDataSetAttributesInformation.h>
#include <vtkSMCoreUtilities.h>
#include <vtkSMProperty.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMPVRepresentationProxy.h>
#include <vtkSMSessionProxyManager.h>
#include <vtkSMTransferFunctionManager.h>
#include <vtkSMTransferFunctionPresets.h>

#include <vtkVariant.h>

#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include <fitsio.h>

/**
 * @brief vlvaStackImage::vlvaStackImage
 * Default constructor. Sets or instantiates all the required components for this class instance to
 * successfully communicate with the server.
 * @param filePath The file path of the image managed by this class instance.
 * @param counter A unique integer, managed by the pqWindowClass, used to ensure unique colour map names.
 * @param log A toggle that determines if the image starts in log scale or linear scale.
 * @param bldr The pqObjectBuilder, managed by the pqWindowClass instance. Reused across all instances of
 *             vlvaStackImage controlled by a single pqWindowClass instance.
 * @param vwImg The renderview that the image managed by this class instance should be displayed in.
 * @param spm The sessionproxymanager, used to generate proxies as used by this class instance. Reused across all instances of
 *             vlvaStackImage controlled by a single pqWindowClass instance.
 */
vlvaStackImage::vlvaStackImage(QString filePath, int counter, bool log, pqObjectBuilder* bldr, pqRenderView *vwImg, vtkSMSessionProxyManager *spm) : logScale(log),
                                                                                                           active(true), builder(bldr), viewImage(vwImg), serverProxyManager(spm)
{
    initialised = false;
    FitsFileName = QFileInfo(filePath).fileName();
    
    // Create a unique color map proxy for each instance
    vtkNew<vtkSMTransferFunctionManager> mgr;
    // Generate a unique name for the color map proxy
    std::string colorMapName = "ColorMap_" + std::to_string(counter);
    lutProxy = vtkSMTransferFunctionProxy::SafeDownCast(mgr->GetColorTransferFunction(colorMapName.c_str(), spm));
}

vlvaStackImage::~vlvaStackImage()
{
//    if (initialised) builder->destroy(imageSource); //Destructor causes issues, don't know why.
    imageSource = NULL;
    setActive(false);
}

/**
 * @brief vlvaStackImage::init
 * This function checks that the image to be loaded conforms to the correct type
 * to be managed by this class. Also instantiates the remaining proxies not needed on construction.
 * @param f The file path of the image.
 * @param subset The subset values chosen by the user for this image.
 * @return 1 if successful, 0 if unsuccessful.
 */
int vlvaStackImage::init(QString f, CubeSubset subset)
{
    try {
        initialised = true;
        imageSource = pqLoadDataReaction::loadData({ f });

        // Handle Subset selection
        setSubsetProperties(subset);
        if (!checkValid())
        {
            std::stringstream eString, eInfo;
            eString << "File " << f.toStdString() << " is a cube!";
            eInfo << "Images must be 2D FITS images, with NAXES == 2.";
            throwError(eString.str().c_str(), eInfo.str().c_str());
            return 2;
        }

        imageRep = builder->createDataRepresentation(this->imageSource->getOutputPort(0), viewImage);
        imageProxy = imageRep->getProxy();
        vtkSMPropertyHelper(imageProxy, "Representation").Set("Slice");
        vtkSMProperty* separateProperty = vtkSMPVRepresentationProxy::SafeDownCast(imageProxy)->GetProperty("UseSeparateColorMap");
        vtkSMPropertyHelper(separateProperty).Set(1);
        vtkSMPVRepresentationProxy::SetScalarColoring(imageProxy, "FITSImage", vtkDataObject::POINT);
        imageProxy->UpdateVTKObjects();

        readInfoFromSource();
        readHeaderFromSource();
        fitsHeaderPath = createFitsHeaderFile(fitsHeader);
        // Set default colour map
        changeColorMap("Grayscale");
        setLogScale(this->logScale);
        setOpacity(1);
        setActive(true);
        type = 0;
        pixCount = std::make_pair(fitsHeader.value("NAXIS1").toInt(), fitsHeader.value("NAXIS2").toInt());
    }
    catch (std::exception& e)
    {
        initialised = false;
        std::cerr << "Error when loading vlvaStackImage! Error: " << e.what() << std::endl;
        return 0;
    }
    return 1;
}

/**
 * @brief vlvaStackImage::getIndex
 * @return The index of the image managed by this class instance in the stack
 */
size_t vlvaStackImage::getIndex() const
{
    return this->index;
}

/**
 * @brief vlvaStackImage::getFitsHeaderPath
 * @return The file path of the FitsHeader used by interactors and the like to interact with this image's FITS header.
 */
const std::string vlvaStackImage::getFitsHeaderPath() const
{
    if (initialised) return fitsHeaderPath.toStdString();
    std::cerr << "StackImage not initialised, returning default value." << std::endl;
    return "";
}

/**
 * @brief vlvaStackImage::getFitsFileName
 * @return The file path of the image managed by this class instance.
 */
const QString vlvaStackImage::getFitsFileName() const
{
    if (initialised) return FitsFileName;
    std::cerr << "StackImage not initialised, returning default value." << std::endl;
    return "";
}

/**
 * @brief vlvaStackImage::operator <
 * Compare this class instance to another class instance.
 * @param other The other class instance to be compared to.
 * @return True if the index of this class instance is less than the index of the other class instance.
 */
bool vlvaStackImage::operator<(const vlvaStackImage &other) const
{
    return this->getIndex() < other.getIndex();
}

/**
 * @brief vlvaStackImage::operator ==
 * Compare this class instance to another class instance.
 * @param other The other class instance to be compared to.
 * @return True if the file path of this class instance's image is the same
 * as the file path of the other class instance's image.
 */
bool vlvaStackImage::operator==(const vlvaStackImage &other) const
{
    if (this->getFitsFileName() != other.getFitsFileName())
        return false;
    return true;
}

/**
 * @brief vlvaStackImage::getImageRep
 * @return A pointer to the client side data representation for this image,
 * used for removing this image and the image's colour legend bar
 */
pqDataRepresentation *vlvaStackImage::getImageRep() const
{
    return imageRep;
}

/**
 * @brief vlvaStackImage::getPixCount
 * @return The number of pixels in the image, calculated on intialisation.
 */
const std::pair<int, int> vlvaStackImage::getPixCount() const
{
    return pixCount;
}

/**
 * @brief vlvaStackImage::setSubsetProperties
 * Function sets load properties for the FitsReader plugin on the server for this image.
 * @param subset The user-selected values for the subset of this image.
 * @return 1 if successful, 0 if unsuccessful
 */
int vlvaStackImage::setSubsetProperties(CubeSubset& subset)
{
    if (initialised) {
        try
        {
            auto sourceProxy = imageSource->getProxy();
            vtkSMPropertyHelper(sourceProxy, "ReadSubExtent").Set(subset.ReadSubExtent);
            vtkSMPropertyHelper(sourceProxy, "SubExtent").Set(subset.SubExtent, 6);
            vtkSMPropertyHelper(sourceProxy, "AutoScale").Set(subset.AutoScale);
            vtkSMPropertyHelper(sourceProxy, "CubeMaxSize").Set(subset.CubeMaxSize);
            if (!subset.AutoScale)
                vtkSMPropertyHelper(sourceProxy, "ScaleFactor").Set(subset.ScaleFactor);
            sourceProxy->UpdateVTKObjects();
            return 1;
        }
        catch (std::exception& e)
        {
            std::cerr << "Error when setting subset properties of stack image! Error: " << e.what() << std::endl;
            return 0;
        }
    }
    std::cerr << "StackImage not initialised, returning default value." << std::endl;
    return 0;
}

/**
 * @brief vlvaStackImage::setActive
 * Function to set the image visible or invisible (called when selecting the checkbox in the list of images in the stack)
 * @param act True if set visible, false if to set invisible.
 * @return 1 if successful, 0 if unsuccessful
 */
int vlvaStackImage::setActive(bool act)
{
    if (initialised)
    {
        try
        {
            if (act){
                vtkSMPropertyHelper(imageProxy->GetProperty("Visibility")).Set(1);
                setOpacity(opacity, false);
                this->active = true;
            }
            else{
                vtkSMPropertyHelper(imageProxy->GetProperty("Visibility")).Set(0);
                setOpacity(0, false);
                this->active = false;
            }
        }
        catch (std::exception& e)
        {
            std::cerr << "Error when attempting to set visibility of vlvaStackImage! Error: " << e.what() << std::endl;
            return 0;
        }
        return 1;
    }
    std::cerr << "StackImage not initialised, returning default value." << std::endl;
    return 0;
}

/**
 * @brief vlvaStackImage::isEnabled
 * @return True if the image is visible, false if it is invisible
 */
const bool vlvaStackImage::isEnabled() const
{
    return this->active;
}

/**
 * @brief vlvaStackImage::getColourMap
 * @return The name of the colour map applied to this image.
 */
const QString vlvaStackImage::getColourMap() const
{
    return this->colourMap;
}

/**
 * @brief vlvaStackImage::readInfoFromSource
 * Function that extracts the information of the image
 * from the source.
 */
void vlvaStackImage::readInfoFromSource()
{
    if (initialised)
    {
        // Bounds
        auto fitsInfo = this->imageSource->getOutputPort(0)->getDataInformation();
        fitsInfo->GetBounds(bounds);

               // Data range
        auto fitsImageInfo = fitsInfo->GetPointDataInformation()->GetArrayInformation("FITSImage");
        double dataRange[2];
        fitsImageInfo->GetComponentRange(0, dataRange);
        dataMin = dataRange[0];
        dataMax = dataRange[1];
    }
    else
    {
        std::cerr << "StackImage not initialised, returning default value." << std::endl;
        return;
    }
}

/**
 * @brief vlvaStackImage::readHeaderFromSource
 * Function that retrieves the FITS header information from
 * the file loaded on the server.
 */
void vlvaStackImage::readHeaderFromSource()
{
    if (initialised)
    {
        auto dataMoverProxy = vtk::TakeSmartPointer(serverProxyManager->NewProxy("misc", "DataMover"));
        vtkSMPropertyHelper(dataMoverProxy, "Producer").Set(this->imageSource->getProxy());
        vtkSMPropertyHelper(dataMoverProxy, "PortNumber").Set(1);
        vtkSMPropertyHelper(dataMoverProxy, "SkipEmptyDataSets").Set(1);
        dataMoverProxy->UpdateVTKObjects();
        dataMoverProxy->InvokeCommand("Execute");

        auto dataMover = vtkPVDataMover::SafeDownCast(dataMoverProxy->GetClientSideObject());
        int datasets = dataMover->GetNumberOfDataSets();
        for (int table = 0; table < datasets; ++table) {
            vtkTable *headerTable = vtkTable::SafeDownCast(dataMover->GetDataSetAtIndex(table));
            for (vtkIdType i = 0; i < headerTable->GetNumberOfRows(); ++i) {
                fitsHeader.insert(QString::fromStdString(headerTable->GetValue(i, 0).ToString()),
                                  QString::fromStdString(headerTable->GetValue(i, 1).ToString()));
            }
        }
    }
    else
    {
        std::cerr << "StackImage not initialised, returning default value." << std::endl;
        return;
    }
}

/**
 * @brief vlvaStackImage::checkValid
 * Function checks if the image is a proper 2D FITS image (rather than cube)
 * @return True if the image is a 2D FITS image, false if it is a cube or poorly formed.
 */
bool vlvaStackImage::checkValid()
{
    if (initialised)
    {
        try
        {
            auto sourceProxy = imageSource->getProxy();
            sourceProxy->UpdateVTKObjects();
            int imType = 0;
            vtkSMPropertyHelper(sourceProxy, "ImgType").Get(&imType);
            if (imType == imageType::FITS2DIMAGE)
                return true;
        }
        catch (std::exception& e)
        {
            std::cerr << "Error when retrieving image type property of stack image! Error: " << e.what() << std::endl;
            return false;
        }
    }
    return false;
}

/**
 * @brief vlvaStackImage::createFitsHeaderFile
 * This function creates a temporary local copy of the FITS header of this image,
 * used by the AstroWCS library functions.
 * @param fitsHeader The FITS header of this image, pulled from the server
 * @return The file path to the new local copy of the FITS header
 */
QString vlvaStackImage::createFitsHeaderFile(const FitsHeaderMap &fitsHeader)
{
    if (initialised)
    {
        fitsfile *fptr;
        int status = 0;

        QString headerFile = QDir::homePath()
                                     .append(QDir::separator())
                                     .append("VisIVODesktopTemp")
                                     .append(QDir::separator())
                                     .append(this->FitsFileName);

        fits_create_file(&fptr, ("!" + headerFile).toStdString().c_str(), &status);
        fits_update_key_log(fptr, "SIMPLE", TRUE, "", &status);

        int datatype;
        foreach (const auto &key, fitsHeader.keys()) {
            const auto &value = fitsHeader[key];
            if (key.compare("SIMPLE") == 0) {
                continue;
            }

            if (strchr(value.toStdString().c_str(), '\'')) {
                datatype = TSTRING;
            } else if (strchr(value.toStdString().c_str(), '\"'))
                datatype = TSTRING;
            else if (strchr(value.toStdString().c_str(), '.'))
                datatype = TDOUBLE;
            else if (isdigit(value.toStdString().c_str()[0]))
                datatype = TLONG;
            else if (strcasecmp(value.toStdString().c_str(), "TRUE") == 0
                     || strcasecmp(value.toStdString().c_str(), "FALSE") == 0)
                datatype = TLOGICAL;
            else
                datatype = TLONG;

            switch (datatype) {
            case TSTRING: {
                // remove quotes
                char *cstr = new char[value.toStdString().length() + 1];
                std::strcpy(cstr, value.toStdString().c_str());
                std::string str(cstr);
                std::string result = str.substr(1, str.size() - 2);

                fits_update_key_str(fptr, key.toStdString().c_str(), result.c_str(), "", &status);
                break;
            }
            case TFLOAT:
                fits_update_key_flt(fptr, key.toStdString().c_str(), atof(value.toStdString().c_str()),
                                    -7, "", &status);
                break;
            case TDOUBLE:
                fits_update_key_dbl(fptr, key.toStdString().c_str(), atof(value.toStdString().c_str()),
                                    -15, "", &status);
                break;
            case TLONG: {
                fits_update_key_lng(fptr, key.toStdString().c_str(), atol(value.toStdString().c_str()),
                                    "", &status);
                break;
            }
            }
        }
        fits_close_file(fptr, &status);

        return headerFile;
    }
    else
    {
        std::cerr << "StackImage not initialised, returning default value." << std::endl;
        return "";
    }
}

/**
 * @brief vlvaStackImage::changeColorMap
 * Changes the colour map used to visualize the FITS image.
 * @param name The name of the colour map (provided by paraview presets)
 */
int vlvaStackImage::changeColorMap(const QString &name)
{
    if (initialised)
    {
        if (vtkSMProperty *lutProperty = imageProxy->GetProperty("LookupTable")) {
            auto presets = vtkSMTransferFunctionPresets::GetInstance();
            lutProxy->ApplyPreset(presets->GetFirstPresetWithName(name.toStdString().c_str()));
            vtkSMPropertyHelper(lutProperty).Set(lutProxy);
            lutProxy->UpdateVTKObjects();
            vtkSMPVRepresentationProxy::RescaleTransferFunctionToDataRange(imageProxy, false, false);

            vtkSMPVRepresentationProxy::SetScalarBarVisibility(imageProxy, viewImage->getProxy(), true);

            imageProxy->UpdateVTKObjects();
            this->colourMap = name;
            return 1;
        }
        return 0;
    }
    else
    {
        std::cerr << "StackImage not initialised, returning default value." << std::endl;
        return 0;
    }
}

/**
 * @brief vlvaStackImage::getLogScale
 * @return True if the image is using log scaling, false if it is using linear scaling.
 */
const bool vlvaStackImage::getLogScale() const
{
    return this->logScale;
}

/**
 * @brief vlvaStackImage::setLogScale
 * Sets the image visualization to use log10 scaling on the colour map.
 * @param logScale
 * A boolean whether or not to use log10 scaling.
 */
int vlvaStackImage::setLogScale(bool useLog)
{
    if (initialised)
    {
        if (useLog) {
            if (auto logProperty = lutProxy->GetProperty("UseLogScale"))
            {
                double range[2];
                vtkSMTransferFunctionProxy::GetRange(lutProxy, range);
                vtkSMCoreUtilities::AdjustRangeForLog(range);

                this->logScale = true;
                vtkSMTransferFunctionProxy::RescaleTransferFunction(lutProxy, range);
                vtkSMPropertyHelper(logProperty).Set(1);

                lutProxy->UpdateVTKObjects();
                imageProxy->UpdateVTKObjects();
                changeColorMap(this->getColourMap());
                return 1;
            }
            else
            {
                std::cerr << "Error with logscale proxy: not found correctly." << std::endl;
                return 0;
            }
        } else {
            if (auto logProperty = lutProxy->GetProperty("UseLogScale"))
            {
                this->logScale = false;
                vtkSMTransferFunctionProxy::RescaleTransferFunctionToDataRange(lutProxy);
                vtkSMPropertyHelper(logProperty).Set(0);

                lutProxy->UpdateVTKObjects();
                imageProxy->UpdateVTKObjects();
                changeColorMap(this->getColourMap());
                return 1;
            }
            else
            {
                std::cerr << "Error with logscale proxy: not found correctly." << std::endl;
                return 0;
            }
        }
    }
    else
    {
        std::cerr << "StackImage not initialised, returning default value." << std::endl;
        return 0;
    }
}

/**
 * @brief vlvaStackImage::getOpacity
 * @return The opacity of the image, in range [0, 1].
 */
const double vlvaStackImage::getOpacity() const
{
    return this->opacity;
}

/**
 * @brief vlvaStackImage::setOpacity
 * Function that sets the image opacity according to the given value.
 * @param value A value between 0 and 1, with 0 being fully transparent and 1 being fully opaque.
 * @param updateVal Set if the image opacity should be updated or if this is being set as a "removal".
 * @return 1 if successful, 0 if unsuccessful
 */
int vlvaStackImage::setOpacity(float value, bool updateVal)
{
    if (initialised)
    {
        if (vtkSMProperty *opacityProperty = imageProxy->GetProperty("Opacity")) {

            vtkSMPropertyHelper(opacityProperty).Set(value);
            imageProxy->UpdateVTKObjects();

            if (updateVal)
            {
                opacity = value;
            }

            return 1;
        }
        return 0;
    }
    else
    {
        std::cerr << "StackImage not initialised, returning default value." << std::endl;
        return 0;
    }
}

/**
 * @brief vlvaStackImage::setXYPosition
 * Set the XY position of the image (Z is set by the index)
 * @param x The x coordinate of the bottom left of the image
 * @param y The y coordinate of the bottom left of the image
 * @return 1 if successful, 0 if unsuccessful
 */
int vlvaStackImage::setXYPosition(double x, double y)
{
    if (initialised)
    {
        xyPosition.first = x;
        xyPosition.second = y;
        return this->setZPosition();
    }
    else
    {
        std::cerr << "StackImage not initialised, returning default value." << std::endl;
        return 0;
    }
}

/**
 * @brief vlvaStackImage::getXYPosition
 * @return The
 */
const std::pair<double, double> vlvaStackImage::getXYPosition() const
{
    return this->xyPosition;
}

/**
 * @brief vlvaStackImage::setZPosition
 * This function sets the Z position of the image
 * @return 1 if successful, 0 if unsuccessful
 */
int vlvaStackImage::setZPosition()
{
    if (initialised)
    {
        if (auto positionProperty = imageProxy->GetProperty("Position"))
        {
            double* val = new double[3];
            val[0] = xyPosition.first;
            val[1] = xyPosition.second;
            val[2] = static_cast<double>(index);
            vtkSMPropertyHelper(positionProperty).Set(val, 3);
            imageProxy->UpdateVTKObjects();
            delete[] val;
            return 1;
        }
        std::cerr << "Error with position proxy: not found correctly." << std::endl;
        return 0;
    }
    else
    {
        std::cerr << "StackImage not initialised, returning default value." << std::endl;
        return 0;
    }
}

/**
 * @brief vlvaStackImage::setScale
 * Function that sets the scale of the image according to WCS coordinates
 * @param x The scaling along the x-axis
 * @param y The scaling along the y-axis
 * @param z The scaling along the z-axis (always 1)
 * @return 1 if successful, 0 if unsuccessful
 */
int vlvaStackImage::setScale(double x, double y, double z)
{
    if (initialised)
    {
        if (auto scaleProperty = imageProxy->GetProperty("Scale"))
        {
            double* val = new double[3];
            val[0] = x;
            val[1] = y;
            val[2] = z;
            vtkSMPropertyHelper(scaleProperty).Set(val, 3);
            imageProxy->UpdateVTKObjects();
            delete[] val;
            this->scale = std::make_tuple(x, y, z);
            return 1;
        }
        std::cerr << "Error with scale proxy: not found correctly." << std::endl;
        return 0;
    }
    else
    {
        std::cerr << "StackImage not initialised, returning default value." << std::endl;
        return 0;
    }
}

/**
 * @brief vlvaStackImage::setOrientation
 * Function sets the orientation of the image (as determined by WCS)
 * @param phi The phi angle of the image (about the x-axis)
 * @param theta The theta angle of the image (about the y-axis)
 * @param psi The psi angle of the image (about the z-axis)
 * @return 1 if successful, 0 if unsuccessful
 */
int vlvaStackImage::setOrientation(double phi, double theta, double psi)
{
    if (initialised)
    {
        if (auto orientationProperty = imageProxy->GetProperty("Orientation"))
        {
            double* val = new double[3];
            val[0] = phi;
            val[1] = theta;
            val[2] = psi;
            vtkSMPropertyHelper(orientationProperty).Set(val, 3);
            std::get<0>(this->angle) = phi;
            std::get<1>(this->angle) = theta;
            std::get<2>(this->angle) = psi;
            imageProxy->UpdateVTKObjects();
            delete[] val;
            return 1;
        }
        std::cerr << "Error with orientation proxy: not found correctly." << std::endl;
        return 0;
    }
    else
    {
        std::cerr << "StackImage not initialised, returning default value." << std::endl;
        return 0;
    }
}

/**
 * @brief vlvaStackImage::getOrientation
 * @return Return the orientation of this image
 */
const std::tuple<double, double, double> vlvaStackImage::getOrientation() const
{
    return this->angle;
}

/**
 * @brief vlvaStackImage::setIndex
 * Function that sets the index of this image in the stack (and the z-axis)
 * @param val The index, range [0, N) where N is the number of images in the stack.
 */
void vlvaStackImage::setIndex(const size_t val)
{
    this->index = val;
}
