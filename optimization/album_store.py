import csv
import os
import re
from dataclasses import dataclass
from pathlib import Path


@dataclass
class Album:
    id: str
    image_filename: str
    color: str  # as scraped from Spotify: "#RRGGBBAA"


def album_to_filename(name: str, artist: str) -> str:
    return re.sub(r'[\\/*?:"<>|]', "", f"{artist}-{name}.jpg")


# Albums live in a CSV (one row per album) and their covers in image_root.
class AlbumStore:
    FIELDS = ["id", "image_path", "color"]

    def __init__(self, csv_name: str | Path = "albums.csv", image_root: str | Path = "covers"):
        self._csv_name = Path(csv_name)
        self._image_root = Path(image_root)

    def image_path(self, album: Album) -> Path:
        return self._image_root / album.image_filename

    def load_all(self) -> list[Album]:
        with open(self._csv_name, "r", encoding="utf-8") as fh:
            return [
                Album(id=row["id"], image_filename=row["image_path"], color=row["color"])
                for row in csv.DictReader(fh)
            ]

    def writer(self) -> "AlbumWriter":
        """Starts a new store, replacing the existing CSV."""
        return AlbumWriter(self)


class AlbumWriter:
    def __init__(self, store: AlbumStore):
        self._store = store
        os.makedirs(store._image_root, exist_ok=True)
        self._fh = open(store._csv_name, "w", newline="", encoding="utf-8")
        self._writer = csv.writer(self._fh)
        self._writer.writerow(AlbumStore.FIELDS)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self._fh.close()

    def save(self, id: str, name: str, artist: str, image: bytes, color: str) -> Album:
        album = Album(id=id, image_filename=album_to_filename(name, artist), color=color)
        with open(self._store.image_path(album), "wb") as img_fh:
            img_fh.write(image)
        self._writer.writerow([album.id, album.image_filename, album.color])
        return album
