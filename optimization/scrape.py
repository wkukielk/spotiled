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
from playwright.async_api import async_playwright, Browser, Route
from album_store import AlbumStore, AlbumWriter


@dataclass
class FetchedSpotifyAlbumDto:
    id: str
    name: str
    artist: str
    image: bytes
    image_color: str


class AlbumFetcher:
    # Not needed to read the album color; skipping them speeds up page loads.
    BLOCKED_RESOURCES = {"image", "font", "media"}

    def __init__(self, spotify: Spotify, browser: Browser):
        self._spotify = spotify
        self._browser = browser
        self._client = httpx.AsyncClient()

    async def __aenter__(self):
        return self

    async def __aexit__(self, exc_type, exc_val, exc_tb):
        await self._client.aclose()

    async def get_album(self, name) -> FetchedSpotifyAlbumDto | None:
        album = await self._spotify.fetch_album(name)
        if album is None:
            return None
        cover = await self._fetch_cover(album.image_url)
        if cover is None:
            return None
        color_dict = await self._fetch_cover_color(album.id)
        color = color_dict["background_base_min_contrast"]
        if color is None:
            return None

        return FetchedSpotifyAlbumDto(
            id=album.id,
            name=album.name,
            artist=album.artist,
            image=cover,
            image_color=color
        )
        

    async def _fetch_cover(self, cover_url: str) -> bytes | None:
        try:
            response = await self._client.get(cover_url)
            return response.content
        except Exception:
            return None

    async def _fetch_cover_color(self, album_id: str):
        album_url = f"https://open.spotify.com/album/{album_id}"
        async with await self._browser.new_page() as page:
            await page.route("**/*", self._block_heavy_resources)
            await page.goto(album_url, wait_until="domcontentloaded")

            selector = '[style*="--background-base-min-contrast"]'
            await page.wait_for_selector(selector, timeout=10000)

            element = page.locator(selector).first
            style_attr = await element.get_attribute("style") or ""

        base_match = re.search(r"--background-base:\s*(#[0-9a-fA-F]+)", style_attr)
        min_contrast_match = re.search(r"--background-base-min-contrast:\s*(#[0-9a-fA-F]+)", style_attr)

        return {
            "background_base": base_match.group(1) if base_match else None,
            "background_base_min_contrast": min_contrast_match.group(1) if min_contrast_match else None
        }

    @staticmethod
    async def _block_heavy_resources(route: Route):
        if route.request.resource_type in AlbumFetcher.BLOCKED_RESOURCES:
            await route.abort()
        else:
            await route.continue_()


class FileHandle:
    def __init__(self, fh):
        self._fh = fh

    def __enter__(self):
            return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self._fh.close()


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
    CONCURRENCY_LIMIT = 10

    def __init__(self):
        client_id = os.getenv("SPOTIFY_CLIENT_ID")
        client_secret = os.getenv("SPOTIFY_CLIENT_SECRET")
        if not client_id or not client_secret:
            raise RuntimeError("SPOTIFY_CLIENT_ID or SPOTIFY_CLIENT_SECRET not found")
        self._spotify = Spotify.create(client_id, client_secret)

        self._semaphore = asyncio.Semaphore(AlbumScraper.CONCURRENCY_LIMIT)

    async def scrape(self):
        async with (async_playwright() as playwright,
                    await playwright.chromium.launch(headless=True) as browser,
                    AlbumFetcher(self._spotify, browser) as fetcher):
            with (AlbumStore().writer() as album_writer,
                  AlbumNameReader("queries.csv") as reader):
                await self._process(fetcher, album_writer, reader)

    async def _process(self, fetcher: AlbumFetcher, album_writer: AlbumWriter, reader: AlbumNameReader):
        async with asyncio.TaskGroup() as tg:
            for query in reader:
                tg.create_task(self._process_one(fetcher, album_writer, query))

    async def _process_one(self, fetcher: AlbumFetcher, album_writer: AlbumWriter, query: SpotifyAlbumQuery):
        async with self._semaphore:
            try:
                album = await fetcher.get_album(query.query)
            except Exception as e:
                print(f"failed to fetch {query.query}: {e!r}")
                return
            if not album:
                print(f"failed to fetch {query.query}")
                return
            album_writer.save(id=album.id, name=album.name, artist=album.artist,
                              image=album.image, color=album.image_color)
            print("ok")


def main():
    scraper = AlbumScraper()
    asyncio.run(scraper.scrape())


if __name__ == "__main__":
    main()
