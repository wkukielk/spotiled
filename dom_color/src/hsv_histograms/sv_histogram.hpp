#ifndef SV_HISTOGRAM_HPP
#define SV_HISTOGRAM_HPP

#include "analyzers2D/basic.hpp"
#include "analyzers2D/non_linear.hpp"
#include "histogram2D.hpp"
#include "hsv.hpp"
#include "sumators2D/non_linear.hpp"
#include "sv_histogram_aux.hpp"
#include <cstdint>

class SVHistogram {
  public:
    SVHistogram();
    sv_hist::Index add_pixel(const HSV &);
    void add_bpp32(const uint32_t *bitmap, size_t length);
    sv_hist::Dominant get_dominant() const;
    size_t get_count() const { return histogram.get_count(); }
    void reset() { histogram.reset(); };

  private:
    static const uint8_t weights[hist2D::sz][hist2D::sz];
    hist2D::Histogram histogram;
    hist2D::sumators::NonLinear sumator;
    hist2D::analyzers::NonLinear analyzer;
};

#endif
