#ifndef PROJECTION_HPP
#define PROJECTION_HPP

#include <string>
#include "image.hpp"

namespace projection
{
	void rotation_matrix_with_scaling(double srotm[2][3], const double rotation_angle_deg[3], const double cdelt[3]);


	struct xy_bounds
	{
		double x_min;
		double x_max;
		double y_min;
		double y_max;
	};

	struct xy_bounds bounds(const double rotmx[2][3],
			const long naxis[3],
			const double rot_center_pixels[3]);


	struct point2d {double x; double y;};

	struct point2d rotate(const double rotm[2][3], const double pixel[3], const double center[3]);
}

#endif

