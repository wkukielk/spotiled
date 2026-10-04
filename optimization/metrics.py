import numpy as np

from dom_color import RGB

# Below this CIE76 distance two colors read as "the same color".
HIT_THRESHOLD = 10.0

# sRGB (D65) -> XYZ
_RGB_TO_XYZ = np.array([
    [0.4124564, 0.3575761, 0.1804375],
    [0.2126729, 0.7151522, 0.0721750],
    [0.0193339, 0.1191920, 0.9503041],
])
_WHITE_D65 = np.array([0.95047, 1.0, 1.08883])


def rgb_to_array(colors: list[RGB]) -> np.ndarray:
    return np.array([[c.r, c.g, c.b] for c in colors], dtype=np.float64)


def srgb_to_lab(rgb: np.ndarray) -> np.ndarray:
    """(..., 3) sRGB in 0..255 -> (..., 3) CIELAB."""
    c = rgb / 255.0
    linear = np.where(c <= 0.04045, c / 12.92, ((c + 0.055) / 1.055) ** 2.4)
    xyz = linear @ _RGB_TO_XYZ.T / _WHITE_D65
    eps, kappa = 216 / 24389, 24389 / 27
    f = np.where(xyz > eps, np.cbrt(xyz), (kappa * xyz + 16) / 116)
    l = 116 * f[..., 1] - 16
    a = 500 * (f[..., 0] - f[..., 1])
    b = 200 * (f[..., 1] - f[..., 2])
    return np.stack([l, a, b], axis=-1)


def delta_e(predicted: list[RGB], expected: list[RGB]) -> np.ndarray:
    """Per-album CIE76 distance."""
    diff = srgb_to_lab(rgb_to_array(predicted)) - srgb_to_lab(rgb_to_array(expected))
    return np.linalg.norm(diff, axis=-1)


def loss(errors: np.ndarray) -> float:
    """Mean delta E: unlike MSE, a hopeless album can't dominate."""
    return float(np.mean(errors))


def hit_rate(errors: np.ndarray, threshold: float = HIT_THRESHOLD) -> float:
    return float(np.mean(errors < threshold))
