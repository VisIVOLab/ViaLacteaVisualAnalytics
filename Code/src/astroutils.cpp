#include "astroutils.h"
#include <QDebug>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include "libwcs/wcs.h"
#include "libwcs/fitsfile.h"
#include "libwcs/wcscat.h"
#include "libwcs/lwcs.h"

#include <iostream>
#include <vector>
#include <string>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string.hpp>
using namespace boost::algorithm;


extern void setsys();

AstroUtils::AstroUtils()
{
}


double AstroUtils::arcsecPixel(std::string file)
{

    char *fn= new char[file.length() + 1];
    strcpy(fn, file.c_str());

    struct WorldCoor *wcs;
    char *header;
    double cra, cdec, dra, ddec, secpix;
    int wp, hp;
    int sysout = 0;
    double eqout = 0.0;

    header = GetFITShead (fn, 0);

    wcs= GetFITSWCS (fn,header,0,&cra,&cdec,&dra,&ddec,&secpix, &wp, &hp, &sysout, &eqout);

    return secpix;
}


//void AstroUtils::xy2sky(QString map,float x, float y,double* coord,int wcs_type)
void AstroUtils::xy2sky(std::string map,float x, float y,double* coord,int wcs_type)
{
    struct WorldCoor *wcs;
    char *fn=new char[map.length() + 1];;
    static char coorsys[16];
    char wcstring[64];
    char lstr = 64;
    *coorsys = 0;



    strcpy(fn, map.c_str());
    //    fn=map.c_str();
    wcs = GetWCSFITS (fn, 0);

    wcs->sysout = wcs_type;
    // force the set of wcs in degree


    setwcsdeg(wcs,1);
    if(wcs_type==WCS_GALACTIC)
    {
        wcs->eqout = 2000.0;
    }
    else if ( wcs_type = WCS_J2000)
    {
    }

    if (pix2wcst (wcs, x, y, wcstring, lstr))
    {

        std::string str(wcstring);
        using namespace boost::algorithm;
        std::vector<std::string> tokens;

        trim(str);

        split(tokens, str, is_any_of(" "),boost::token_compress_on);

        coord[0]=atof(tokens[0].c_str());
        coord[1]=atof(tokens[1].c_str());




    }

    delete [] fn;


}

int AstroUtils::getSysOut( std::string file )
{


    double x1,x2,y1,y2;
    double d1, d2, r, diffi;
    double pos1[3], pos2[3], w, diff;
    int i;

    x1=40.25058;

    y1=-0.98334;

    x2= 40.30152;

    y2=-0.30171;

    /* Convert two vectors to direction cosines */
    r = 1.0;
    d2v3 (x1, y1, r, pos1);
    d2v3 (x2, y2, r, pos2);

    /* Modulus squared of half the difference vector */
    w = 0.0;
    for (i = 0; i < 3; i++) {
        diffi = pos1[i] - pos2[i];
        w = w + (diffi * diffi);
        }
    w = w / 4.0;
    if (w > 1.0) w = 1.0;

    /* Angle beween the vectors */
    diff = 2.0 * atan2 (sqrt (w), sqrt (1.0 - w));
    diff = raddeg (diff);

    w = 0.0;
    d1 = 0.0;
    d2 = 0.0;
    for (i = 0; i < 3; i++) {
        w = w + (pos1[i] * pos2[i]);
        d1 = d1 + (pos1[i] * pos1[i]);
        d2 = d2 + (pos2[i] * pos2[i]);
        }
    diff = acosdeg (w / (sqrt (d1) * sqrt (d2)));






    char *fn= new char[file.length() + 1];
    strcpy(fn, file.c_str());

    struct WorldCoor *wcs;
    char *header;
    double cra, cdec, dra, ddec, secpix;
    int wp, hp;
    int sysout = 0;
    double eqout = 0.0;


    //sysout=WCS_GALACTIC;
    header = GetFITShead (fn, 0);

    wcs = GetFITSWCS (fn,header,1,&cra,&cdec,&dra,&ddec,&secpix, &wp, &hp, &sysout, &eqout);

    return wcs->sysout;
}

