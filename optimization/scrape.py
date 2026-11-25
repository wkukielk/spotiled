import os
import random
import re
import httpx
import asyncio
import numpy as np
from spotify import Spotify, SpotifyAlbumDto
from spotipy.oauth2 import SpotifyClientCredentials
from dataclasses import dataclass


SEED_PLAYLIST_IDS = [
    "żabson internaziomal",
]


@dataclass
class SpotifyAlbumFullDto:
    id: str
    image: bytes


@dataclass
class SpotifyAlbum:
    id: str
    image_filename: str


class Scraper:
    def __init__(self, spotify: Spotify):
        self._spotify = spotify

    def _get_album(self, name) -> SpotifyAlbumFullDto | None:
        album = self._spotify.fetch_album(name)
        if album is None:
            return None

        try:
            response = requests.get(album.image_url, timeout=1)
            return SpotifyAlbumFullDto(
                id=album.id,
                image=response.content
            )
        except Exception:
            return None

    def 
    


def draw_weighted_random_album(album_pool):
    """Draws an album weighted by its popularity score (0-100)."""
    scores = [max(a.get("popularity", 1), 1) for a in album_pool]
    scores_np = np.array(scores, dtype=float)

    probabilities = scores_np / scores_np.sum()

    chosen_index = np.random.choice(len(album_pool), p=probabilities)
    return album_pool[chosen_index]

def download_cover(album, output_dir="covers"):
    resp = requests.get(image_url)
    with open(filepath, "wb") as f:
        f.write(resp.content)


if __name__ == "__main__":
    client_id = os.getenv("SPOTIFY_CLIENT_ID")
    client_secret = os.getenv("SPOTIFY_CLIENT_SECRET")
    spotify = Spotify(client_id, client_secret)
    album = spotify.fetch_album(SEED_PLAYLIST_IDS[0])

    print(album)

