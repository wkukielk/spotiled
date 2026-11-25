import numpy as np
import spotipy
from spotipy.oauth2 import SpotifyClientCredentials
from dataclasses import dataclass


@dataclass
class SpotifyAlbumDto:
    id: str
    image_url: str


class Spotify:
    def __init__(self, client_id, client_secret):
        self._sp = spotipy.Spotify(
            auth_manager=SpotifyClientCredentials(
                client_id=client_id,
                client_secret=client_secret
            )
        )

    def fetch_album(self, name) -> SpotifyAlbumDto | None:
        album_dict = self._get_album(name)
        return self._map_album(album_dict) if album_dict else None

    def get_token(self) -> str | None:
        mgr = self._sp.auth_manager
        return mgr.get_access_token() if mgr else None

    def _get_album(self, name) -> dict | None:
        results = self._sp.search(
            name,
            limit=1,
            offset=0,
            type="album",
            market="PL"
        )
        albums_dict = (results or {}).get("albums")
        albums = albums_dict.get("items", []) if albums_dict else []
        return albums[0] if albums else None


    def _get_playlist_item_album(self, item: dict) -> dict | None:
        track = item.get("track")
        if not track or not track["album"]:
            return None
        album = track["album"]
        if not album.get("id") or not album.get("images"):
            return None
        images = album["images"]
        return album if images[0].get("url") else None


    def _map_album(self, album_dict: dict) -> SpotifyAlbumDto | None:
        images = album_dict.get("images")
        if not images or not images[-1].get("url"):
            return None

        return SpotifyAlbumDto(
            id=album_dict["id"],
            image_url=images[0]["url"],
        )
