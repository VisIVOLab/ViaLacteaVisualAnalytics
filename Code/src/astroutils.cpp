#include "astroutils.h"

#include "libwcs/fitsfile.h"
#include "libwcs/lwcs.h"

#include <fitsio.h>

#include <cmath>
#include <exception>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

using namespace boost::algorithm;

AstroUtils::AstroUtils() = default;

void AstroUtils::GetCenterCoords(std::string file, double *coords)
{
    WorldCoor *wc = AstroUtils().GetWCSFITS((char *)file.c_str(), 1);
    AstroUtils().xy2sky(file, wc->nxpix / 2.0, wc->nypix / 2.0, coords, WCS_GALACTIC);
    wcsfree(wc);
}

/// values = [dl, db]
void AstroUtils::GetRectSize(std::string file, double *values)
{
    WorldCoor *wc = AstroUtils().GetWCSFITS((char *)file.c_str(), 1);
    double sky_coords[2], delta[2];
    AstroUtils().xy2sky(file, 0, 0, sky_coords, WCS_GALACTIC);

    /* dl */
    AstroUtils().xy2sky(file, wc->nxpix, 0, delta, WCS_GALACTIC);
    values[0] = abs(delta[0] - sky_coords[0]);

    /* db */
    AstroUtils().xy2sky(file, 0, wc->nypix, delta, WCS_GALACTIC);
    values[1] = abs(delta[1] - sky_coords[1]);

    wcsfree(wc);
}

double AstroUtils::GetRadiusSize(std::string file)
{
    double rect[2];
    AstroUtils().GetRectSize(file, rect);
    double radius = fmax(rect[0], rect[1]) / 2.0;
    return radius;
}

void AstroUtils::GetBounds(std::string file, double *ra_min, double *ra_max, double *dec_min,
                           double *dec_max)
{
    WorldCoor *wcs = AstroUtils::GetWCSFITS(const_cast<char *>(file.c_str()), 0);
    char coorsys[80];
    wcscstr(coorsys, WCS_J2000, 0., 0.);
    wcsoutinit(wcs, coorsys);
    wcsrange(wcs, ra_min, ra_max, dec_min, dec_max);
    wcsfree(wcs);
}

bool AstroUtils::CheckOverlap(std::string f1, std::string f2, bool full)
{
    if (full) {
        // Full overlap
        return CheckFullOverlap(f1, f2);
    } else {
        // Check partial overlap
        double ra_min1, ra_max1, dec_min1, dec_max1;
        AstroUtils::GetBounds(f1, &ra_min1, &ra_max1, &dec_min1, &dec_max1);

        double ra_min2, ra_max2, dec_min2, dec_max2;
        AstroUtils::GetBounds(f2, &ra_min2, &ra_max2, &dec_min2, &dec_max2);

        return std::max(ra_min1, ra_min2) < std::min(ra_max1, ra_max2)
                && std::max(dec_min1, dec_min2) < std::min(dec_max1, dec_max2);
    }
}

int AstroUtils::calculateResizeFactor(long size, long maxSize)
{
    if (size <= 0 || maxSize <= 0 || size <= maxSize)
        return 1;
    return ceil(cbrt(1.0 * size / maxSize));
}

bool AstroUtils::checkSimCubeHeader(const std::string &file,
                                    std::list<std::string> &missingKeywords)
{
    // Set of required header's keywords to handle simcubes
    std::set<std::string> requiredKeywords = { "CTYPE", "CDELT" };

    // Clear return list
    missingKeywords.clear();

    fitsfile *fptr;
    int status = 0;
    char errmsg[FLEN_ERRMSG];
    if (fits_open_data(&fptr, file.data(), READONLY, &status)) {
        fits_get_errstatus(status, errmsg);
        fits_report_error(stderr, status);
        throw std::runtime_error(errmsg);
    }

    int nfound = 0;
    char *cards[3];
    for (int i = 0; i < 3; ++i) {
        cards[i] = new char[FLEN_CARD];
    }
    std::for_each(requiredKeywords.cbegin(), requiredKeywords.cend(),
                  [&](const std::string &keyword) {
                      if (fits_read_keys_str(fptr, keyword.data(), 1, 3, cards, &nfound, &status)) {
                          fits_report_error(stderr, status);
                      }

                      if (nfound != 3) {
                          missingKeywords.push_back(keyword);
                      }
                  });

    for (int i = 0; i < 3; ++i) {
        delete[] cards[i];
    }

    // Returns true if there are no missing keywords in the header
    return missingKeywords.empty();
}

