import array

import pytest

import dom_color


def pack_rgb565(r: int, g: int, b: int) -> int:
    """Pack 8-bit channels (multiples of 8/4/8) into an RGB565 pixel."""
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b >> 3) & 0x1F)


RED = pack_rgb565(248, 0, 0)
GREEN = pack_rgb565(0, 252, 0)
BLUE = pack_rgb565(0, 0, 248)
GREY = pack_rgb565(128, 128, 128)


def pixel_buffer(*pixels: int) -> array.array:
    return array.array("H", pixels)


class TestRGB:
    def test_construct_and_read_fields(self):
        c = dom_color.RGB(10, 20, 30)
        assert (c.r, c.g, c.b) == (10, 20, 30)

    def test_equality(self):
        assert dom_color.RGB(1, 2, 3) == dom_color.RGB(1, 2, 3)
        assert dom_color.RGB(1, 2, 3) != dom_color.RGB(1, 2, 4)

    def test_to_pixel16_and_pixel32(self):
        c = dom_color.RGB(248, 0, 0)
        assert c.to_pixel16() == RED
        assert c.to_pixel32() == 0xF80000


class TestGetDominatingColor:
    def test_solid_red(self):
        buf = pixel_buffer(*([RED] * 100))
        assert dom_color.get_dominating_color(buf) == dom_color.RGB(248, 0, 0)

    def test_solid_green(self):
        buf = pixel_buffer(*([GREEN] * 100))
        assert dom_color.get_dominating_color(buf) == dom_color.RGB(0, 252, 0)

    def test_solid_blue(self):
        buf = pixel_buffer(*([BLUE] * 100))
        assert dom_color.get_dominating_color(buf) == dom_color.RGB(0, 0, 248)

    def test_solid_grey(self):
        buf = pixel_buffer(*([GREY] * 100))
        assert dom_color.get_dominating_color(buf) == dom_color.RGB(128, 128, 128)

    def test_majority_color_wins(self):
        buf = pixel_buffer(*([RED] * 90 + [BLUE] * 10))
        assert dom_color.get_dominating_color(buf) == dom_color.RGB(248, 0, 0)

    def test_numpy_buffer(self):
        np = pytest.importorskip("numpy")
        arr = np.full(64, GREEN, dtype=np.uint16)
        assert dom_color.get_dominating_color(arr) == dom_color.RGB(0, 252, 0)

    def test_rejects_wrong_dtype(self):
        buf = array.array("I", [0xF800] * 10)  # uint32, not uint16
        with pytest.raises(RuntimeError):
            dom_color.get_dominating_color(buf)

    def test_rejects_non_1d_buffer(self):
        flat = memoryview(array.array("H", [RED] * 10))
        buf = flat.cast("B").cast("H", shape=(2, 5))
        with pytest.raises(RuntimeError):
            dom_color.get_dominating_color(buf)


class TestReusedHistograms:
    def test_defaults(self):
        hue = dom_color.HueHistogram()
        assert (hue.alpha, hue.beta) == (3, 23)

    def test_alpha_beta_settable(self):
        hue = dom_color.HueHistogram(alpha=5, beta=40)
        assert (hue.alpha, hue.beta) == (5, 40)
        hue.alpha, hue.beta = 7, 50
        assert (hue.alpha, hue.beta) == (7, 50)

    def test_matches_default_call(self):
        buf = pixel_buffer(*([RED] * 90 + [BLUE] * 10))
        hue, sv = dom_color.HueHistogram(), dom_color.SVHistogram()
        assert dom_color.get_dominating_color(
            buf, hue, sv
        ) == dom_color.get_dominating_color(buf)

    def test_no_state_leaks_between_calls(self):
        hue, sv = dom_color.HueHistogram(), dom_color.SVHistogram()
        red = pixel_buffer(*([RED] * 100))
        blue = pixel_buffer(*([BLUE] * 100))
        grey = pixel_buffer(*([GREY] * 100))
        assert dom_color.get_dominating_color(red, hue, sv) == dom_color.RGB(248, 0, 0)
        assert dom_color.get_dominating_color(blue, hue, sv) == dom_color.RGB(0, 0, 248)
        assert dom_color.get_dominating_color(grey, hue, sv) == dom_color.RGB(128, 128, 128)
        assert dom_color.get_dominating_color(red, hue, sv) == dom_color.RGB(248, 0, 0)

    def test_alpha_controls_grey_shortcut(self):
        # Hue count is weighted (~255 per saturated pixel), so grey wins only
        # when alpha * 60 > ~255 * 40.
        buf = pixel_buffer(*([GREY] * 60 + [RED] * 40))
        sv = dom_color.SVHistogram()
        grey = dom_color.get_dominating_color(buf, dom_color.HueHistogram(alpha=255), sv)
        red = dom_color.get_dominating_color(buf, dom_color.HueHistogram(alpha=0), sv)
        assert grey == dom_color.RGB(128, 128, 128)
        assert red == dom_color.RGB(248, 0, 0)
