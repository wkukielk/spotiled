import sys
from pathlib import Path

import numpy as np
from PIL import Image

from dom_color import RGB, get_dominating_color


def jpg_to_rgb565(path: Path) -> np.ndarray:
    img = Image.open(path).convert("RGB")
    arr = np.asarray(img, dtype=np.uint16)
    r, g, b = arr[..., 0], arr[..., 1], arr[..., 2]
    pixels = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
    return pixels.reshape(-1)


def main():
    if len(sys.argv) != 2:
        print(f"usage: {sys.argv[0]} <path-to-jpg>", file=sys.stderr)
        sys.exit(1)

    path = Path(sys.argv[1])
    pixels = jpg_to_rgb565(path)
    color: RGB = get_dominating_color(pixels)
    print(f"Dominant color of {path}: {color}")


if __name__ == "__main__":
    main()
