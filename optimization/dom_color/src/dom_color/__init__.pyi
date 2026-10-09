from typing import Any, Buffer, overload

class RGB:
    r: int
    g: int
    b: int

    def __init__(self, r: int, g: int, b: int) -> None: ...
    def to_pixel16(self) -> int: ...
    def to_pixel32(self) -> int: ...
    def __eq__(self, other: Any) -> bool: ...

class HueHistogram:
    alpha: int
    beta: int
    gamma: int

    def __init__(self, alpha: int = 3, beta: int = 23, gamma: int = 0) -> None: ...

class SVHistogram:
    def __init__(self) -> None: ...

def is_grey(rgb: RGB) -> bool: ...

@overload
def get_dominating_color(pixels: Buffer) -> RGB: ...
@overload
def get_dominating_color(
    pixels: Buffer, hue_histogram: HueHistogram, sv_histogram: SVHistogram
) -> RGB: ...
