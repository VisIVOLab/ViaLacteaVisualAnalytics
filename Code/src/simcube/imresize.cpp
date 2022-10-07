/*
 * Original code from
 * https://github.com/olebole/wcstools/blob/52b63bbae90ace3cc4ac0fc86ca148b440bf593d/imresize.c By
 * Jessica Mink, Harvard-Smithsonian Center for Astrophysics
 */

// -f
// resize = 1, xfactor yfactor

// -g
// filter = GAUSSIAN, xsize ysize

// -h
// ghwidth, setghwidth(ghwidth)

// -o
// overwrite = 1

#include "imresize.h"

#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../libwcs/fitsfile.h"
#include "../libwcs/wcs.h"

extern "C" {

extern char *FiltFITS(char *, char *, int, int, int, int);
extern void setghwidth(double);
extern char *ShrinkFITSHeader(char *, char *, int, int, int, int);
extern char *ShrinkFITSImage(char *, char *, int, int, int, int, int);

void imresize(char *name, int resizeFactor, double sigma, int kernelSize)
{
    char *image; /* FITS image */
    char *header; /* FITS header */
    int lhead; /* Maximum number of bytes in FITS header */
    int nbhead; /* Actual number of bytes in FITS header */
    char newname[256]; /* Name for revised image */
    char *newimage = NULL;
    char *newhead;
    char history[64];
    int mean = 0;
    int bitpix = 0;
    int nlog = 100;

    // int resize = 1;
    int xfactor = resizeFactor;
    int yfactor = resizeFactor;

    int filter = 3; /* MEDIAN=1, MEAN=2, GAUSSIAN=3 */
    int xsize = kernelSize;
    int ysize = kernelSize;

    // int overwrite = 1;
    strcpy(newname, name);

    setghwidth(sigma);

    if ((header = fitsrhead(name, &lhead, &nbhead)) != NULL) {
        if ((image = fitsrimage(name, nbhead, header)) == NULL) {
            fprintf(stderr, "Cannot read FITS image %s\n", name);
            free(header);
            return;
        }
    } else {
        fprintf(stderr, "Cannot read FITS file %s\n", name);
        return;
    }

    if ((newimage = FiltFITS(header, image, filter, xsize, ysize, nlog)) == NULL)
        fprintf(stderr, "Cannot filter image %s; file is unchanged.\n", name);
    else {
        free(image);
        image = newimage;

        /* Add IMSMOOTH keyword to say how image was changed */
        if (hgets(header, "IMSMOOTH", 63, history))
            hputs(header, "HISTORY", history);
        sprintf(history, "Gaussian halfwidth %.2f pixels filtered over %d x %d pixels", sigma,
                xsize, ysize);
        hputs(header, "IMSMOOTH", history);
    }

    if ((newhead = ShrinkFITSHeader(name, header, xfactor, yfactor, mean, bitpix)) == NULL)
        fprintf(stderr, "Cannot make new image header for %s; file is unchanged.\n", newname);
    else if ((newimage = ShrinkFITSImage(header, image, xfactor, yfactor, mean, bitpix, nlog))
             == NULL) {
        fprintf(stderr, "Cannot shrink image %s.\n", name);
        free(newhead);
    } else {
        free(image);
        image = newimage;
        free(header);
        header = newhead;
        /* Add IMRESIZE keyword to say how image was changed */
        if (hgets(header, "IMRESIZE", 63, history))
            hputs(header, "HISTORY", history);
        sprintf(history, "Image size reduced by %d in x, %d in y", xfactor, yfactor);
        hputs(header, "IMRESIZE", history);
    }

    fitswimage(newname, header, image);
    free(header);
    free(image);
}
}
