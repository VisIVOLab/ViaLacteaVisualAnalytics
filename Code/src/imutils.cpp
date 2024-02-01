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

void cubesmooth(const std::string &inFile, const double sigma, const std::string &outFile)
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
        if ((image = fitsrfull(name, nbhead, header)) == NULL) {
            std::cerr << "Cannot read FITS Cube " << inFile << std::endl;
            free(header);
            return;
        }
    } else {
        std::cerr << "Cannot read FITS file " << inFile << std::endl;
        return;
    }

    int bitpix, nx, ny, nz;
    hgeti4(header, "BITPIX", &bitpix);
    hgeti4(header, "NAXIS1", &nx);
    hgeti4(header, "NAXIS2", &ny);
    hgeti4(header, "NAXIS3", &nz);

    int npix = nx * ny;
    int nvox = npix * nz;
    int bytepix = std::abs(bitpix / 8);
    int nbimage = nvox * bytepix;
    int nblocks = nbimage / FITSBLOCK;
    if ((nblocks * FITSBLOCK) < nbimage) {
        ++nblocks;
    }
    int nbytes = nblocks * FITSBLOCK;

    int kernelSize = std::ceil(6. * sigma);
    kernelSize += (kernelSize & 1) ^ 1;
    setghwidth(sigma);
    gausswt(kernelSize, kernelSize, nx);

    char *newimage = nullptr;
    if (bitpix == -32) {
        float *inPtr = reinterpret_cast<float *>(image);
        float *outPtr = reinterpret_cast<float *>(malloc(nbytes));

        float *slicePtr = inPtr;
        float *tmpOut = outPtr;

        for (int k = 0; k < nz; ++k) {
            float *tmpIn = slicePtr;
            for (int j = 0; j < ny; ++j) {
                for (int i = 0; i < nx; ++i) {
                    *tmpOut++ = gausspixr4(slicePtr, *tmpIn++, i, j, nx, ny);
                }
            }

            slicePtr += npix;
        }

        newimage = reinterpret_cast<char *>(outPtr);
    }
    if (bitpix == -64) {
        double *inPtr = reinterpret_cast<double *>(image);
        double *outPtr = reinterpret_cast<double *>(malloc(nbytes));

        double *slicePtr = inPtr;
        double *tmpOut = outPtr;

        for (int k = 0; k < nz; ++k) {
            double *tmpIn = slicePtr;
            for (int j = 0; j < ny; ++j) {
                for (int i = 0; i < nx; ++i) {
                    *tmpOut++ = gausspixr8(slicePtr, *tmpIn++, i, j, nx, ny);
                }
            }

            slicePtr += npix;
        }

        newimage = reinterpret_cast<char *>(outPtr);
    }

    if (!newimage) {
        std::cerr << "Cannot filter cube " << inFile << "; file is unchanged" << std::endl;
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

void smooth(const std::string &inFile, const double sigma, const std::string &outFile)
{
    fitsfile *fptr;
    int status = 0;
    int naxis = 0;
    fits_open_image(&fptr, inFile.c_str(), READONLY, &status);
    fits_get_img_dim(fptr, &naxis, &status);
    fits_close_file(fptr, &status);

    if (naxis == 2) {
        imsmooth(inFile, sigma, outFile);
    } else {
        cubesmooth(inFile, sigma, outFile);
    }
}
