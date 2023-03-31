#include "../simcube_projection.hpp"
#include "fits_simimg.hpp"

#include <cassert>
#include <map>
#include <string>

using namespace std;

const double PI = 3.141592653589793238463;
// const double d2r = PI / 180.0;
const double r2d = 180.0 / PI;

void simcube::collapsed_to_galactic(std::string fitsname_image, double dist_parsec,
                                    const double center_deg[2])
{
    assert(dist_parsec > 0.0);

    fits_simimg simg(fitsname_image);

    double cdelt_gal_deg[2] { r2d * simg.cdelt_vals[0] / dist_parsec,
                              r2d * simg.cdelt_vals[1] / dist_parsec };

    simg.update_header(cdelt_gal_deg, center_deg);
}
