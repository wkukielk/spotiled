from app.esp.utils import parse_version, Version
from app.esp.services import get_latest_firmware
from app.esp.models import Chip, Firmware
from sqlmodel import Session


def test_parse_version():
    assert parse_version("1.2") == Version(1, 2)


def test_get_latest(session: Session):
    def get_fw(mj, mn):
        return Firmware(path="app.bin", major=mj, minor=mn, chip=Chip.ESP32S3)
    latest = get_fw(2, 2)
    session.add(latest)
    session.add(get_fw(1, 3))
    session.add(get_fw(2, 1))
    assert get_latest_firmware(session, Chip.ESP32S3) == latest


async def test_get_firmware(aclient):
    headers = {"X-Firmware-Version": "1.2"}
    response = await aclient.get("/esp/firmware", headers=headers)
    assert response.status_code == 200
    assert response.json()["major"] == 1
    assert response.json()["minor"] == 2
