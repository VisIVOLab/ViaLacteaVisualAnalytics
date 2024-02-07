/*
 * Original code from
 * https://github.com/olebole/wcstools/blob/52b63bbae90ace3cc4ac0fc86ca148b440bf593d/imresize.c By
 * Jessica Mink, Harvard-Smithsonian Center for Astrophysics
 */

#include "imutils.h"

#include "fitsfile.h"

#include <fitsio.h>

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>

#define MEDIAN 1
#define MEAN 2
#define GAUSSIAN 3

void imsmooth(const std::string &inFile, const double sigma, const std::string &outFile)
{
    char name[inFile.size() + 1];
    std::strncpy(name, inFile.c_str(), sizeof(name));

    char fout[outFile.size() + 1];
    std::strncpy(fout, outFile.c_str(), sizeof(fout));

    char *header;
    char *image;
    int lhead;
    int nbhead;
    if ((header = fitsrhead(name, &lhead, &nbhead)) != NULL) {
        if ((image = fitsrimage(name, nbhead, header)) == NULL) {
            std::cerr << "Cannot read FITS Image " << inFile << std::endl;
            free(header);
            return;
        }
    } else {
        std::cerr << "Cannot read FITS file " << inFile << std::endl;
        return;
    }

    int kernelSize = std::ceil(6. * sigma);
    kernelSize += (kernelSize & 1) ^ 1;
    setghwidth(sigma);
    char *newimage = nullptr;
    if ((newimage = FiltFITS(header, image, GAUSSIAN, kernelSize, kernelSize, 0)) == NULL) {
        std::cerr << "Cannot filter image " << inFile << "; file is unchanged" << std::endl;
        free(image);
        free(header);
        return;
    }
    free(image);
    image = newimage;

    char history[FLEN_VALUE];
    if (hgets(header, "IMSMOOTH", FLEN_VALUE, history)) {
        hputs(header, "HISTORY", history);
    }
    std::snprintf(history, FLEN_VALUE,
                  "Gaussian halfwidth %.2f pixels filtered over %d x %d pixels", sigma, kernelSize,
                  kernelSize);
    hputs(header, "IMSMOOTH", history);

    fitswimage(fout, header, image);
    free(image);
    free(header);
}

void imresize(const std::string &inFile, const int resizeFactor, const std::string &outFile)
{
    char name[inFile.size() + 1];
    std::strncpy(name, inFile.c_str(), sizeof(name));

    char fout[outFile.size() + 1];
    std::strncpy(fout, outFile.c_str(), sizeof(fout));

    char *header;
    char *image;
    int lhead;
    int nbhead;
    if ((header = fitsrhead(name, &lhead, &nbhead)) != NULL) {
        if ((image = fitsrimage(name, nbhead, header)) == NULL) {
            std::cerr << "Cannot read FITS Image " << inFile << std::endl;
            free(header);
            return;
        }
    } else {
        std::cerr << "Cannot read FITS file " << inFile << std::endl;
        return;
    }

    char *newhead = nullptr;
    char *newimage = nullptr;
    const int mean = 1;
    const int bitpix = 0;
    if ((newhead = ShrinkFITSHeader(name, header, resizeFactor, resizeFactor, mean, bitpix))
        == NULL) {
        std::cerr << "Cannot make new image header for " << outFile << std::endl;
        free(image);
        free(header);
        return;
    }
    if ((newimage = ShrinkFITSImage(header, image, resizeFactor, resizeFactor, mean, bitpix, 0))
        == NULL) {
        std::cerr << "Cannot shrink image " << inFile << std::endl;
        free(image);
        free(header);
        return;
    }

    free(header);
    header = newhead;
    free(image);
    image = newimage;

    char history[FLEN_VALUE];
    if (hgets(header, "IMRESIZE", FLEN_VALUE, history)) {
        hputs(header, "HISTORY", history);
    }
    std::snprintf(history, FLEN_VALUE, "Image size reduced by %d", resizeFactor);
    hputs(header, "IMRESIZE", history);

    fitswimage(fout, header, image);
    free(image);
    free(header);
}
