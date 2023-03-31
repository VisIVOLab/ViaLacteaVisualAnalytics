#ifndef FITS_PROJECTION_HPP
#define FITS_PROJECTION_HPP

#include <string>

namespace simcube {

void rotate_and_collapse(std::string fitsname_image_out, std::string fitsname_cube_in,
                         const double rotation_angles_deg[3], double scale = 1.8);

void collapsed_to_galactic(std::string fitsname_image, double dist_parsec,
                           const double center_deg[2]);

} // namespace simcube

#endif
