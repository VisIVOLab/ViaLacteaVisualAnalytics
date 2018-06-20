#ifndef ASTROUTILS_H
#define ASTROUTILS_H

#include <QString>
#include "libwcs/wcs.h"

class AstroUtils
{
public:
    AstroUtils();
    static bool sky2xy(std::string map,double ra, double dec,double* coord);

    static void xy2sky(std::string map, float x, float y, double *coord,  int wcs_type=WCS_ECLIPTIC);
//    static void xy2sky(QString map, float x, float y, double *coord,  int wcs_type=WCS_ECLIPTIC);
//    static bool sky2xy(QString map, double l, double b, double* coord);
    static double arcsecPixel(std::string file);
    static void getRotationAngle(std::string file);
    static int getSysOut( std::string file );


private:
    static WorldCoor* GetWCSFITS (char *filename, int verbose);
    static char * GetFITShead (char * filename, int verbose);
    static WorldCoor* GetFITSWCS (char *filename, char	*header, int verbose, double *cra, double *cdec,double *dra, double *ddec, double *secpix, int *wp,int	*hp,int	*sysout ,double	*eqout);
    static WorldCoor* ChangeFITSWCS (char *filename, char* header, int verbose);


};

#endif // ASTROUTILS_H