void AstroUtils::getRotationAngle( std::string file )
{


    //char *fn;
    // do stuff

    char *fn= new char[file.length() + 1];
    strcpy(fn, file.c_str());

    struct WorldCoor *wcs;
    char *header;
    double cra, cdec, dra, ddec, secpix;
    int wp, hp;
    int sysout = 0;
    double eqout = 0.0;


    header = GetFITShead (fn, 0);

    wcs = GetFITSWCS (fn,header,1,&cra,&cdec,&dra,&ddec,&secpix, &wp, &hp, &sysout, &eqout);



    wcsoutinit(wcs,"GALACTIC");


    int off;
    double xc, xn, xe, yc, yn, ye;

    /* If image is one-dimensional, leave rotation angle alone */
    if (wcs->nxpix < 1.5 || wcs->nypix < 1.5) {
        wcs->imrot = wcs->rot;
        wcs->pa_north = wcs->rot + 90.0;
        wcs->pa_east = wcs->rot + 180.0;
        return;
    }


    /* Do not try anything if image is LINEAR (not Cartesian projection) */
    if (wcs->syswcs == WCS_LINEAR)
        return;

    wcs->xinc = fabs (wcs->xinc);
    wcs->yinc = fabs (wcs->yinc);

    /* Compute position angles of North and East in image */
    xc = wcs->xrefpix;
    yc = wcs->yrefpix;
    pix2wcs (wcs, xc, yc, &cra, &cdec);
    if (wcs->coorflip) {
        wcs2pix (wcs, cra+wcs->yinc, cdec, &xe, &ye, &off);
        wcs2pix (wcs, cra, cdec+wcs->xinc, &xn, &yn, &off);
    }
    else {
        wcs2pix (wcs, cra+wcs->xinc, cdec, &xe, &ye, &off);
        wcs2pix (wcs, cra, cdec+wcs->yinc, &xn, &yn, &off);
    }
    wcs->pa_north = raddeg (atan2 (yn-yc, xn-xc));
    if (wcs->pa_north < -90.0)
        wcs->pa_north = wcs->pa_north + 360.0;
    wcs->pa_east = raddeg (atan2 (ye-yc, xe-xc));
    if (wcs->pa_east < -90.0)
        wcs->pa_east = wcs->pa_east + 360.0;

    /* Compute image rotation angle from North */
    if (wcs->pa_north < -90.0)
        wcs->imrot = 270.0 + wcs->pa_north;
    else
        wcs->imrot = wcs->pa_north - 90.0;

    /* Compute CROTA */
    if (wcs->coorflip) {
        wcs->rot = wcs->imrot + 90.0;
        if (wcs->rot < 0.0)
            wcs->rot = wcs->rot + 360.0;
    }
    else
        wcs->rot = wcs->imrot;
    if (wcs->rot < 0.0)
        wcs->rot = wcs->rot + 360.0;
    if (wcs->rot >= 360.0)
        wcs->rot = wcs->rot - 360.0;

    /* Set image mirror flag based on axis orientation */
    wcs->imflip = 0;
    if (wcs->pa_east - wcs->pa_north < -80.0 &&
            wcs->pa_east - wcs->pa_north > -100.0)
        wcs->imflip = 1;
    if (wcs->pa_east - wcs->pa_north < 280.0 &&
            wcs->pa_east - wcs->pa_north > 260.0)
        wcs->imflip = 1;
    if (wcs->pa_north - wcs->pa_east > 80.0 &&
            wcs->pa_north - wcs->pa_east < 100.0)
        wcs->imflip = 1;
    if (wcs->coorflip) {
        if (wcs->imflip)
            wcs->yinc = -wcs->yinc;
    }
    else {
        if (!wcs->imflip)
            wcs->xinc = -wcs->xinc;
    }


    delete [] fn;

}

