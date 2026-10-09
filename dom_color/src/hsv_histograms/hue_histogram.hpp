#ifndef HUE_HISTOGRAM_HPP
#define HUE_HISTOGRAM_HPP

#include "analyzers/analyzer.hpp"
#include "histogram.hpp"
#include "hsv.hpp"
#include "sumators/non_linear.hpp"
#include "wraparound_it.hpp"
#include <array>
#include <cstddef>
#include <memory>

namespace hue_hist {
struct Dominant {
    hist::Bounds bounds;
    uint16_t hue;
    uint32_t count;
    bool is_grey;
    static Dominant grey;
};

Dominant create_grey(uint32_t grey_count);
Dominant convert_dominant(const hist::Dominant &dominant);
}; // namespace hue_hist

template <typename T>
concept I_HueHistogram = requires(T t, HSV hsv, uint8_t alpha) {
    { t.get_dominant() } -> std::same_as<hue_hist::Dominant>;
    { t.add_pixel(hsv) };
    { t.set_alpha(alpha) };
    { t.reset() };
    { t.get_count() } -> std::same_as<uint32_t>;
};

class HueHistogram {
  public:
    HueHistogram();
    // analyzer refers to histogram, so the object must never be relocated.
    HueHistogram(const HueHistogram &) = delete;
    HueHistogram &operator=(const HueHistogram &) = delete;
    uint32_t get_count() const { return histogram.get_count(); };
    hue_hist::Dominant get_dominant() const;
    void add_pixel(const HSV &);
    void set_alpha(uint8_t alpha);
    void reset();

  private:
    hist::Histogram histogram;
    hist::analyzers::Analyzer<WrappingSearchAlgorithm> analyzer;
};

template <I_HueHistogram H> class GreyFilterHueHistogram {
  public:
    GreyFilterHueHistogram(uint8_t alpha = 3, uint8_t beta = 23)
        requires std::default_initializable<H>;
    // make_hue_histogram() returns H by value; it is constructed in place.
    template <std::invocable F>
        requires std::same_as<std::invoke_result_t<F>, H>
    GreyFilterHueHistogram(F &&make_hue_histogram, uint8_t alpha = 3,
                           uint8_t beta = 23);
    hue_hist::Dominant get_dominant();
    void reset();
    void add_pixel(const HSV &hsv);
    uint32_t get_count() const { return hue_histogram.get_count(); }
    uint32_t get_grey_count() const { return alpha * grey_count; }
    uint8_t get_alpha() const { return alpha; };
    void set_alpha(uint8_t alpha) { this->alpha = alpha; };
    uint8_t get_beta() const { return beta; };
    void set_beta(uint8_t beta) { this->beta = beta; };
    H &get_wrapped() { return hue_histogram; }
    const H &get_wrapped() const { return hue_histogram; }

  private:
    uint8_t calculate_beta() const;

    H hue_histogram;
    uint8_t alpha;
    uint8_t beta;
    uint16_t grey_count = 0;
};

template <I_HueHistogram H> class WCAGFilterHueHistogram {
  public:
    WCAGFilterHueHistogram(uint8_t gamma = 45)
        requires std::default_initializable<H>;
    template <std::invocable F>
        requires std::same_as<std::invoke_result_t<F>, H>
    WCAGFilterHueHistogram(F &&make_hue_histogram, uint8_t gamma = 45);
    hue_hist::Dominant get_dominant() { return hue_histogram.get_dominant(); }
    void reset() { hue_histogram.reset(); }
    void add_pixel(const HSV &hsv);
    void set_alpha(uint8_t alpha) { hue_histogram.set_alpha(alpha); }
    uint32_t get_count() const { return hue_histogram.get_count(); }
    uint8_t get_gamma() const { return gamma; };
    void set_gamma(uint8_t gamma) { this->gamma = gamma; };

  private:
    H hue_histogram;
    uint8_t gamma;
};

static_assert(I_HueHistogram<HueHistogram>);
static_assert(I_HueHistogram<GreyFilterHueHistogram<HueHistogram>>);
static_assert(I_HueHistogram<WCAGFilterHueHistogram<HueHistogram>>);
static_assert(I_HueHistogram<
              GreyFilterHueHistogram<WCAGFilterHueHistogram<HueHistogram>>>);

#include "hue_histogram.tpp"

#endif
