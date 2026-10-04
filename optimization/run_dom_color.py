from dataclasses import dataclass

import numpy as np
from PIL import Image

import metrics
from album_store import Album, AlbumStore
from dom_color import RGB, HueHistogram, SVHistogram, get_dominating_color
from main import jpg_to_rgb565
from visualize import collage, comparison_tile


@dataclass
class LoadedAlbum:
    album: Album
    spotify_color: RGB
    pixels: np.ndarray


@dataclass
class EvaluatedAlbum:
    loaded: LoadedAlbum
    predicted: RGB
    error: float


def parse_hex_color(color: str) -> RGB:
    # "#RRGGBB" or "#RRGGBBAA"; alpha is ignored.
    return RGB(int(color[1:3], 16), int(color[3:5], 16), int(color[5:7], 16))


def load_albums(store: AlbumStore) -> list[LoadedAlbum]:
    return [
        LoadedAlbum(
            album=album,
            spotify_color=parse_hex_color(album.color),
            pixels=jpg_to_rgb565(store.image_path(album)),
        )
        for album in store.load_all()
    ]


def album_tile(store: AlbumStore, evaluated: EvaluatedAlbum) -> Image.Image:
    loaded = evaluated.loaded
    return comparison_tile(Image.open(store.image_path(loaded.album)), loaded.spotify_color,
                           evaluated.predicted,
                           caption=f"{loaded.album.image_filename}  dE={evaluated.error:.1f}")


def save_worst_collage(store: AlbumStore, evaluated: list[EvaluatedAlbum], path: str, count: int):
    worst = sorted(evaluated, key=lambda e: e.error, reverse=True)[:count]
    tiles = [album_tile(store, e) for e in worst]
    collage(tiles, columns=4).save(path)


def evaluate(albums: list[LoadedAlbum], hue_histogram: HueHistogram,
             sv_histogram: SVHistogram) -> list[EvaluatedAlbum]:
    predicted = [get_dominating_color(loaded.pixels, hue_histogram, sv_histogram)
                 for loaded in albums]
    expected = [loaded.spotify_color for loaded in albums]
    errors = metrics.delta_e(predicted, expected)
    return [EvaluatedAlbum(loaded=loaded, predicted=color, error=float(error))
            for loaded, color, error in zip(albums, predicted, errors)]


def errors_of(evaluated: list[EvaluatedAlbum]) -> np.ndarray:
    return np.array([e.error for e in evaluated])


def summary(errors: np.ndarray) -> str:
    return (f"loss (mean dE): {metrics.loss(errors):.2f}  median dE: {np.median(errors):.2f}  "
            f"hit rate (dE<{metrics.HIT_THRESHOLD:g}): {metrics.hit_rate(errors):.0%}")


def main():
    store = AlbumStore()
    albums = load_albums(store)
    evaluated = evaluate(albums, HueHistogram(), SVHistogram())

    for e in evaluated:
        print(f"{e.loaded.album.image_filename}: dom_color={e.predicted} "
              f"spotify={e.loaded.spotify_color} dE={e.error:.1f}")
    print(summary(errors_of(evaluated)))

    WORST_COUNT = 12
    WORST_COLLAGE_PATH = "worst.png"
    save_worst_collage(store, evaluated, WORST_COLLAGE_PATH, WORST_COUNT)
    print(f"saved {WORST_COUNT} worst albums to {WORST_COLLAGE_PATH}")


if __name__ == "__main__":
    main()
