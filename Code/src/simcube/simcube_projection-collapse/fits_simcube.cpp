
#include <stdexcept>
#include <string>

#include "fits_simcube.hpp"

using namespace std;

// utils

void assert_hdutype_is_image(fitsfile * fptr)
{
	int hdutype, status=0;

	if (fits_get_hdu_type(fptr, &hdutype, &status) || hdutype != IMAGE_HDU)
	{
		throw std::invalid_argument("FITS-file must contain IMAGE_HDU");
	}
}

void assert_cunit3_is_homogeneous(fitsfile * fptr)
{
	char *cunit[3];
	int status=0,ii,nfound;

	for (ii = 0; ii < 3; ii++) cunit[ii] = (char *) malloc(FLEN_VALUE);

	if( fits_read_keys_str(fptr, "CUNIT", 1, 3, cunit, &nfound, &status) )
	{
		for (ii = 0; ii < 3; ii++) free( cunit[ii] );
		char err_text[32]={0}; // CFITSIO doc: max 30 char error text
		fits_get_errstatus(status, err_text);
		throw std::invalid_argument(string("Error: reading CUNT 1 2 3 : ") + string(err_text));
	}

	if( (nfound != 3) || strcmp(cunit[0],cunit[1]) || strcmp(cunit[1],cunit[2]) )
	{
		for (ii = 0; ii < 3; ii++) free( cunit[ii] );
		throw invalid_argument("CUNIT key(s) missing or have not equal value");
	}

	for (ii = 0; ii < 3; ii++) free( cunit[ii] );
}

void read_naxes3(fitsfile * fptr, long naxes[3])
{
	int naxis, status = 0;

	fits_get_img_dim(fptr, &naxis, &status);
	fits_get_img_size(fptr, naxis, naxes, &status);
	if (status || naxis != 3)
	{
		char err_text[128]={0};
		sprintf(err_text,"Error: NAXIS = %d.  Only 3D cubes eligible\n", naxis);
		throw std::invalid_argument(err_text);
	}
}

void read_cdelt3(fitsfile * fptr, double cdelt[3])
{
	int status=0,nfound;

	if( fits_read_keys_dbl(fptr, "CDELT", 1, 3, cdelt, &nfound, &status) )
	{
		char err_text[32]={0}; // FITS doc: max 30 char error text
		fits_get_errstatus(status, err_text);
		throw std::invalid_argument(string("Error: reading CDELT 1 2 3 : ") + string(err_text));
	}
	if(nfound != 3) throw invalid_argument("CDELT key(s) missing");
}


// class

fits_simcube::fits_simcube(std::string fits_filename)
	:fptr{nullptr}
	,pix_row{nullptr}
{
	int status = 0;

	if ( fits_open_image(&fptr, fits_filename.c_str(), READONLY, &status) )
	{
		char err_text[32]; // CFITSIO doc: max 30 char error text
		fits_get_errstatus(status, err_text);
		throw std::runtime_error(err_text + (" : " + fits_filename));
	}

	assert_hdutype_is_image(fptr);

	read_naxes3(fptr, naxes);

	assert_cunit3_is_homogeneous(fptr);

	read_cdelt3(fptr, cdelt);

	pix_row = (double *) malloc(naxes[0] * sizeof(double)); // memory for 1 row
	if (pix_row == NULL)
	{
		throw bad_alloc();
	}
}

fits_simcube::~fits_simcube()
{
	if(pix_row!=nullptr)
		free(pix_row);

	int status =0;
	if(fptr != nullptr)
		fits_close_file(fptr, &status);
}

void fits_simcube::read_one_row(long fpixel[3], long naxis1)
{
	double nullval = 0.0;
	int status = 0;

	if (fits_read_pix(fptr, TDOUBLE, fpixel, naxis1, (void*)&nullval, pix_row,0, &status))
	{
		char err_text[32]; // FITS doc: max 30 char error text
		fits_get_errstatus(status, err_text);
		throw std::runtime_error(err_text);
	}
}


