#include "projection.hpp"

#include <fitsio.h>

#include <cassert>
#include <cmath> // ceil floor
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

using namespace std;

const double PI = 3.141592653589793238463;
const double d2r = PI / 180.0;
// const double r2d = 180.0/PI;

void xyz_rotation_matrix_with_scaling(double srotm[2][3], const double rotation_angle_deg[3],
                                      const double cdelt[3])
{
    // roll pitch yaw convention, instrinsic rotation: angles always relative to the rotated frame
    // airplane flying in direction positive-x axis then:
    // roll  = rotate around x
    // pitch =        around y
    // yaw   =        around z
    // RotM = yaw(A) * pitch(B) * roll(G)
    // A B G = alpha beta gamma
    const double sA = sin(d2r * rotation_angle_deg[0]);
    const double cA = cos(d2r * rotation_angle_deg[0]);
    const double sB = sin(d2r * rotation_angle_deg[1]);
    const double cB = cos(d2r * rotation_angle_deg[1]);
    const double sG = sin(d2r * rotation_angle_deg[2]);
    const double cG = cos(d2r * rotation_angle_deg[2]);

    const double l_srotm[2][3] = {
        { cdelt[0] * (cA * cB), cdelt[1] * (cA * sB * sG - sA * cG),
          cdelt[2] * (cA * sB * cG + sA * sG) },
        { cdelt[0] * (sA * cB), cdelt[1] * (sA * sB * sG + cA * cG),
          cdelt[2] * (sA * sB * cG - cA * sG) }
        //,{cdelt[0] * (-sB),      cdelt[1] * (cB*sG),             cdelt[2] * (cB*cG) }
    };
    for (int r = 0; r < 2; r++)
        for (int c = 0; c < 3; c++)
            srotm[r][c] = l_srotm[r][c];
}

// convention used in vtk-library [see Section 3.9.2]
void yxz_rotation_matrix_with_scaling(double srotm[2][3], const double rotation_angle_deg[3],
                                      const double cdelt[3])
{
    // order: YXZ: use first angle index 1, then 0, and finally index 2
    // A B G = alpha beta gamma = 1st 2nd 3rd angle applied, respectively
    const double sA = sin(d2r * rotation_angle_deg[1]);
    const double cA = cos(d2r * rotation_angle_deg[1]);
    const double sB = sin(d2r * rotation_angle_deg[0]);
    const double cB = cos(d2r * rotation_angle_deg[0]);
    const double sG = sin(d2r * rotation_angle_deg[2]);
    const double cG = cos(d2r * rotation_angle_deg[2]);

    const double l_srotm[2][3] = {
        { cdelt[0] * (cA * cG + sA * sB * sG), cdelt[1] * (cG * sA * sB - cA * sG),
          cdelt[2] * (cB * sA) },
        { cdelt[0] * (cB * sG), cdelt[1] * (cB * cG), cdelt[2] * (-sB) }
        //,{cdelt[0] * (cA*sB*sG - cG*sA),    cdelt[1] * (cA*cG*sB + sA*sG),  cdelt[2] * (cA*cB) }
    };
    for (int r = 0; r < 2; r++)
        for (int c = 0; c < 3; c++)
            srotm[r][c] = l_srotm[r][c];
}

void projection::rotation_matrix_with_scaling(double srotm[2][3],
                                              const double rotation_angle_deg[3],
                                              const double cdelt[3])
{
    yxz_rotation_matrix_with_scaling(srotm, rotation_angle_deg, cdelt);
}

//====================================================================================================
struct projection::point2d projection::rotate(const double rotm[2][3], const double pixel[3],
                                              const double center[3])
{
    double xr = pixel[0] - center[0];
    double yr = pixel[1] - center[1];
    double zr = pixel[2] - center[2];

    struct projection::point2d xy_pc
    {
        rotm[0][0] * xr + rotm[0][1] * yr + rotm[0][2] * zr,
                rotm[1][0] * xr + rotm[1][1] * yr + rotm[1][2] * zr
    };
    return xy_pc;
}

//====================================================================================================
struct projection::xy_bounds projection::bounds(const double rotmx[2][3], const long naxis[3],
                                                const double rot_center_pixels[3])
{
    struct projection::xy_bounds bnds;
    bnds.x_min = numeric_limits<double>::max();
    bnds.x_max = numeric_limits<double>::lowest();
    bnds.y_min = numeric_limits<double>::max();
    bnds.y_max = numeric_limits<double>::lowest();

    struct point3d
    {
        double x;
        double y;
        double z;
    };

    double n1 = double(naxis[0]) + 0.5;
    double n2 = double(naxis[1]) + 0.5;
    double n3 = double(naxis[2]) + 0.5;

    const vector<point3d> cube_vertices { { 0.5, 0.5, 0.5 }, { 0.5, n2, 0.5 }, { n1, n2, 0.5 },
                                          { n1, 0.5, 0.5 },  { 0.5, 0.5, n3 }, { 0.5, n2, n3 },
                                          { n1, n2, n3 },    { n1, 0.5, n3 } };

    for (struct point3d vert : cube_vertices) {
        /*		double v_x = pixel_offset(vert.x, rot_center_pixels[0]);
                        double v_y = pixel_offset(vert.y, rot_center_pixels[1]);
                        double v_z = pixel_offset(vert.z, rot_center_pixels[2]);

                        double xx = rotmx[0][0] * v_x + rotmx[0][1] * v_y + rotmx[0][2] * v_z;
                        double yy = rotmx[1][0] * v_x + rotmx[1][1] * v_y + rotmx[1][2] * v_z;
        */
        double vvv[3] { vert.x, vert.y, vert.z };
        struct projection::point2d xy_parsec
        {
            projection::rotate(rotmx, vvv, rot_center_pixels)
        };

        if (xy_parsec.x < bnds.x_min)
            bnds.x_min = xy_parsec.x; // xx;
        if (xy_parsec.x > bnds.x_max)
            bnds.x_max = xy_parsec.x; // xx;
        if (xy_parsec.y < bnds.y_min)
            bnds.y_min = xy_parsec.y; // yy;
        if (xy_parsec.y > bnds.y_max)
            bnds.y_max = xy_parsec.y; // yy;
    }
    return bnds;
}