QPair<QVector<double>, QVector<double>> AstroUtils::extractSpectrum(const char *fn, int x, int y,
                                                                    double nulval)
{
    fitsfile *fptr;
    int status = 0;
    char errtext[FLEN_ERRMSG];
    if (fits_open_data(&fptr, fn, READONLY, &status)) {
        fits_get_errstatus(status, errtext);
        throw std::runtime_error(errtext);
    }

    int nlen = 3;
    long naxes[3];
    if (fits_get_img_size(fptr, nlen, naxes, &status)) {
        fits_get_errstatus(status, errtext);
        throw std::runtime_error(errtext);
    }

    QVector<double> spectrum(naxes[2]);
    QVector<double> nanIndices;
    long fpixel[3] = { x + 1, y + 1, 0 };
    float value;
    for (long i = 1; i <= naxes[2]; ++i) {
        fpixel[2] = i;
        if (fits_read_pix(fptr, TFLOAT, fpixel, 1, 0, &value, 0, &status)) {
            fits_get_errstatus(status, errtext);
            throw std::runtime_error(errtext);
        }

        if (std::isnan(value)) {
            value = nulval;
            nanIndices.append(i - 1);
        }

        spectrum[i - 1] = value;
    }

    fits_close_file(fptr, &status);

    return qMakePair(spectrum, nanIndices);
}

bool AstroUtils::isFitsImage(const std::string &filename)
{
    fitsfile *fptr;
    int ReadStatus = 0;
    if (fits_open_data(&fptr, filename.c_str(), READONLY, &ReadStatus)) {
        fits_report_error(stderr, ReadStatus);
        return false;
    }

    int naxis = 0;
    if (fits_get_img_dim(fptr, &naxis, &ReadStatus)) {
        fits_report_error(stderr, ReadStatus);
        return false;
    }

    fits_close_file(fptr, &ReadStatus);

    return naxis == 2;
}

void AstroUtils::setMomentFITSHeader(const std::string &src, const std::string &dst, int order)
{
    char fin[FLEN_FILENAME];
    snprintf(fin, FLEN_FILENAME, "%s", src.c_str());

    char fout[FLEN_FILENAME];
    snprintf(fout, FLEN_FILENAME, "%s", dst.c_str());

    int lhead = 0;
    int nbhead = 0;
    char *header = fitsrhead(fin, &lhead, &nbhead);

    // Fix NAXIS
    hputi4(header, "NAXIS", 2);
    hdel(header, "WCSAXES");

    // Fix BUNIT
    char key[FLEN_CARD];
    hgets(header, "BUNIT", FLEN_VALUE, key);
    std::string bunit(key);
    hgets(header, "CUNIT3", FLEN_VALUE, key);
    std::string cunit3(key);
    if (!bunit.empty() && order == 0) {
        bunit += " " + cunit3;
    }
    if (order == 1 || order == 2) {
        bunit = cunit3;
    }
    // for orders 6, 8 and 10 bunit is unchanged
    if (!bunit.empty()) {
        hputs(header, "BUNIT", bunit.c_str());
    }

    // Remove NAXIS{3,4} keywords
    for (int i = 3; i <= 4; ++i) {
        std::snprintf(key, FLEN_KEYWORD, "NAXIS%d", i);
        hdel(header, key);

        std::snprintf(key, FLEN_KEYWORD, "CRPIX%d", i);
        hdel(header, key);

        std::snprintf(key, FLEN_KEYWORD, "CRVAL%d", i);
        hdel(header, key);

        std::snprintf(key, FLEN_KEYWORD, "CDELT%d", i);
        hdel(header, key);

        std::snprintf(key, FLEN_KEYWORD, "CTYPE%d", i);
        hdel(header, key);

        std::snprintf(key, FLEN_KEYWORD, "CUNIT%d", i);
        hdel(header, key);
    }

    // Add HISTORY keyword
    snprintf(key, FLEN_VALUE, "Moment %d Map", order);
    hputs(header, "HISTORY", key);

    int fd = fitswhead(fout, header);
    close(fd);
    free(header);
}

bool AstroUtils::CheckFullOverlap(std::string f1, std::string f2)
{
    double ra_min1, ra_max1, dec_min1, dec_max1;
    AstroUtils::GetBounds(f1, &ra_min1, &ra_max1, &dec_min1, &dec_max1);

    double ra_min2, ra_max2, dec_min2, dec_max2;
    AstroUtils::GetBounds(f2, &ra_min2, &ra_max2, &dec_min2, &dec_max2);

    return ra_min1 <= ra_max2 && ra_max1 >= ra_min2 && dec_max1 >= dec_min2 && dec_min1 <= dec_max2;
}

