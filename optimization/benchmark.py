"""Speed of dom_color vs Pillow's quantize-based dominant colour, per 64x64 cover."""
import statistics
import time
from pathlib import Path

import numpy as np
from PIL import Image

from dom_color import HueHistogram, SVHistogram, get_dominating_color
from main import jpg_to_rgb565

COVERS = sorted(Path("covers").glob("*.jpg"))
REPEATS = 7


def rgb_to_rgb565(img: Image.Image) -> np.ndarray:
    arr = np.asarray(img, dtype=np.uint16)
    r, g, b = arr[..., 0], arr[..., 1], arr[..., 2]
    return (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)).reshape(-1)


def pillow_dominant(img: Image.Image, colors: int, method: Image.Quantize):
    q = img.quantize(colors=colors, method=method)
    pixel_counts = q.getcolors()
    palette = q.getpalette()
    assert pixel_counts is not None and palette is not None
    _, index = max(pixel_counts)
    i = int(index)  # type: ignore[arg-type]
    return tuple(palette[3 * i: 3 * i + 3])


def bench(name, fn, items):
    runs = []
    for _ in range(REPEATS):
        start = time.perf_counter()
        for item in items:
            fn(item)
        runs.append((time.perf_counter() - start) / len(items))
    best, median = min(runs), statistics.median(runs)
    print(f"{name:<48} {best * 1e6:8.1f} us/img (best)  {median * 1e6:8.1f} us/img (median)")
    return best


def main():
    images = [Image.open(p).convert("RGB") for p in COVERS]
    pixels = [jpg_to_rgb565(p) for p in COVERS]
    hue, sv = HueHistogram(), SVHistogram()
    print(f"{len(images)} covers, {images[0].size[0]}x{images[0].size[1]}, best/median of {REPEATS} passes\n")

    results = {
        "dom_color + RGB565 conversion": bench(
            "dom_color incl. PIL->RGB565 conversion (numpy)",
            lambda im: get_dominating_color(rgb_to_rgb565(im)), images),
    }
    print()
    for k in (4, 8, 16):
        for method, label in [(Image.Quantize.MEDIANCUT, "median cut"),
                              (Image.Quantize.FASTOCTREE, "fast octree")]:
            results[f"pillow {label} k={k}"] = bench(
                f"Pillow quantize({k}, {label}) + most common",
                lambda im, k=k, m=method: pillow_dominant(im, k, m), images)
    print()

    base = results["dom_color + RGB565 conversion"]
    print("\nrelative to dom_color (default histograms):")
    for name, t in results.items():
        print(f"  {name:<34} {t / base:6.2f}x")


if __name__ == "__main__":
    main()