//bool AstroUtils::sky2xy(QString map,double ra, double dec,double* coord)
bool AstroUtils::sky2xy(std::string map, double ra, double dec, double* coord)
{

    //char *fn;
    char *fn=new char[map.length() + 1];;

    struct WorldCoor *wcs;
    char *header;
    double cra, cdec, dra, ddec, secpix;
    int wp, hp;
    int sysout = 0;
    double eqout = 0.0;
    double x, y, ra0, dec0;
    int sysin;
    char csys[16];
    double eqin = 0.0;
    int offscale;

    strcpy(fn, map.c_str());
    //    fn=map.c_str();

    header = GetFITShead (fn, 0);

    wcs = GetFITSWCS (fn,header,0,&cra,&cdec,&dra,&ddec,&secpix, &wp, &hp, &sysout, &eqout);


    ra0=ra;
    dec0=dec;

    if (wcs->prjcode < 0)
        strcpy (csys, "PIXEL");
    else if (wcs->prjcode < 2)
        strcpy (csys, "LINEAR");
    else
        strcpy (csys, wcs->radecsys);

    sysin = wcscsys (csys);
    eqin = wcsceq (csys);

    if (wcs->syswcs > 0 && wcs->syswcs != 6 && wcs->syswcs != 10)
        wcscon (sysin, wcs->syswcs, eqin, eqout, &ra, &dec, wcs->epoch);



    wcsc2pix (wcs, ra0, dec0, csys, &x, &y, &offscale);


    //    delete[] fn;
    delete[] header;
    delete[] wcs;


    /*
 // COMMENTATO FV PER AGGIUNGERE LAYER, ANCHE SE IMMAGINE E' FUORI RESTITUISCE OFFSET

    if (offscale == 2){
        return false;
    }
    else if (offscale){
        return false;
    }
*/
    coord[0]= x;
    coord[1]= y;
    coord[2]= secpix;



    delete []fn;
    return true;
}


//QUI
static double secpix0 = PSCALE;		/* Set image scale--override header */
static int usecdelt = 0;		/* Use CDELT if 1, else CD matrix */
static int hp0 = 0;			/* Initial height of image */
static int wp0 = 0;			/* Initial width of image */
static double ra0 = -99.0;		/* Initial center RA in degrees */
static double dec0 = -99.0;		/* Initial center Dec in degrees */
static int comsys = WCS_J2000;		/* Command line center coordinte system */
static int ptype0 = -1;			/* Projection type to fit */
static int  nctype = 28;		/* Number of possible projections */
static char ctypes[32][4];		/* 3-letter codes for projections */
static double xref0 = -99999.0;		/* Reference pixel X coordinate */
static double yref0 = -99999.0;		/* Reference pixel Y coordinate */
static double secpix2 = PSCALE;		/* Set image scale 2--override header */
static double *cd0 = NULL;		/* Set CD matrix--override header */
static double rot0 = 361.0;		/* Initial image rotation */
static char *dateobs0 = NULL;		/* Initial DATE-OBS value in FITS date format */

//END




