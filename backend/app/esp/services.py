from pathlib import Path

from sqlmodel import select, desc, tuple_
from sqlalchemy.ext.asyncio import AsyncSession

from app.config import settings
from .models import (
    Firmware, RequestedFirmware, Chip, Version, SpecificVersion
)


def is_up_to_date(requested: RequestedFirmware,
                  owned_version: SpecificVersion | None) -> bool:
    if owned_version is None:
        return False
    return requested.is_latest and owned_version >= requested.firmware.version


def firmware_path(firmware: Firmware) -> Path:
    return settings.firmware_root_folder / firmware.relative_path


async def get_firmware(
        session: AsyncSession, chip: Chip, version: Version,
        blacklist: list[SpecificVersion] = []) -> Firmware | None:
    if version == "latest":
        return await _get_latest_firmware(session, chip, blacklist)
    else:
        return await _get_firmware_by_version(session, chip, version)


async def _get_latest_firmware(session: AsyncSession, chip: Chip,
                               blacklist: list[SpecificVersion] = []) \
        -> Firmware | None:
    cursor = await session.execute(
        select(Firmware)
        .where(Firmware.chip == chip,
               ~tuple_(Firmware.major, Firmware.minor).in_(blacklist))
        .order_by(desc(Firmware.major), desc(Firmware.minor))
        .limit(1)
    )
    return cursor.scalars().first()


async def _get_firmware_by_version(
        session: AsyncSession, chip: Chip, version: SpecificVersion) \
        -> Firmware | None:
    mj, mn = version
    clause = Firmware.chip == chip, Firmware.major == mj, Firmware.minor == mn
    cursor = await session.execute(select(Firmware).where(*clause).limit(1))
    return cursor.scalars().first()
