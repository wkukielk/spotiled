from app.esp.utils import parse_version, Version
from app.esp.services import get_latest_firmware
from app.esp.models import Chip

from sqlmodel import Session
from httpx import AsyncClient

from .utils import get_firmware


def test_parse_version():
    assert parse_version("1.2") == Version(1, 2)


def test_get_latest(session: Session):
    latest = get_firmware(2, 2)
    session.add_all([latest, get_firmware(1, 3), get_firmware(2, 1)])
    assert get_latest_firmware(session, Chip.ESP32S3) == latest


async def test_get_firmware(aclient: AsyncClient, session: Session):
    firmwares = [get_firmware(1, 4), get_firmware(2, 2), get_firmware(2, 3)]
    session.add_all(firmwares)
    headers = {"X-Firmware-Version": "1.2"}
    response = await aclient.get("/esp/firmware", headers=headers)
    assert response.status_code == 200
    exp = b"firmware 2.3"
    assert response.content == exp