WorldCoor* AstroUtils::GetFITSWCS (char *filename, char	*header, int verbose, double *cra, double *cdec,double	*dra, double *ddec, double *secpix, int *wp,int	*hp,int	*sysout ,double	*eqout)
{

    int naxes;
    double eq1, x, y;
    double ra1, dec1, dx, dy;
    double xmin, xmax, ymin, ymax, ra2, dec2, ra3, dec3, ra4, dec4;
    double dra0, dra1, dra2, dra3, dra4;
    struct WorldCoor *wcs;
    char rstr[64], dstr[64], cstr[16];


    /* Initialize WCS structure from possibly revised FITS header */

    wcs = ChangeFITSWCS (filename, header, verbose);
    if (wcs == NULL) {
        return (NULL);
    }
    *hp = (int) wcs->nypix;
    *wp = (int) wcs->nxpix;

    /* If incomplete WCS in header, drop out */
    if (nowcs (wcs)) {
        setwcsfile (filename);
        /* wcserr(); */
        if (verbose)
            fprintf (stderr,"Insufficient information for initial WCS\n");
        return (NULL);
    }

    /* If in linear coordinates, do not print as sexigesimal */
    if (wcs->sysout < 1 || wcs->sysout == 6 || wcs->sysout == 10)
        wcs->degout = 1;

    /* Set flag to get appropriate equinox for catalog search */
    if (!*sysout)
        *sysout = wcs->syswcs;
    if (*eqout == 0.0)
        *eqout = wcs->equinox;
    eq1 = wcs->equinox;
    if (wcs->coorflip) {
        ra1 = wcs->crval[1];
        dec1 = wcs->crval[0];
    }
    else {
        ra1 = wcs->crval[0];
        dec1 = wcs->crval[1];
    }

    /* Print reference pixel position and value */
    if (verbose && (eq1 != *eqout || wcs->syswcs != *sysout)) {
        if (wcs->degout) {
            deg2str (rstr, 32, ra1, 6);
            deg2str (dstr, 32, dec1, 6);
        }
        else {
            ra2str (rstr, 32, ra1, 3);
            dec2str (dstr, 32, dec1, 2);
        }
        wcscstr (cstr, wcs->syswcs, wcs->equinox, wcs->epoch);
        fprintf (stderr,"Reference pixel (%.2f,%.2f) %s %s %s\n",
                 wcs->xrefpix, wcs->yrefpix, rstr, dstr, cstr);
    }

    /* Get coordinates of corners for size for catalog searching */
    dx = wcs->nxpix;
    dy = wcs->nypix;
    xmin = 0.5;
    ymin = 0.5;
    xmax = 0.5 + dx;
    ymax = 0.5 + dy;
    pix2wcs (wcs, xmin, ymin, &ra1, &dec1);
    pix2wcs (wcs, xmin, ymax, &ra2, &dec2);
    pix2wcs (wcs, xmax, ymin, &ra3, &dec3);
    pix2wcs (wcs, xmax, ymax, &ra4, &dec4);

    /* Convert search corners to output coordinate system and equinox */
    if (wcs->syswcs > 0 && wcs->syswcs != 6 && wcs->syswcs != 10) {
        wcscon (wcs->syswcs,*sysout,wcs->equinox,*eqout,&ra1,&dec1,wcs->epoch);
        wcscon (wcs->syswcs,*sysout,wcs->equinox,*eqout,&ra2,&dec2,wcs->epoch);
        wcscon (wcs->syswcs,*sysout,wcs->equinox,*eqout,&ra3,&dec3,wcs->epoch);
        wcscon (wcs->syswcs,*sysout,wcs->equinox,*eqout,&ra4,&dec4,wcs->epoch);
    }

    /* Find center and convert to output coordinate system and equinox */
    x = 0.5 + (dx * 0.5);
    y = 0.5 + (dy * 0.5);
    pix2wcs (wcs, x, y, cra, cdec);
    if (wcs->syswcs > 0 && wcs->syswcs != 6 && wcs->syswcs != 10)
        wcscon (wcs->syswcs,*sysout,wcs->equinox,*eqout,cra,cdec,wcs->epoch);

    /* Find maximum half-width in declination */
    *ddec = fabs (dec1 - *cdec);
    if (fabs (dec2 - *cdec) > *ddec)
        *ddec = fabs (dec2 - *cdec);
    if (fabs (dec3 - *cdec) > *ddec)
        *ddec = fabs (dec3 - *cdec);
    if (fabs (dec4 - *cdec) > *ddec)
        *ddec = fabs (dec4 - *cdec);

    /* Find maximum half-width in right ascension */
    dra0 = (dx / dy) * (*ddec / cos (*cdec));
    dra1 = ra1 - *cra;
    dra2 = ra2 - *cra;
    if (*cra < 0 && *cra + dra0 > 0.0) {
        dra1 = -(dra1 - 360.0);
        dra2 = -(dra2 - 360.0);
    }
    if (dra1 > 180.0)
        dra1 = dra1 - 360.0;
    else if (dra1 < -180.0)
        dra1 = dra1 + 360.0;
    else if (dra1 < 0.0)
        dra1 = -dra1;
    if (dra2 > 180.0)
        dra2 = dra2 - 360.0;
    else if (dra2 < -180.0)
        dra2 = dra2 + 360.0;
    else if (dra2 < 0.0)
        dra2 = -dra2;
    dra3 = *cra - ra3;
    dra4 = *cra - ra4;
    if (*cra > 0 && *cra - dra0 < 0.0) {
        dra3 = dra3 + 360.0;
        dra4 = dra4 + 360.0;
    }
    if (dra3 > 180.0)
        dra3 = dra3 - 360.0;
    else if (dra3 < -180.0)
        dra3 = dra3 + 360.0;
    else if (dra3 < 0.0)
        dra3 = -dra3;
    if (dra4 > 180.0)
        dra4 = dra4 - 360.0;
    else if (dra4 < -180.0)
        dra4 = dra4 + 360.0;
    else if (dra4 < 0.0)
        dra4 = -dra4;
    *dra = dra1;
    if (dra2 > *dra)
        *dra = dra2;
    if (dra3 > *dra)
        *dra = dra3;
    if (dra4 > *dra)
        *dra = dra4;

    /* wcssize (wcs, cra, cdec, dra, ddec); */

    /* Set reference pixel to center of image if it has not been set */
    if (wcs->xref == -999.0 && wcs->yref == -999.0) {
        wcs->xref = *cra;
        wcs->cel.ref[0] = *cra;
        wcs->crval[0] = *cra;
        wcs->yref = *cdec;
        wcs->cel.ref[1] = *cdec;
        wcs->crval[1] = *cdec;
        ra1 = *cra;
        dec1 = *cdec;
        if (wcs->xrefpix == 0.0 && wcs->yrefpix == 0.0) {
            wcs->xrefpix = 0.5 + (double) wcs->nxpix * 0.5;
            wcs->yrefpix = 0.5 + (double) wcs->nypix * 0.5;
        }
        wcs->xinc = *dra * 2.0 / (double) wcs->nxpix;
        wcs->yinc = *ddec * 2.0 / (double) wcs->nypix;
        /* hchange (header,"PLTRAH","PLT0RAH");
    wcs->plate_fit = 0; */
    }

    /* Convert center to desired coordinate system */
    else if (wcs->syswcs != *sysout && wcs->equinox != *eqout) {
        wcscon (wcs->syswcs, *sysout, wcs->equinox, *eqout, &ra1, &dec1, wcs->epoch);
        if (wcs->coorflip) {
            wcs->yref = ra1;
            wcs->xref = dec1;
        }
        else {
            wcs->xref = ra1;
            wcs->yref = dec1;
        }
    }

    /* Compute plate scale to return if it was not set on the command line */
    if (secpix0 <= 0.0) {
        pix2wcs (wcs, wcs->xrefpix-0.5, wcs->yrefpix, &ra1, &dec1);
        pix2wcs (wcs, wcs->xrefpix+0.5, wcs->yrefpix, &ra2, &dec2);
        *secpix = 3600.0 * wcsdist (ra1, dec1, ra2, dec2);
    }

    wcs->crval[0] = wcs->xref;
    wcs->crval[1] = wcs->yref;
    if (wcs->coorflip) {
        wcs->cel.ref[0] = wcs->crval[1];
        wcs->cel.ref[1] = wcs->crval[0];
    }
    else {
        wcs->cel.ref[0] = wcs->crval[0];
        wcs->cel.ref[1] = wcs->crval[1];
    }

    if (wcs->syswcs > 0 && wcs->syswcs != 6 && wcs->syswcs != 10) {
        wcs->cel.flag = 0;
        wcs->wcsl.flag = 0;
    }
    else {
        wcs->lin.flag = LINSET;
        wcs->wcsl.flag = WCSSET;
    }

    wcs->equinox = *eqout;
    wcs->syswcs = *sysout;
    wcs->sysout = *sysout;
    wcs->eqout = *eqout;
    wcs->sysin = *sysout;
    wcs->eqin = *eqout;
    wcscstr (cstr,*sysout,*eqout,wcs->epoch);
    strcpy (wcs->radecsys, cstr);
    strcpy (wcs->radecout, cstr);
    strcpy (wcs->radecin, cstr);
    wcsininit (wcs, wcs->radecsys);
    wcsoutinit (wcs, wcs->radecsys);

    naxes = wcs->naxis;
    if (naxes < 1 || naxes > 9) {
        naxes = wcs->naxes;
        wcs->naxis = naxes;
    }

    if (usecdelt) {
        hputnr8 (header, "CDELT1", 9, wcs->xinc);
        if (naxes > 1) {
            hputnr8 (header, "CDELT2", 9, wcs->yinc);
            hputnr8 (header, "CROTA2", 9, wcs->rot);
        }
        hdel (header, "CD1_1");
        hdel (header, "CD1_2");
        hdel (header, "CD2_1");
        hdel (header, "CD2_2");
    }
    else {
        hputnr8 (header, "CD1_1", 9, wcs->cd[0]);
        if (naxes > 1) {
            hputnr8 (header, "CD1_2", 9, wcs->cd[1]);
            hputnr8 (header, "CD2_1", 9, wcs->cd[2]);
            hputnr8 (header, "CD2_2", 9, wcs->cd[3]);
        }
    }

    /* Print reference pixel position and value */
    if (verbose) {
        if (wcs->degout) {
            deg2str (rstr, 32, ra1, 6);
            deg2str (dstr, 32, dec1, 6);
        }
        else {
            ra2str (rstr, 32, ra1, 3);
            dec2str (dstr, 32, dec1, 2);
        }
        wcscstr (cstr,*sysout,*eqout,wcs->epoch);
        fprintf (stderr,"Reference pixel (%.2f,%.2f) %s %s %s\n",
                 wcs->xrefpix, wcs->yrefpix, rstr, dstr, cstr);
    }

    /* Image size for catalog search */
    if (verbose) {
        if (wcs->degout) {
            deg2str (rstr, 32, *cra, 6);
            deg2str (dstr, 32, *cdec, 6);
        }
        else {
            ra2str (rstr, 32, *cra, 3);
            dec2str (dstr, 32, *cdec, 2);
        }
        wcscstr (cstr, *sysout, *eqout, wcs->epoch);
        fprintf (stderr,"Search at %s %s %s", rstr, dstr, cstr);
        if (wcs->degout) {
            deg2str (rstr, 32, *dra, 6);
            deg2str (dstr, 32, *ddec, 6);
        }
        else {
            ra2str (rstr, 32, *dra, 3);
            dec2str (dstr, 32, *ddec, 2);
        }
        fprintf (stderr," +- %s %s\n", rstr, dstr);
        fprintf (stderr,"Image width=%d height=%d, %g arcsec/pixel\n",
                 *wp, *hp, *secpix);
    }

    return (wcs);
}


