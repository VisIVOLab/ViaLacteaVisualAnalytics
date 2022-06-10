
#include "../simcube_projection.hpp"
#include "fits_simcube.hpp"
#include "projection.hpp"
#include <cassert>
#include <cmath>
#include <fitsio.h>
#include <limits>
#include <stdexcept>
#include <string.h>

#include <iostream>
#include "utils.hpp"

using namespace std;

// to avoid undef data-points scale up by sqrt(3) (or needs interpolattion)
double max_abs_cdelt(const double cdelt[3], double scale = 1.0)
{
        double max_cdelt = numeric_limits<double>::lowest();
        for(int i=0; i<3; i++) if (abs(cdelt[i]) > max_cdelt) max_cdelt = abs(cdelt[i]);
        return max_cdelt * scale;
}


void simcube::rotate_and_collapse(std::string fitsname_image_out, std::string fitsname_cube_in, const double rotation_angles_deg[3]
		, double scale)
{
	fits_simcube f(fitsname_cube_in); 

	double srotm[2][3];
	projection::rotation_matrix_with_scaling(srotm, rotation_angles_deg, f.cdelt);

	assert(f.naxes[0] > 0);
	assert(f.naxes[1] > 0);
	assert(f.naxes[2] > 0);
	// assert or exception invali_param: fits_file not ok
	assert(f.cdelt[0] != 0.0);
	assert(f.cdelt[1] != 0.0);
	assert(f.cdelt[2] != 0.0);

	double rot_center[3] { double(f.naxes[0]+1)/2.0, double(f.naxes[1]+1)/2.0, double(f.naxes[2]+1)/2.0 };

	struct projection::xy_bounds bnds = projection::bounds(srotm, f.naxes, rot_center);

	const double span[2]{ (bnds.x_max - bnds.x_min), (bnds.y_max - bnds.y_min) };
        const double center[2]{ (bnds.x_max + bnds.x_min)/2.0, (bnds.y_max + bnds.y_min)/2.0 };

	// choose resolution of projected image: biggest pixel size of the original cube
	// use scaling factor CDELT (step at reference point) as resolution
	// CDELT has unit as defined by CUNIT (also center and span)
	const double res = max_abs_cdelt(f.cdelt, scale);
	const double resolution[2]{ res, res };

	image img(resolution, center, span);
	// img will cover area at least 'span' size or bigger by one pixel

	cout << img << endl;

	long fpixel[3];
	fpixel[0] = 1;  // read starting with first pixel in each row
	double nullval = 0.0;
	int status = 0;

	for (fpixel[2] = 1; fpixel[2] <= f.naxes[2]; fpixel[2]++)
		for (fpixel[1] = 1; fpixel[1] <= f.naxes[1]; fpixel[1]++)
		{
			f.read_one_row(fpixel, f.naxes[0]);

			for (int ii = 1; ii <= f.naxes[0]; ii++)
			{
				double vvv[3]{double(ii),double(fpixel[1]),double(fpixel[2])};
				struct projection::point2d xy_parsec{ projection::rotate(srotm, vvv, rot_center) };

				img.add_pixel(xy_parsec.x, xy_parsec.y, f.pix_row[ii]);
			}
		}

	img.writeimage(fitsname_image_out);
}
