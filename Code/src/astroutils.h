#ifndef ASTROUTILS_H
#define ASTROUTILS_H

#include <QPair>
#include <QVector>

#include <list>
#include <string>

#include "libwcs/fitswcs.h"

class AstroUtils
{
public:
    AstroUtils();
    static void sky2xy_t(std::string map, double xpos, double ypos, int wcs_type, double *xpix,
                         double *ypix);
    static bool sky2xy(std::string map, double ra, double dec, double *coord);
    static void xy2sky(std::string map, float x, float y, double *coord,
                       int wcs_type = WCS_ECLIPTIC);
    static double arcsecPixel(std::string file);
    static void getRotationAngle(std::string file, double *rot, int wcs_type = WCS_GALACTIC);
    static int getSysOut(std::string file);
    static void GetCenterCoords(std::string file, double *coords);
    static void GetRectSize(std::string file, double *values);
    static double GetRadiusSize(std::string file);
    static bool CheckOverlap(std::string f1, std::string f2, bool full = false);
    static int calculateResizeFactor(long size, long maxSize);
    static bool checkSimCubeHeader(const std::string &file,
                                   std::list<std::string> &missingKeywords);
    static QPair<QVector<double>, QVector<double>> extractSpectrum(const char *fn, int x, int y,
                                                                   double nulval);
    static bool isFitsImage(const std::string &filename);

    static void setMomentFITSHeader(const std::string &src, const std::string &dst, int order);

private:
    static WorldCoor *GetWCSFITS(char *filename, int verbose);
    static char *GetFITShead(char *filename, int verbose);
    static WorldCoor *GetFITSWCS(char *filename, char *header, int verbose, double *cra,
                                 double *cdec, double *dra, double *ddec, double *secpix, int *wp,
                                 int *hp, int *sysout, double *eqout);
    static WorldCoor *ChangeFITSWCS(char *filename, char *header, int verbose);
    static void GetBounds(std::string file, double *ra_min, double *ra_max, double *dec_min,
                          double *dec_max);
    static bool CheckFullOverlap(std::string f1, std::string f2);
};

#endif // ASTROUTILS_H