double AstroUtils::arcsecPixel(std::string file)
{
    char *fn = new char[file.length() + 1];
    strcpy(fn, file.c_str());

    struct WorldCoor *wcs;
    char *header;
    double cra, cdec, dra, ddec, secpix;
    int wp, hp;
    int sysout = 0;
    double eqout = 0.0;

    header = GetFITShead(fn, 0);
    wcs = GetFITSWCS(fn, header, 0, &cra, &cdec, &dra, &ddec, &secpix, &wp, &hp, &sysout, &eqout);
    wcsfree(wcs);

    return secpix;
}

void AstroUtils::xy2sky(std::string map, float x, float y, double *coord, int wcs_type)
{
    struct WorldCoor *wcs;
    char *fn = new char[map.length() + 1];
    static char coorsys[16];
    char wcstring[64];
    char lstr = 64;
    *coorsys = 0;

    strcpy(fn, map.c_str());
    wcs = GetWCSFITS(fn, 0);
    wcs->sysout = wcs_type;
    wcs->degout = 1; /* Output degrees instead of hh:mm:ss dd:mm:ss */
    // force the set of wcs in degree
    setwcsdeg(wcs, 1);

    if (wcs_type == WCS_GALACTIC || wcs_type == WCS_ECLIPTIC || wcs_type == WCS_J2000) {
        wcs->eqout = 2000.0;
    } else if (wcs_type == WCS_B1950) {
        wcs->eqout = 1950.0;
    }

    if (pix2wcst(wcs, x, y, wcstring, lstr)) {
        std::string str(wcstring);
        std::vector<std::string> tokens;
        trim(str);
        split(tokens, str, is_any_of(" "), boost::token_compress_on);
        coord[0] = atof(tokens[0].c_str());
        coord[1] = atof(tokens[1].c_str());
    }

    delete[] fn;
    wcsfree(wcs);
}

int AstroUtils::getSysOut(std::string file)
{
    char *fn = new char[file.length() + 1];
    strcpy(fn, file.c_str());

    struct WorldCoor *wcs;
    char *header;
    double cra, cdec, dra, ddec, secpix;
    int wp, hp;
    int sysout = 0;
    double eqout = 0.0;

    header = GetFITShead(fn, 0);
    wcs = GetFITSWCS(fn, header, 1, &cra, &cdec, &dra, &ddec, &secpix, &wp, &hp, &sysout, &eqout);
    wcsfree(wcs);

    delete[] fn;

    return sysout;
}

void AstroUtils::getRotationAngle(std::string file, double *rot, int wcs_type)
{
    char *fn = new char[file.length() + 1];
    strcpy(fn, file.c_str());

    char type[20];
    switch (wcs_type) {
    case WCS_GALACTIC:
        strcpy(type, "galactic");
        break;
    case WCS_J2000:
        strcpy(type, "fk5");
        break;
    case WCS_B1950:
        strcpy(type, "fk4");
        break;
    }

    struct WorldCoor *wcs;
    char *header;
    double cra, cdec, dra, ddec, secpix;
    int wp, hp;
    int sysout = 0;
    double eqout = 0.0;

    header = GetFITShead(fn, 0);
    wcs = GetFITSWCS(fn, header, 0, &cra, &cdec, &dra, &ddec, &secpix, &wp, &hp, &sysout, &eqout);
    wcsininit(wcs, type);
    wcsoutinit(wcs, type);

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

    wcs->xinc = fabs(wcs->xinc);
    wcs->yinc = fabs(wcs->yinc);

    /* Compute position angles of North and East in image */
    xc = wcs->xrefpix;
    yc = wcs->yrefpix;
    pix2wcs(wcs, xc, yc, &cra, &cdec);
    if (wcs->coorflip) {
        wcs2pix(wcs, cra + wcs->yinc, cdec, &xe, &ye, &off);
        wcs2pix(wcs, cra, cdec + wcs->xinc, &xn, &yn, &off);
    } else {
        wcs2pix(wcs, cra + wcs->xinc, cdec, &xe, &ye, &off);
        wcs2pix(wcs, cra, cdec + wcs->yinc, &xn, &yn, &off);
    }
    wcs->pa_north = raddeg(atan2(yn - yc, xn - xc));
    if (wcs->pa_north < -90.0)
        wcs->pa_north = wcs->pa_north + 360.0;
    wcs->pa_east = raddeg(atan2(ye - yc, xe - xc));
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
    } else
        wcs->rot = wcs->imrot;
    if (wcs->rot < 0.0)
        wcs->rot = wcs->rot + 360.0;
    if (wcs->rot >= 360.0)
        wcs->rot = wcs->rot - 360.0;
    *rot = wcs->rot;

    /* Set image mirror flag based on axis orientation */
    wcs->imflip = 0;
    if (wcs->pa_east - wcs->pa_north < -80.0 && wcs->pa_east - wcs->pa_north > -100.0)
        wcs->imflip = 1;
    if (wcs->pa_east - wcs->pa_north < 280.0 && wcs->pa_east - wcs->pa_north > 260.0)
        wcs->imflip = 1;
    if (wcs->pa_north - wcs->pa_east > 80.0 && wcs->pa_north - wcs->pa_east < 100.0)
        wcs->imflip = 1;
    if (wcs->coorflip) {
        if (wcs->imflip)
            wcs->yinc = -wcs->yinc;
    } else {
        if (!wcs->imflip)
            wcs->xinc = -wcs->xinc;
    }

    delete[] fn;
    wcsfree(wcs);
}

