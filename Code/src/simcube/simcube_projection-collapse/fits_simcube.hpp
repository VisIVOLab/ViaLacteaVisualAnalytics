#ifndef FITS_SIMCUBE_HPP
#define FITS_SIMCUBE_HPP

#include <fitsio.h>

#include <string>

class fits_simcube
{
public:
    fits_simcube(std::string fits_filename);
    ~fits_simcube();

    long naxes[3];
    double cdelt[3];

    void read_one_row(long fpixel[3], long naxis1);
    double *pix_row;

private:
    fitsfile *fptr;
};

#endif
