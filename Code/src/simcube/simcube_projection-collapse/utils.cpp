#include "utils.hpp"

std::ostream &operator<<(std::ostream &out, struct projection::xy_bounds const &p)
{
    out << "( " << p.x_min << " .. " << p.x_max << ") "
        << "(" << p.y_min << " .. " << p.y_max << ")";
    return out;
}

std::ostream &operator<<(std::ostream &out, class image const &p)
{
    out << "image "
        << "resolution (" << p.m_resolution[0] << " x " << p.m_resolution[1] << ") "
        << "Nrows x Ncols (" << p.m_Nrows << " x " << p.m_Ncols << ") "
        << "xy-bounds "
        << "( " << p.m_xmin << " .. " << p.m_xmin + p.m_span_x << ") "
        << "(" << p.m_ymin << " .. " << p.m_ymin + p.m_span_y << ")";

    return out;
}
