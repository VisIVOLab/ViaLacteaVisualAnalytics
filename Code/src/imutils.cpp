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

void fits_smooth(const std::string &inFile, const double sigma, const std::string &outFile)
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

void fits_resize(const std::string &inFile, const int resizeFactor, const std::string &outFile)
{
    fitsfile *fptr;
    int status = 0;
    int naxis = 0;
    fits_open_image(&fptr, inFile.c_str(), READONLY, &status);
    fits_get_img_dim(fptr, &naxis, &status);
    fits_close_file(fptr, &status);

    if (naxis == 2) {
        imresize(inFile, resizeFactor, outFile);
    } else {
        cuberesize(inFile, resizeFactor, outFile);
    }
}

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

    const int npix = nx * ny;
    const int nvox = npix * nz;
    const int bytepix = std::abs(bitpix / 8);
    const int nbimage = nvox * bytepix;

    int kernelSize = std::ceil(6. * sigma);
    kernelSize += (kernelSize & 1) ^ 1;
    setghwidth(sigma);
    gausswt(kernelSize, kernelSize, nx);

    char *newimage = nullptr;
    if (bitpix == -32) {
        float *inPtr = reinterpret_cast<float *>(image);
        float *outPtr = reinterpret_cast<float *>(malloc(nbimage));

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
        double *outPtr = reinterpret_cast<double *>(malloc(nbimage));

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

void cuberesize(const std::string &inFile, const int resizeFactor, const std::string &outFile)
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
            std::cerr << "Cannot read FITS Image " << inFile << std::endl;
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
    double bzero = 0.;
    hgetr8(header, "BZERO", &bzero);
    double bscale = 1.;
    hgetr8(header, "BSCALE", &bscale);

    const int bytepix = std::abs(bitpix / 8);
    const int outDimX = (nx > resizeFactor) ? nx / resizeFactor : nx;
    const int outDimY = (ny > resizeFactor) ? ny / resizeFactor : ny;
    const int outImageSize = outDimX * outDimY * nz * bytepix;

    char *newhead = nullptr;
    const int mean = 1;
    if ((newhead = ShrinkFITSHeader(name, header, resizeFactor, resizeFactor, mean, bitpix))
        == NULL) {
        std::cerr << "Cannot make new image header for " << outFile << std::endl;
        free(image);
        free(header);
        return;
    }

    char *newimage = reinterpret_cast<char *>(malloc(outImageSize));
    float *outPtrFlt = reinterpret_cast<float *>(newimage);
    double *outPtrDlb = reinterpret_cast<double *>(newimage);
    char *slicePtr = image;
    for (int k = 0; k < nz; ++k) {
        for (int j = 0; j < outDimY; ++j) {
            for (int i = 0; i < outDimX; ++i) {
                double pixij = 0.;
                double dnp = 0.;

                int jIn = j * resizeFactor;
                const int jBound = (jIn + resizeFactor > ny) ? ny - jIn + 1 : resizeFactor;

                for (int iy = 0; iy < jBound; ++iy) {
                    int iIn = i * resizeFactor;
                    const int iBound = (iIn + resizeFactor > nx) ? nx - iIn + 1 : resizeFactor;

                    for (int ii = 0; ii < iBound; ++ii) {
                        const double pixval =
                                getpix(slicePtr, bitpix, nx, ny, bzero, bscale, iIn++, jIn);
                        pixij += pixval;
                        ++dnp;
                    }
                    ++jIn;
                }

                if (bitpix == -32) {
                    *outPtrFlt++ = static_cast<float>(pixij / dnp);
                }
                if (bitpix == -64) {
                    *outPtrDlb++ = pixij / dnp;
                }
            }
        }

        slicePtr += nx * ny * bytepix;
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
