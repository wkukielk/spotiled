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
