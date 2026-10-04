template <I_HueHistogram H>
RGB get_dominating_color(std::span<const uint16_t> pixels, H &hue_histogram,
                         SVHistogram &sv_histogram) {
    hue_histogram.reset();
    for (auto px : pixels)
        hue_histogram.add_pixel(HSV{px});
    return detail::from_dominant_hue(pixels, hue_histogram.get_dominant(),
                                     sv_histogram);
}
