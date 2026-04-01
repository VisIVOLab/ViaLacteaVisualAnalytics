#ifndef AstroUtils_h
#define AstroUtils_h

#include <fitsio.h>

#include <string>

struct WorldCoor;

/**
 * Util class to get FITS Header keywords and perform WCS conversions
 */
class AstroUtils
{
public:
    /**
   * Util class to get FITS Header keywords and perform WCS conversions
   * @param filepath FITS absolute filepath
   */
    AstroUtils(const std::string &filepath);
    ~AstroUtils();

    /**
   * Convert pixel coordinates to World Coordinates
   * @param pix Pixel coordinates
   * @param pos World Coordinates (returned)
   * @param syswcs Output WCS
   */
    void xy2sky(const double *pix, double *pos, int syswcs);

    /**
   * Convert World Coordinates to pixel coordinates
   * @param pos World Coordinates
   * @param pix Pixel coordinates (returned)
   * @param syswcs Input WCS
   */
    void sky2xy(const double *pos, double *pix, int syswcs);

    /**
   * @return Arcseconds per pixel
   */
    double getSecPix() const;

    /**
   * @return BUNIT value or empty string
   */
    std::string getPhysicalUnit() const;

    /**
   * @param axis (0-index)
   * @return CUNIT value or empty string
   */
    std::string getAxisUnit(int axis) const;

    /**
   * @param axis (0-index)
   * @return CTYPE value or empty string
   */
    std::string getAxisType(int axis) const;

    /**
   * @return NAXIS values
   */
    const int *getDimensions() const;

    /**
   * @return CDELT values
   */
    const double *getIncrements() const;

    /**
   * @return Spectral value at first slice
   */
    double getInitialSpectralValue() const;

    /**
   * Return the bounds in World Coordinates
   * @param ra_min Right Ascension min value
   * @param ra_max Right Ascension max value
   * @param dec_min Declination min value
   * @param dec_max Declination max value
   * @param syswcs Output WCS
   */
    void getBounds(double *ra_min, double *ra_max, double *dec_min, double *dec_max,
                   int syswcs) const;

    /**
   * Check if two files overlap each other
   * @param other Absolute FITS filepath
   * @return true if there is an overlap
   */
    bool overlap(const std::string &filepath) const;

    /**
   * @return true if FITS file has exactly 2 axes
   */
    bool isImage() const;

    /**
   * @return true if FITS file has at least 3 axes
   */
    bool isCube() const;

    /**
   * @return true if FITS file has 4 axes
   */
    bool hasStokes() const;

    /**
   * @return true if FITS file has no WCS keywords or WCS = XY | LINEAR
   */
    bool isSimulation() const;

private:
    std::string filepath;
    struct WorldCoor *wcs;
    int naxis;
    double secpix;
    char bunit[FLEN_VALUE];

    int naxes[4];
    double crpix[4];
    double crval[4];
    double cdelt[4];
    char ctype[4][FLEN_VALUE];
    char cunit[4][FLEN_VALUE];
};

#endif