WorldCoor* AstroUtils::ChangeFITSWCS (char *filename, char* header, int verbose)
{
    int nax, i, hp, wp;
    double xref, yref, degpix, secpix;
    struct WorldCoor *wcs;
    char temp[16];
    char *cwcs;

    /* Set the world coordinate system from the image header */



    if (strlen (filename) > 0)
    {
        cwcs = strchr (filename, '%');


        if (cwcs != NULL)
            cwcs++;
    }

    if (!strncmp (header, "END", 3)) {
        //qDebug()<<"strncmp - pre ";


        cwcs = NULL;
        for (i = 0; i < 2880; i++)
            header[i] = (char) 32;
        hputl (header, "SIMPLE", 1);
        hputi4 (header, "BITPIX", 0);
        hputi4 (header, "NAXIS", 2);
        hputi4 (header, "NAXIS1", 1);
        hputi4 (header, "NAXIS2", 1);

        // qDebug()<<"strncmp - post ";

    }

    //qDebug()<<"dopo id strncmp --  ";

    /* Set image dimensions */
    nax = 0;
    if (hp0 > 0 || wp0 > 0) {
        hp = hp0;
        wp = wp0;
        if (hp > 0 && wp > 0)
            nax = 2;
        else
            nax = 1;
        hputi4 (header, "NAXIS", nax);
        hputi4 (header, "NAXIS1", wp);
        hputi4 (header, "NAXIS2", hp);
    }
    else if (hgeti4 (header,"NAXIS",&nax) < 1 || nax < 1) {
        if (hgeti4 (header, "WCSAXES", &nax) < 1)
            return (NULL);
        else {
            if (hgeti4 (header, "IMAGEW", &wp) < 1)
                return (NULL);
            if (hgeti4 (header, "IMAGEH", &wp) < 1)
                return (NULL);
        }
    }
    else {
        if (hgeti4 (header,"NAXIS1",&wp) < 1)
            return (NULL);
        if (hgeti4 (header,"NAXIS2",&hp) < 1)
            return (NULL);
    }

    /* Set plate center from command line, if it is there */
    if (ra0 > -99.0 && dec0 > -99.0) {
        hputnr8 (header, "CRVAL1" ,8,ra0);
        hputnr8 (header, "CRVAL2" ,8,dec0);
        hputra (header, "RA", ra0);
        hputdec (header, "DEC", dec0);
        if (comsys == WCS_B1950) {
            hputi4 (header, "EPOCH", 1950);
            hputi4 (header, "EQUINOX", 1950);
            hputs (header, "RADECSYS", "FK4");
        }
        else {
            hputi4 (header, "EPOCH", 2000);
            hputi4 (header, "EQUINOX", 2000);
            if (comsys == WCS_GALACTIC)
                hputs (header, "RADECSYS", "GALACTIC");
            else if (comsys == WCS_ECLIPTIC)
                hputs (header, "RADECSYS", "ECLIPTIC");
            else if (comsys == WCS_ICRS)
                hputs (header, "RADECSYS", "ICRS");
            else
                hputs (header, "RADECSYS", "FK5");
        }
        if (hgetr8 (header, "SECPIX", &secpix)) {
            degpix = secpix / 3600.0;
            hputnr8 (header, "CDELT1", 8, -degpix);
            hputnr8 (header, "CDELT2", 8, degpix);
            hdel (header, "CD1_1");
            hdel (header, "CD1_2");
            hdel (header, "CD2_1");
            hdel (header, "CD2_2");
        }
    }
    if (ptype0 > -1 && ptype0 < nctype) {
        strcpy (temp,"RA---");
        strcat (temp, ctypes[ptype0]);
        hputs (header, "CTYPE1", temp);
        strcpy (temp,"DEC--");
        strcat (temp, ctypes[ptype0]);
        hputs (header, "CTYPE2", temp);
    }

    /* Set reference pixel from command line, if it is there */
    if (xref0 > -99999.0 && yref0 > -99999.0) {
        hputr8 (header, "CRPIX1", xref0);
        hputr8 (header, "CRPIX2", yref0);
    }
    else if (hgetr8 (header, "CRPIX1", &xref) < 1) {
        xref = 0.5 + (double) wp / 2.0;
        yref = 0.5 + (double) hp / 2.0;
        hputnr8 (header, "CRPIX1", 3, xref);
        hputnr8 (header, "CRPIX2", 3, yref);
    }

    /* Set plate scale from command line, if it is there */
    if (secpix0 != 0.0 || cd0 != NULL) {
        if (secpix2 != 0.0) {
            secpix = 0.5 * (secpix0 + secpix2);
            hputnr8 (header, "SECPIX1", 5, secpix0);
            hputnr8 (header, "SECPIX2", 5, secpix2);
            degpix = -secpix0 / 3600.0;
            hputnr8 (header, "CDELT1", 8, degpix);
            degpix = secpix2 / 3600.0;
            hputnr8 (header, "CDELT2", 8, degpix);
            hdel (header, "CD1_1");
            hdel (header, "CD1_2");
            hdel (header, "CD2_1");
            hdel (header, "CD2_2");
        }
        else if (secpix0 != 0.0) {
            secpix = secpix0;
            hputnr8 (header, "SECPIX", 5, secpix);
            degpix = secpix / 3600.0;
            hputnr8 (header, "CDELT1", 8, -degpix);
            hputnr8 (header, "CDELT2", 8, degpix);
            hdel (header, "CD1_1");
            hdel (header, "CD1_2");
            hdel (header, "CD2_1");
            hdel (header, "CD2_2");
        }
        else {
            hputr8 (header, "CD1_1", cd0[0]);
            hputr8 (header, "CD1_2", cd0[1]);
            hputr8 (header, "CD2_1", cd0[2]);
            hputr8 (header, "CD2_2", cd0[3]);
            hdel (header, "CDELT1");
            hdel (header, "CDELT2");
            hdel (header, "CROTA1");
            hdel (header, "CROTA2");
        }
        if (!ksearch (header,"CRVAL1")) {
            hgetra (header, "RA", &ra0);
            hgetdec (header, "DEC", &dec0);
            hputnr8 (header, "CRVAL1", 8, ra0);
            hputnr8 (header, "CRVAL2", 8, dec0);
        }
        if (!ksearch (header,"CRPIX1")) {
            xref = (double) wp / 2.0;
            yref = (double) hp / 2.0;
            hputnr8 (header, "CRPIX1", 3, xref);
            hputnr8 (header, "CRPIX2", 3, yref);
        }
        if (!ksearch (header,"CTYPE1")) {
            if (comsys == WCS_GALACTIC) {
                hputs (header, "CTYPE1", "GLON-TAN");
                hputs (header, "CTYPE2", "GLAT-TAN");
            }
            else {
                hputs (header, "CTYPE1", "RA---TAN");
                hputs (header, "CTYPE2", "DEC--TAN");
            }
        }
    }

    /* Set rotation angle from command line, if it is there */
    if (rot0 < 361.0) {
        hputnr8 (header, "CROTA1", 5, rot0);
        hputnr8 (header, "CROTA2", 5, rot0);
    }

    /* Set observation date for epoch, if it is there */
    if (dateobs0 != NULL)
        hputs (header, "DATE-OBS", dateobs0);

    /* Initialize WCS structure from FITS header */
    wcs = wcsinitn (header, cwcs);

    /* If incomplete WCS in header, drop out */
    if (nowcs (wcs)) {
        setwcsfile (filename);
        /* wcserr(); */
        if (verbose)
            fprintf (stderr,"Insufficient information for initial WCS\n");
        return (NULL);
    }
    return (wcs);
}