void AstroUtils::sky2xy_t(std::string map, double xpos, double ypos, int wcs_type, double *xpix,
                          double *ypix)
{
    char *fn = new char[map.length() + 1];
    strncpy(fn, map.c_str(), map.size());
    fn[map.size()] = 0;

    char type[20];
    switch (wcs_type) {
    case WCS_GALACTIC:
        strcpy(type, "galactic");
        break;
    case WCS_J2000:
        strcpy(type, "fk5");
        break;
    case WCS_B1950:
        strcpy(type, "fk4");
        break;
    }

    auto wcs = GetWCSFITS(fn, 0);
    wcsininit(wcs, type);
    int offset;
    wcsc2pix(wcs, xpos, ypos, type, xpix, ypix, &offset);

    wcsfree(wcs);
    delete[] fn;
}

bool AstroUtils::sky2xy(std::string map, double ra, double dec, double *coord)
{
    char *fn = new char[map.length() + 1];
    strcpy(fn, map.c_str());

    struct WorldCoor *wcs;
    char *header;
    double cra, cdec, dra, ddec, secpix;
    int wp, hp;
    int sysout = 0;
    double eqout = 0.0;
    double x, y;
    int sysin;
    char csys[16];
    double eqin = 0.0;
    int offscale;

    header = GetFITShead(fn, 0);
    wcs = GetFITSWCS(fn, header, 0, &cra, &cdec, &dra, &ddec, &secpix, &wp, &hp, &sysout, &eqout);

    if (wcs->prjcode < 0)
        strcpy(csys, "PIXEL");
    else if (wcs->prjcode < 2)
        strcpy(csys, "LINEAR");
    else
        strcpy(csys, wcs->radecsys);

    sysin = wcscsys(csys);
    eqin = wcsceq(csys);

    if (wcs->syswcs > 0 && wcs->syswcs != 6 && wcs->syswcs != 10)
        wcscon(sysin, wcs->syswcs, eqin, eqout, &ra, &dec, wcs->epoch);

    wcsc2pix(wcs, ra, dec, csys, &x, &y, &offscale);

    coord[0] = x;
    coord[1] = y;
    coord[2] = secpix;

    delete[] fn;
    delete[] header;
    wcsfree(wcs);

    return true;
}

static double secpix0 = PSCALE; /* Set image scale--override header */
static int usecdelt = 0; /* Use CDELT if 1, else CD matrix */
static int hp0 = 0; /* Initial height of image */
static int wp0 = 0; /* Initial width of image */
static double ra0 = -99.0; /* Initial center RA in degrees */
static double dec0 = -99.0; /* Initial center Dec in degrees */
static int comsys = WCS_J2000; /* Command line center coordinte system */
static int ptype0 = -1; /* Projection type to fit */
static int nctype = 28; /* Number of possible projections */
static char ctypes[32][4]; /* 3-letter codes for projections */
static double xref0 = -99999.0; /* Reference pixel X coordinate */
static double yref0 = -99999.0; /* Reference pixel Y coordinate */
static double secpix2 = PSCALE; /* Set image scale 2--override header */
static double *cd0 = NULL; /* Set CD matrix--override header */
static double rot0 = 361.0; /* Initial image rotation */
static char *dateobs0 = NULL; /* Initial DATE-OBS value in FITS date format */

