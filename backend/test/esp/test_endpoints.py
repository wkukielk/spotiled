from httpx import AsyncClient
from fastapi import status
from yarl import URL

FIRMWARE_URL = URL("/esp/firmware/esp32s3/")


async def test_get_firmware(aclient: AsyncClient, add_firmware):
    headers = {"X-Firmware-Version": "1.2"}
    response = await aclient.get(str(FIRMWARE_URL / "latest"), headers=headers)
    assert response.status_code == 200
    assert response.content == b"firmware 2.3"


async def test_get_firmware_uptodate(aclient: AsyncClient, add_firmware):
    headers = {"X-Firmware-Version": "2.3"}
    response = await aclient.get(str(FIRMWARE_URL / "latest"), headers=headers)
    assert response.status_code == status.HTTP_304_NOT_MODIFIED


async def test_get_firmware_specific(aclient: AsyncClient, add_firmware):
    response = await aclient.get(str(FIRMWARE_URL / "2.2"))
    assert response.status_code == 200
    assert response.content == b"firmware 2.2"
