import pytest

from app.esp.services import (
    validate_requested_firmware,
    validate_owned_version,
    get_firmware
)
from app.esp.models import Chip, SpecificVersion

from sqlalchemy.ext.asyncio import AsyncSession


async def test_validate_owned_version():
    v = await validate_owned_version("1.1")
    assert v == SpecificVersion(1, 1)


async def test_validate_owned_version_none():
    v = await validate_owned_version()
    assert v is None


async def test_validate_req_firmware(session: AsyncSession, add_firmware):
    firmware = await validate_requested_firmware(Chip.ESP32S3, "1.4", session)
    assert firmware.chip == Chip.ESP32S3
    assert firmware.version_str == "1.4"
    assert firmware.is_latest is False


async def test_validate_req_fware_latest(session: AsyncSession, add_firmware):
    fw = await validate_requested_firmware(Chip.ESP32S3, "latest", session)
    assert fw.chip == Chip.ESP32S3
    assert fw.version_str == "2.3"
    assert fw.is_latest is True


async def test_val_req_fware_blacklist(session: AsyncSession, add_firmware):
    blacklist = ["2.3"]
    fw = await validate_requested_firmware(Chip.ESP32S3, "latest", session,
                                           blacklist)
    assert fw.chip == Chip.ESP32S3
    assert fw.version_str == "2.2"
    assert fw.is_latest is True


async def test_val_rq_fw_blacklist_latest(session: AsyncSession, add_firmware):
    blacklist = ["latest"]
    with pytest.raises(ValueError):
        await validate_requested_firmware(Chip.ESP32S3, "latest", session,
                                          blacklist)


async def test_get_firmware(session: AsyncSession, add_firmware):
    fw = await get_firmware(session, Chip.ESP32S3, SpecificVersion(1, 4))
    assert fw.chip == Chip.ESP32S3
    assert fw.version_str == "1.4"


async def test_get_latest_firmware(session: AsyncSession, add_firmware):
    fw = await get_firmware(session, Chip.ESP32S3, "latest")
    assert fw.chip == Chip.ESP32S3
    assert fw.version_str == "2.3"


async def test_get_firmware_none(session: AsyncSession):
    assert await get_firmware(session, Chip.ESP32S3, SpecificVersion(1, 1)) is None
    assert await get_firmware(session, Chip.ESP32S3, "latest") is None
