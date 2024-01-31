#ifndef IMUTILS_H
#define IMUTILS_H

#include <string>

#ifdef __cplusplus
extern "C" {
#endif

char *FiltFITS(char *header, char *image, int filter, int xsize, int ysize, int nlog);
void setghwidth(double ghwidth);
char *ShrinkFITSHeader(char *name, char *header, int xfactor, int yfactor, int mean, int bitpix);
char *ShrinkFITSImage(char *header, char *image, int xfactor, int yfactor, int mean, int bitpix,
                      int nlog);

#ifdef __cplusplus
}
#endif

void imsmooth(const std::string &inFile, const double sigma, const std::string &outFile);
void imresize(const std::string &inFile, const int resizeFactor, const std::string &outFile);

#endif // IMUTILS_H