WorldCoor *AstroUtils::GetFITSWCS(char *filename, char *header, int verbose, double *cra,
                                  double *cdec, double *dra, double *ddec, double *secpix, int *wp,
                                  int *hp, int *sysout, double *eqout)
{
    int naxes;
    double eq1, x, y;
    double ra1, dec1, dx, dy;
    double xmin, xmax, ymin, ymax, ra2, dec2, ra3, dec3, ra4, dec4;
    double dra0, dra1, dra2, dra3, dra4;
    struct WorldCoor *wcs;
    char rstr[64], dstr[64], cstr[16];

    /* Initialize WCS structure from possibly revised FITS header */
    wcs = ChangeFITSWCS(filename, header, verbose);
    if (wcs == NULL) {
        return (NULL);
    }
    *hp = (int)wcs->nypix;
    *wp = (int)wcs->nxpix;

    /* If incomplete WCS in header, drop out */
    if (nowcs(wcs)) {
        setwcsfile(filename);
        /* wcserr(); */
        if (verbose)
            fprintf(stderr, "Insufficient information for initial WCS\n");
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
    } else {
        ra1 = wcs->crval[0];
        dec1 = wcs->crval[1];
    }

    /* Print reference pixel position and value */
    if (verbose && (eq1 != *eqout || wcs->syswcs != *sysout)) {
        if (wcs->degout) {
            deg2str(rstr, 32, ra1, 6);
            deg2str(dstr, 32, dec1, 6);
        } else {
            ra2str(rstr, 32, ra1, 3);
            dec2str(dstr, 32, dec1, 2);
        }
        wcscstr(cstr, wcs->syswcs, wcs->equinox, wcs->epoch);
        fprintf(stderr, "Reference pixel (%.2f,%.2f) %s %s %s\n", wcs->xrefpix, wcs->yrefpix, rstr,
                dstr, cstr);
    }

    /* Get coordinates of corners for size for catalog searching */
    dx = wcs->nxpix;
    dy = wcs->nypix;
    xmin = 0.5;
    ymin = 0.5;
    xmax = 0.5 + dx;
    ymax = 0.5 + dy;
    pix2wcs(wcs, xmin, ymin, &ra1, &dec1);
    pix2wcs(wcs, xmin, ymax, &ra2, &dec2);
    pix2wcs(wcs, xmax, ymin, &ra3, &dec3);
    pix2wcs(wcs, xmax, ymax, &ra4, &dec4);

    /* Convert search corners to output coordinate system and equinox */
    if (wcs->syswcs > 0 && wcs->syswcs != 6 && wcs->syswcs != 10) {
        wcscon(wcs->syswcs, *sysout, wcs->equinox, *eqout, &ra1, &dec1, wcs->epoch);
        wcscon(wcs->syswcs, *sysout, wcs->equinox, *eqout, &ra2, &dec2, wcs->epoch);
        wcscon(wcs->syswcs, *sysout, wcs->equinox, *eqout, &ra3, &dec3, wcs->epoch);
        wcscon(wcs->syswcs, *sysout, wcs->equinox, *eqout, &ra4, &dec4, wcs->epoch);
    }

    /* Find center and convert to output coordinate system and equinox */
    x = 0.5 + (dx * 0.5);
    y = 0.5 + (dy * 0.5);
    pix2wcs(wcs, x, y, cra, cdec);
    if (wcs->syswcs > 0 && wcs->syswcs != 6 && wcs->syswcs != 10)
        wcscon(wcs->syswcs, *sysout, wcs->equinox, *eqout, cra, cdec, wcs->epoch);

    /* Find maximum half-width in declination */
    *ddec = fabs(dec1 - *cdec);
    if (fabs(dec2 - *cdec) > *ddec)
        *ddec = fabs(dec2 - *cdec);
    if (fabs(dec3 - *cdec) > *ddec)
        *ddec = fabs(dec3 - *cdec);
    if (fabs(dec4 - *cdec) > *ddec)
        *ddec = fabs(dec4 - *cdec);

    /* Find maximum half-width in right ascension */
    dra0 = (dx / dy) * (*ddec / cos(*cdec));
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
            wcs->xrefpix = 0.5 + (double)wcs->nxpix * 0.5;
            wcs->yrefpix = 0.5 + (double)wcs->nypix * 0.5;
        }
        wcs->xinc = *dra * 2.0 / (double)wcs->nxpix;
        wcs->yinc = *ddec * 2.0 / (double)wcs->nypix;
        /* hchange (header,"PLTRAH","PLT0RAH");
        wcs->plate_fit = 0; */
    }

    /* Convert center to desired coordinate system */
    else if (wcs->syswcs != *sysout && wcs->equinox != *eqout) {
        wcscon(wcs->syswcs, *sysout, wcs->equinox, *eqout, &ra1, &dec1, wcs->epoch);
        if (wcs->coorflip) {
            wcs->yref = ra1;
            wcs->xref = dec1;
        } else {
            wcs->xref = ra1;
            wcs->yref = dec1;
        }
    }

    /* Compute plate scale to return if it was not set on the command line */
    if (secpix0 <= 0.0) {
        pix2wcs(wcs, wcs->xrefpix - 0.5, wcs->yrefpix, &ra1, &dec1);
        pix2wcs(wcs, wcs->xrefpix + 0.5, wcs->yrefpix, &ra2, &dec2);
        *secpix = 3600.0 * wcsdist(ra1, dec1, ra2, dec2);
    }

    wcs->crval[0] = wcs->xref;
    wcs->crval[1] = wcs->yref;
    if (wcs->coorflip) {
        wcs->cel.ref[0] = wcs->crval[1];
        wcs->cel.ref[1] = wcs->crval[0];
    } else {
        wcs->cel.ref[0] = wcs->crval[0];
        wcs->cel.ref[1] = wcs->crval[1];
    }

    if (wcs->syswcs > 0 && wcs->syswcs != 6 && wcs->syswcs != 10) {
        wcs->cel.flag = 0;
        wcs->wcsl.flag = 0;
    } else {
        wcs->lin.flag = LINSET;
        wcs->wcsl.flag = WCSSET;
    }

    wcs->equinox = *eqout;
    wcs->syswcs = *sysout;
    wcs->sysout = *sysout;
    wcs->eqout = *eqout;
    wcs->sysin = *sysout;
    wcs->eqin = *eqout;
    wcscstr(cstr, *sysout, *eqout, wcs->epoch);
    strcpy(wcs->radecsys, cstr);
    strcpy(wcs->radecout, cstr);
    strcpy(wcs->radecin, cstr);
    wcsininit(wcs, wcs->radecsys);
    wcsoutinit(wcs, wcs->radecsys);

    naxes = wcs->naxis;
    if (naxes < 1 || naxes > 9) {
        naxes = wcs->naxes;
        wcs->naxis = naxes;
    }

    if (usecdelt) {
        hputnr8(header, "CDELT1", 9, wcs->xinc);
        if (naxes > 1) {
            hputnr8(header, "CDELT2", 9, wcs->yinc);
            hputnr8(header, "CROTA2", 9, wcs->rot);
        }
        hdel(header, "CD1_1");
        hdel(header, "CD1_2");
        hdel(header, "CD2_1");
        hdel(header, "CD2_2");
    } else {
        hputnr8(header, "CD1_1", 9, wcs->cd[0]);
        if (naxes > 1) {
            hputnr8(header, "CD1_2", 9, wcs->cd[1]);
            hputnr8(header, "CD2_1", 9, wcs->cd[2]);
            hputnr8(header, "CD2_2", 9, wcs->cd[3]);
        }
    }

    /* Print reference pixel position and value */
    if (verbose) {
        if (wcs->degout) {
            deg2str(rstr, 32, ra1, 6);
            deg2str(dstr, 32, dec1, 6);
        } else {
            ra2str(rstr, 32, ra1, 3);
            dec2str(dstr, 32, dec1, 2);
        }
        wcscstr(cstr, *sysout, *eqout, wcs->epoch);
        fprintf(stderr, "Reference pixel (%.2f,%.2f) %s %s %s\n", wcs->xrefpix, wcs->yrefpix, rstr,
                dstr, cstr);
    }

    /* Image size for catalog search */
    if (verbose) {
        if (wcs->degout) {
            deg2str(rstr, 32, *cra, 6);
            deg2str(dstr, 32, *cdec, 6);
        } else {
            ra2str(rstr, 32, *cra, 3);
            dec2str(dstr, 32, *cdec, 2);
        }
        wcscstr(cstr, *sysout, *eqout, wcs->epoch);
        fprintf(stderr, "Search at %s %s %s", rstr, dstr, cstr);
        if (wcs->degout) {
            deg2str(rstr, 32, *dra, 6);
            deg2str(dstr, 32, *ddec, 6);
        } else {
            ra2str(rstr, 32, *dra, 3);
            dec2str(dstr, 32, *ddec, 2);
        }
        fprintf(stderr, " +- %s %s\n", rstr, dstr);
        fprintf(stderr, "Image width=%d height=%d, %g arcsec/pixel\n", *wp, *hp, *secpix);
    }

    return (wcs);
}

