#include "image.hpp"

#include <fitsio.h>

#include <cassert>
#include <cmath> // ceil floor
#include <limits>
#include <stdexcept>

using namespace std;

long span(double size, double resolution)
{
    assert(resolution != 0.0);

    if ((size / resolution) > std::numeric_limits<long int>::max())
        throw invalid_argument("(size / resolution) is bigger then max(long int)");

    return ceil(size / resolution); // NOTE ceil: span >= size
}

image::image(const double resolution[2], const double center[2], const double size[2])
    : m_resolution { resolution[0], resolution[1] },
      m_center { center[0], center[1] }
      // CRVAL normally 0,0 because 3D rotate is around 3D-grid center
      ,
      m_Ncols { span(size[0], m_resolution[0]) },
      m_Nrows { span(size[1], m_resolution[1]) },
      m_img(m_Nrows * m_Ncols),
      m_cnt(m_Nrows * m_Ncols)

{
    assert(m_resolution[0] > 0.0);
    assert(m_resolution[1] > 0.0);

    m_span_x = m_Ncols * m_resolution[0];
    m_span_y = m_Nrows * m_resolution[1];

    m_xmin = center[0] - m_span_x / 2.0;
    m_ymin = center[1] - m_span_y / 2.0;

    // init image values

    for (long r = 0; r < m_Nrows; r++)
        for (long c = 0; c < m_Ncols; c++) {
            m_img[r + m_Nrows * c] = 0.0;
            m_cnt[r + m_Nrows * c] = 0;
        }
}

void image::add_pixel(const double xx, const double yy, const double value)
{
    unsigned int ci = floor((xx - m_xmin) / m_resolution[0]);
    unsigned int ri = floor((yy - m_ymin) / m_resolution[1]);

    assert(!(ri > m_Nrows) || !(ci > m_Ncols));

    unsigned int ix = ri + m_Nrows * ci;
    m_img[ix] += value;
    m_cnt[ix] += 1;
}

//====================================================================================================
// FITS
//====================================================================================================
string throw_on_error(int status, double *img)
{
    if (img != nullptr)
        free(img);
    char err_text[32]; // CFITSIO doc: max 30 char error text
    fits_get_errstatus(status, err_text);
    throw runtime_error(string { err_text });
}

void image::writeimage(string fits_filename)
{
    /******************************************************/
    /* Create a FITS primary array containing a 2-D image */
    /******************************************************/

    const char *filename { fits_filename.c_str() };

    fitsfile *fptr; /* pointer to the FITS file, defined in fitsio.h */
    int status, ii, jj;
    long fpixel, nelements;
    double *array[m_Nrows];

    /* initialize FITS image parameters */
    int bitpix = DOUBLE_IMG;
    long naxis = 2; /* 2-dimensional image */
    long naxes[2] = { m_Ncols, m_Nrows }; /* image is m_Ncols pixels wide by m_Nrows rows */

    /* allocate memory for the whole image */
    array[0] = (double *)malloc(naxes[0] * naxes[1] * sizeof(double));

    /* initialize pointers to the start of each row of the image */
    for (ii = 1; ii < naxes[1]; ii++)
        array[ii] = array[ii - 1] + naxes[0];

    remove(filename); /* Delete old file if it already exists */

    status = 0; /* initialize status before calling fitsio routines */

    if (fits_create_file(&fptr, filename, &status)) /* create new FITS file */
        throw_on_error(status, array[0]);

    /* write the required keywords for the primary array image.     */

    if (fits_create_img(fptr, bitpix, naxis, naxes, &status))
        throw_on_error(status, array[0]);

    if (fits_write_key_lng(fptr, "WCSAXES", 2, NULL, &status))
        throw_on_error(status, array[0]);

    if (fits_write_keys_dbl(fptr, "CDELT", 1, 2, m_resolution, -10, NULL, &status))
        throw_on_error(status, array[0]);

    double crpix_vals[2] { double(m_Ncols / 2.0), double(m_Nrows / 2.0) };
    if (fits_write_keys_dbl(fptr, "CRPIX", 1, 2, crpix_vals, -6, NULL, &status))
        throw_on_error(status, array[0]);

    double crval_vals[2] { 0.0, 0.0 };
    if (fits_write_keys_dbl(fptr, "CRVAL", 1, 2, crval_vals, -6, NULL, &status))
        throw_on_error(status, array[0]);

    char ctype1[FLEN_VALUE] { "x" };
    char ctype2[FLEN_VALUE] { "y" };
    char *ctype_vals[2] { ctype1, ctype2 };
    if (fits_write_keys_str(fptr, "CTYPE", 1, 2, ctype_vals, NULL, &status))
        throw_on_error(status, array[0]);

    char cunit1[FLEN_VALUE] { "pc" };
    char cunit2[FLEN_VALUE] { "pc" };
    char *cunit_vals[2] { cunit1, cunit2 };
    if (fits_write_keys_str(fptr, "CUNIT", 1, 2, cunit_vals, NULL, &status))
        throw_on_error(status, array[0]);

    /* initialize the values in the image */
    for (jj = 0; jj < naxes[1]; jj++) {
        for (ii = 0; ii < naxes[0]; ii++) {
            if (m_cnt[jj + m_Nrows * ii] == 0)
                array[jj][ii] = std::numeric_limits<double>::quiet_NaN();
            else
                array[jj][ii] = m_img[jj + m_Nrows * ii] / m_cnt[jj + m_Nrows * ii];

            // array[jj][ii] = m_cnt[jj+m_Nrows*ii];
            //  FIXME add write m_cnt to FITS-extension: use max(m_cnt) to decide BITPIX and BLANK
        }
    }

    fpixel = 1; /* first pixel to write      */
    nelements = naxes[0] * naxes[1]; /* number of pixels to write */

    /* write the array of doubles to the FITS file */
    if (fits_write_img(fptr, TDOUBLE, fpixel, nelements, array[0], &status))
        throw_on_error(status, array[0]);

    free(array[0]); /* free previously allocated memory */

    if (fits_close_file(fptr, &status)) /* close the file */
        throw_on_error(status, nullptr);

    return;
}
