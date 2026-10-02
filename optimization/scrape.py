import os
import random
from pathlib import Path
import re
import httpx
import asyncio
import numpy as np
from spotify import Spotify, SpotifyAlbumDto
import csv
from dataclasses import dataclass


@dataclass
class FetchedSpotifyAlbumDto:
    id: str
    name: str
    artist: str
    image: bytes


@dataclass
class SpotifyAlbum:
    id: str
    image_filename: str


class AlbumFetcher:
    def __init__(self, spotify: Spotify):
        self._spotify = spotify
        self._client = httpx.AsyncClient()

    async def __aenter__(self):
        return self

    async def __aexit__(self, exc_type, exc_val, exc_tb):
        await self._client.aclose()

    async def get_album(self, name) -> FetchedSpotifyAlbumDto | None:
        album = await self._spotify.fetch_album(name)
        if album is None:
            return None

        try:
            response = await self._client.get(album.image_url)            
            return FetchedSpotifyAlbumDto(
                id=album.id,
                name=album.name,
                artist=album.artist,
                image=response.content
            )
        except Exception:
            return None


def album_to_filename(name: str, artist: str):
    return re.sub(r'[\\/*?:"<>|]', "", f"{artist}-{name}.jpg")


class FileHandle:
    def __init__(self, fh):
        self._fh = fh

    def __enter__(self):
            return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        self._fh.close()

class AlbumPersister(FileHandle):
    def __init__(self, csv_name: str | Path, image_root: str | Path):
        super().__init__(open(csv_name, "w", newline="", encoding="utf-8"))
        self._image_root = Path(image_root)
        os.makedirs(image_root, exist_ok=True)
        self._writer = csv.writer(self._fh)
        self._writer.writerow(["id", "image_path"])

    def save(self, album: FetchedSpotifyAlbumDto):
        path = self._image_root / album_to_filename(album.name, album.artist)
        with open(path, 'wb') as img_fh:
            img_fh.write(album.image)
        self._writer.writerow([album.id, str(path)])


@dataclass
class SpotifyAlbumQuery:
    query: str


class AlbumNameReader(FileHandle):
    def __init__(self, csv_name: str | Path):
        super().__init__(open(csv_name, 'r', encoding="utf-8"))
        self._reader = csv.reader(self._fh)

    def __iter__(self):
        for row in self._reader:
            if not row:
                raise RuntimeError("malformed album query row")
            yield SpotifyAlbumQuery(row[0])



class AlbumScraper:
    CONCURRENCY_LIMIT = 20

    def __init__(self):
        client_id = os.getenv("SPOTIFY_CLIENT_ID")
        client_secret = os.getenv("SPOTIFY_CLIENT_SECRET")
        if not client_id or not client_secret:
            raise RuntimeError("SPOTIFY_CLIENT_ID or SPOTIFY_CLIENT_SECRET not found")
        self._spotify = Spotify.create(client_id, client_secret)

        self._semaphore = asyncio.Semaphore(AlbumScraper.CONCURRENCY_LIMIT)

    async def scrape(self):
        async with AlbumFetcher(self._spotify) as fetcher:
            with (AlbumPersister(csv_name="albums.csv", image_root="covers") as persister,
                  AlbumNameReader("queries.csv") as reader):
                await self._process(fetcher, persister, reader)
                
    async def _process(self, fetcher: AlbumFetcher, persister: AlbumPersister, reader: AlbumNameReader):
        for query in reader:
            async with self._semaphore:
                album = await fetcher.get_album(query.query)
                if not album:
                    continue
                persister.save(album)


def main():
    scraper = AlbumScraper()
    asyncio.run(scraper.scrape())


if __name__ == "__main__":
    main()
