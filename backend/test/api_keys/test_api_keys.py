from app.esp.endpoints import firmware
from httpx import AsyncClient
from yarl import URL
from ..esp.conftest import *


FIRMWARE_URL = URL("/esp/firmware/esp32s3/")


async def test_authorization(aclient: AsyncClient, apikey, add_firmware):
    headers = {"Authorization": f"Bearer {apikey}"}
    response = await aclient.get(str(FIRMWARE_URL / "latest"), headers=headers)
    assert response.status_code == 200


async def test_authorization_401(aclient: AsyncClient, apikey, add_firmware):
    response = await aclient.get(str(FIRMWARE_URL / "latest"))
    assert response.status_code == 401