WorldCoor *AstroUtils::ChangeFITSWCS(char *filename, char *header, int verbose)
{
    int nax, i, hp, wp;
    double xref, yref, degpix, secpix;
    struct WorldCoor *wcs;
    char temp[16];
    char *cwcs;

    /* Set the world coordinate system from the image header */
    if (strlen(filename) > 0) {
        cwcs = strchr(filename, '%');
        if (cwcs != NULL)
            cwcs++;
    }

    if (!strncmp(header, "END", 3)) {
        cwcs = NULL;
        for (i = 0; i < 2880; i++)
            header[i] = (char)32;
        hputl(header, "SIMPLE", 1);
        hputi4(header, "BITPIX", 0);
        hputi4(header, "NAXIS", 2);
        hputi4(header, "NAXIS1", 1);
        hputi4(header, "NAXIS2", 1);
    }

    /* Set image dimensions */
    nax = 0;
    if (hp0 > 0 || wp0 > 0) {
        hp = hp0;
        wp = wp0;
        if (hp > 0 && wp > 0)
            nax = 2;
        else
            nax = 1;
        hputi4(header, "NAXIS", nax);
        hputi4(header, "NAXIS1", wp);
        hputi4(header, "NAXIS2", hp);
    } else if (hgeti4(header, "NAXIS", &nax) < 1 || nax < 1) {
        if (hgeti4(header, "WCSAXES", &nax) < 1)
            return (NULL);
        else {
            if (hgeti4(header, "IMAGEW", &wp) < 1)
                return (NULL);
            if (hgeti4(header, "IMAGEH", &wp) < 1)
                return (NULL);
        }
    } else {
        if (hgeti4(header, "NAXIS1", &wp) < 1)
            return (NULL);
        if (hgeti4(header, "NAXIS2", &hp) < 1)
            return (NULL);
    }

    /* Set plate center from command line, if it is there */
    if (ra0 > -99.0 && dec0 > -99.0) {
        hputnr8(header, "CRVAL1", 8, ra0);
        hputnr8(header, "CRVAL2", 8, dec0);
        hputra(header, "RA", ra0);
        hputdec(header, "DEC", dec0);
        if (comsys == WCS_B1950) {
            hputi4(header, "EPOCH", 1950);
            hputi4(header, "EQUINOX", 1950);
            hputs(header, "RADECSYS", "FK4");
        } else {
            hputi4(header, "EPOCH", 2000);
            hputi4(header, "EQUINOX", 2000);
            if (comsys == WCS_GALACTIC)
                hputs(header, "RADECSYS", "GALACTIC");
            else if (comsys == WCS_ECLIPTIC)
                hputs(header, "RADECSYS", "ECLIPTIC");
            else if (comsys == WCS_ICRS)
                hputs(header, "RADECSYS", "ICRS");
            else
                hputs(header, "RADECSYS", "FK5");
        }
        if (hgetr8(header, "SECPIX", &secpix)) {
            degpix = secpix / 3600.0;
            hputnr8(header, "CDELT1", 8, -degpix);
            hputnr8(header, "CDELT2", 8, degpix);
            hdel(header, "CD1_1");
            hdel(header, "CD1_2");
            hdel(header, "CD2_1");
            hdel(header, "CD2_2");
        }
    }
    if (ptype0 > -1 && ptype0 < nctype) {
        strcpy(temp, "RA---");
        strcat(temp, ctypes[ptype0]);
        hputs(header, "CTYPE1", temp);
        strcpy(temp, "DEC--");
        strcat(temp, ctypes[ptype0]);
        hputs(header, "CTYPE2", temp);
    }

    /* Set reference pixel from command line, if it is there */
    if (xref0 > -99999.0 && yref0 > -99999.0) {
        hputr8(header, "CRPIX1", xref0);
        hputr8(header, "CRPIX2", yref0);
    } else if (hgetr8(header, "CRPIX1", &xref) < 1) {
        xref = 0.5 + (double)wp / 2.0;
        yref = 0.5 + (double)hp / 2.0;
        hputnr8(header, "CRPIX1", 3, xref);
        hputnr8(header, "CRPIX2", 3, yref);
    }

    /* Set plate scale from command line, if it is there */
    if (secpix0 != 0.0 || cd0 != NULL) {
        if (secpix2 != 0.0) {
            secpix = 0.5 * (secpix0 + secpix2);
            hputnr8(header, "SECPIX1", 5, secpix0);
            hputnr8(header, "SECPIX2", 5, secpix2);
            degpix = -secpix0 / 3600.0;
            hputnr8(header, "CDELT1", 8, degpix);
            degpix = secpix2 / 3600.0;
            hputnr8(header, "CDELT2", 8, degpix);
            hdel(header, "CD1_1");
            hdel(header, "CD1_2");
            hdel(header, "CD2_1");
            hdel(header, "CD2_2");
        } else if (secpix0 != 0.0) {
            secpix = secpix0;
            hputnr8(header, "SECPIX", 5, secpix);
            degpix = secpix / 3600.0;
            hputnr8(header, "CDELT1", 8, -degpix);
            hputnr8(header, "CDELT2", 8, degpix);
            hdel(header, "CD1_1");
            hdel(header, "CD1_2");
            hdel(header, "CD2_1");
            hdel(header, "CD2_2");
        } else {
            hputr8(header, "CD1_1", cd0[0]);
            hputr8(header, "CD1_2", cd0[1]);
            hputr8(header, "CD2_1", cd0[2]);
            hputr8(header, "CD2_2", cd0[3]);
            hdel(header, "CDELT1");
            hdel(header, "CDELT2");
            hdel(header, "CROTA1");
            hdel(header, "CROTA2");
        }
        if (!ksearch(header, "CRVAL1")) {
            hgetra(header, "RA", &ra0);
            hgetdec(header, "DEC", &dec0);
            hputnr8(header, "CRVAL1", 8, ra0);
            hputnr8(header, "CRVAL2", 8, dec0);
        }
        if (!ksearch(header, "CRPIX1")) {
            xref = (double)wp / 2.0;
            yref = (double)hp / 2.0;
            hputnr8(header, "CRPIX1", 3, xref);
            hputnr8(header, "CRPIX2", 3, yref);
        }
        if (!ksearch(header, "CTYPE1")) {
            if (comsys == WCS_GALACTIC) {
                hputs(header, "CTYPE1", "GLON-TAN");
                hputs(header, "CTYPE2", "GLAT-TAN");
            } else {
                hputs(header, "CTYPE1", "RA---TAN");
                hputs(header, "CTYPE2", "DEC--TAN");
            }
        }
    }

    /* Set rotation angle from command line, if it is there */
    if (rot0 < 361.0) {
        hputnr8(header, "CROTA1", 5, rot0);
        hputnr8(header, "CROTA2", 5, rot0);
    }

    /* Set observation date for epoch, if it is there */
    if (dateobs0 != NULL)
        hputs(header, "DATE-OBS", dateobs0);

    /* Initialize WCS structure from FITS header */
    wcs = wcsinitn(header, cwcs);

    /* If incomplete WCS in header, drop out */
    if (nowcs(wcs)) {
        setwcsfile(filename);
        /* wcserr(); */
        if (verbose)
            fprintf(stderr, "Insufficient information for initial WCS\n");
        return (NULL);
    }
    return (wcs);
}

