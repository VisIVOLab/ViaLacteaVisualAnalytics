#ifndef UTILS_HPP
#define UTILS_HPP

#include "image.hpp"
#include "projection.hpp"

#include <ostream>

std::ostream &operator<<(std::ostream &out, struct projection::xy_bounds const &p);
std::ostream &operator<<(std::ostream &out, class image const &p);

#endif
