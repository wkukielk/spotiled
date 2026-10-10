import pytest
from pathlib import Path

from app.esp.services import get_firmware, is_up_to_date, firmware_path
from app.esp.models import Chip, SpecificVersion, RequestedFirmware

from sqlalchemy.ext.asyncio import AsyncSession
from .utils import get_fware


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


@pytest.mark.parametrize("owned, expected", [
    (None, False),
    (SpecificVersion(2, 2), False),
    (SpecificVersion(2, 3), True),
    (SpecificVersion(3, 0), True),
])
def test_is_up_to_date_latest(owned, expected):
    requested = RequestedFirmware(get_fware(2, 3), is_latest=True)
    assert is_up_to_date(requested, owned) is expected


@pytest.mark.parametrize("owned", [None, SpecificVersion(2, 3),
                                   SpecificVersion(3, 0)])
def test_is_up_to_date_specific(owned):
    requested = RequestedFirmware(get_fware(2, 3), is_latest=False)
    assert is_up_to_date(requested, owned) is False


def test_firmware_path():
    path = firmware_path(get_fware(2, 3, "2_3.bin"))
    assert path == Path("/firmware/2_3.bin")
