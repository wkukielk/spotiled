import pytest

from httpx import AsyncClient
from fastapi import status
from yarl import URL

from app.main import app
from app.api_keys.services import authorize


FIRMWARE_URL = URL("/esp/firmware/esp32s3/")


@pytest.fixture
def mock_security():
    async def get_mock_authorize():
        yield None

    app.dependency_overrides[authorize] = get_mock_authorize


@pytest.fixture
def client(aclient: AsyncClient, mock_security):
    yield aclient


async def test_get_firmware(client: AsyncClient, add_firmware):
    headers = {"Owned-Version": "1.2"}
    response = await client.get(str(FIRMWARE_URL / "latest"), headers=headers)
    assert response.status_code == 200
    assert response.content == b"firmware 2.3"


async def test_get_firmware_422(client: AsyncClient, add_firmware):
    headers = {"Owned-Version": "1.2a"}
    response = await client.get(str(FIRMWARE_URL / "latest"), headers=headers)
    assert response.status_code == 422


async def test_get_firmware_uptodate(client: AsyncClient, add_firmware):
    headers = {"Owned-Version": "2.3"}
    response = await client.get(str(FIRMWARE_URL / "latest"), headers=headers)
    assert response.status_code == status.HTTP_304_NOT_MODIFIED


async def test_get_firmware_specific(client: AsyncClient, add_firmware):
    response = await client.get(str(FIRMWARE_URL / "2.2"))
    assert response.status_code == 200
    assert response.content == b"firmware 2.2"


async def test_get_firmware_specific_owned(client: AsyncClient, add_firmware):
    headers = {"Owned-Version": "2.3"}
    response = await client.get(str(FIRMWARE_URL / "2.2"), headers=headers)
    assert response.status_code == 200
    assert response.content == b"firmware 2.2"


async def test_get_firmware_none(client: AsyncClient):
    response = await client.get(str(FIRMWARE_URL / "9.9"))
    assert response.status_code == 404


async def test_get_firmware_none_latest(client: AsyncClient):
    response = await client.get(str(FIRMWARE_URL / "latest"))
    assert response.status_code == 404
