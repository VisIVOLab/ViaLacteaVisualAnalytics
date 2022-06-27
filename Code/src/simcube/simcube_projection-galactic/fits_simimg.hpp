#ifndef FITS_SIMIMG_HPP
#define FITS_SIMIMG_HPP

#include <fitsio.h>

#include <string>

class fits_simimg
{
public:
    fits_simimg(std::string fits_filename);
    ~fits_simimg()
    {
        int status = 0;
        if (fptr != nullptr)
            fits_close_file(fptr, &status);
    };

    void update_card(std::string keyname, std::string keyvalue);
    void update_header(const double cdelt_deg[2], const double crval_deg[2]);

    double cdelt_vals[2];

private:
    fitsfile *fptr;
};

#endif
