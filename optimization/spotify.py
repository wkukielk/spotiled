import numpy as np
import tekore as tk
from typing import cast
from collections.abc import Awaitable
from dataclasses import dataclass
from tekore.model import SimpleAlbumPaging, SimpleAlbum


@dataclass
class SpotifyAlbumDto:
    id: str
    name: str
    artist: str
    image_url: str


class Spotify:
    def __init__(self, spotify: tk.Spotify):
        self._spotify = spotify

    @classmethod
    def create(cls, client_id: str, client_secret: str) -> "Spotify":
        token = tk.request_client_token(client_id, client_secret)
        spotify = tk.Spotify(token, asynchronous=True)
        return cls(spotify)


    async def fetch_album(self, name) -> SpotifyAlbumDto | None:
        simple_album = await self._get_album(name)
        return self._map_album(simple_album) if simple_album else None

    async def _get_album(self, name) -> SimpleAlbum | None:
        albums: SimpleAlbumPaging
        albums, = await cast(
            Awaitable[tuple[SimpleAlbumPaging]],
            self._spotify.search(name, types=("album",), market="PL", limit=1),
        )
        return albums.items[0] if albums.items else None


    def _get_playlist_item_album(self, item: dict) -> dict | None:
        track = item.get("track")
        if not track or not track["album"]:
            return None
        album = track["album"]
        if not album.get("id") or not album.get("images"):
            return None
        images = album["images"]
        return album if images[0].get("url") else None


    def _map_album(self, simple_album: SimpleAlbum) -> SpotifyAlbumDto | None:
        images = simple_album.images
        if not images or not simple_album.artists:
            return None

        return SpotifyAlbumDto(
            id=simple_album.id,
            name=simple_album.name,
            artist=simple_album.artists[0].name,
            image_url=images[-1].url,
        )
