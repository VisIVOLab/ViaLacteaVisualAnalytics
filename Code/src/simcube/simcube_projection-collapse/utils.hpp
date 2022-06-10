#ifndef UTILS_HPP
#define UTILS_HPP

#include <ostream>
#include "projection.hpp"
#include "image.hpp"

std::ostream& operator<<(std::ostream &out, struct projection::xy_bounds const& p);
std::ostream& operator<<(std::ostream &out, class image const& p);

#endif
