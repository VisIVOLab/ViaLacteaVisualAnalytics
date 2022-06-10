#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <vector>
#include <string>

class image
{
	public:
		image(const double resolution[2], const double center[2], const double size[2]);
		void add_pixel(const double xx, const double yy, const double value);
		void writeimage(const std::string fits_filename);
	
		friend std::ostream& operator<<(std::ostream &out, class image const& p);

	private:
		double m_resolution[2];
		double m_center[2];
		long   m_Ncols, m_Nrows;
		double m_span_x, m_span_y;
		double m_xmin, m_ymin;
		std::vector<double> m_img;
		std::vector<double> m_cnt;
};

#endif