WorldCoor * AstroUtils::GetWCSFITS (char *filename, int verbose)
{
    char *header;		/* FITS header */
    struct WorldCoor *wcs;	/* World coordinate system structure */
    //char *GetFITShead();
    char *cwcs;			/* Multiple wcs string (name or character) */

    /* Read the FITS or IRAF image file header */
    header = GetFITShead (filename, verbose);
    if (header == NULL)
        return (NULL);

    verbose=true;

    /* Set the world coordinate system from the image header */
    cwcs = strchr (filename, '%');
    if (cwcs != NULL)
        cwcs++;
    wcs = wcsinitn (header, cwcs);
    if (wcs == NULL) {
        setwcsfile (filename);
        if (verbose)
            wcserr ();
    }
    free (header);

    return (wcs);
}

char * AstroUtils::GetFITShead (char * filename, int verbose)
{
    char *header;		/* FITS header */
    int lhead;			/* Maximum number of bytes in FITS header */
    char *irafheader;		/* IRAF image header */
    int nbiraf, nbfits;

    /* Open IRAF image if .imh extension is present */
    if (isiraf (filename)) {
        if ((irafheader = irafrhead (filename, &nbiraf)) != NULL) {
            if ((header = iraf2fits (filename, irafheader, nbiraf, &lhead)) == NULL) {
                if (verbose)
                    fprintf (stderr, "Cannot translate IRAF header %s\n",filename);
                free (irafheader);
                irafheader = NULL;
                return (NULL);
            }
            free (irafheader);
            irafheader = NULL;
        }
        else {
            if (verbose)
                fprintf (stderr, "Cannot read IRAF header file %s\n", filename);
            return (NULL);
        }
    }
    else if (istiff (filename) || isgif (filename) || isjpeg (filename)) {
        if ((header = fitsrtail (filename, &lhead, &nbfits)) == NULL) {
            if (verbose)
                fprintf (stderr, "TIFF file %s has no appended header\n", filename);
            return (NULL);
        }
    }


    /* Open FITS file if .imh extension is not present */
    else {
        if ((header = fitsrhead (filename, &lhead, &nbfits)) == NULL) {
            if (verbose)
                /* fprintf (stderr, "Cannot read FITS file %s\n", filename); */
                fitserr ();
            return (NULL);
        }
    }

    return (header);
}