WorldCoor *AstroUtils::GetWCSFITS(char *filename, int verbose)
{
    char *header; /* FITS header */
    struct WorldCoor *wcs; /* World coordinate system structure */
    char *cwcs; /* Multiple wcs string (name or character) */

    /* Read the FITS or IRAF image file header */
    header = GetFITShead(filename, verbose);
    if (header == NULL)
        return (NULL);

    verbose = true;

    /* Set the world coordinate system from the image header */
    cwcs = strchr(filename, '%');
    if (cwcs != NULL)
        cwcs++;
    wcs = wcsinitn(header, cwcs);
    if (wcs == NULL) {
        setwcsfile(filename);
        if (verbose)
            wcserr();
    }
    free(header);

    return (wcs);
}

char *AstroUtils::GetFITShead(char *filename, int verbose)
{
    char *header; /* FITS header */
    int lhead; /* Maximum number of bytes in FITS header */
    char *irafheader; /* IRAF image header */
    int nbiraf, nbfits;

    /* Open IRAF image if .imh extension is present */
    if (isiraf(filename)) {
        if ((irafheader = irafrhead(filename, &nbiraf)) != NULL) {
            if ((header = iraf2fits(filename, irafheader, nbiraf, &lhead)) == NULL) {
                if (verbose)
                    fprintf(stderr, "Cannot translate IRAF header %s\n", filename);
                free(irafheader);
                irafheader = NULL;
                return (NULL);
            }
            free(irafheader);
            irafheader = NULL;
        } else {
            if (verbose)
                fprintf(stderr, "Cannot read IRAF header file %s\n", filename);
            return (NULL);
        }
    } else if (istiff(filename) || isgif(filename) || isjpeg(filename)) {
        if ((header = fitsrtail(filename, &lhead, &nbfits)) == NULL) {
            if (verbose)
                fprintf(stderr, "TIFF file %s has no appended header\n", filename);
            return (NULL);
        }
    }

    /* Open FITS file if .imh extension is not present */
    else {
        if ((header = fitsrhead(filename, &lhead, &nbfits)) == NULL) {
            if (verbose)
                /* fprintf (stderr, "Cannot read FITS file %s\n", filename); */
                fitserr();
            return (NULL);
        }
    }

    return (header);
}
