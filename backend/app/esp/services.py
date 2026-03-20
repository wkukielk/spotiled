from typing import Annotated

from fastapi import Path, Header, Depends, Query
from sqlmodel import Session, select, desc, tuple_
from sqlalchemy.ext.asyncio import AsyncSession

from app.database import get_session
from app.utils import get_or_404
from .models import Firmware, FirmwareRead, Chip, Version, SpecificVersion


def _parse_specific_version(version: str) -> SpecificVersion:
    major, minor = version.split('.')
    return SpecificVersion(int(major), int(minor))


def _parse_version(version: str) -> Version:
    if version == "latest":
        return "latest"
    return _parse_specific_version(version)


type PathVersion = Annotated[str, Path(pattern=r"^\d+\.\d+|latest$",
                                       examples=["1.11", "latest"])]

type Blacklist = Annotated[list[str], Query(examples=[["1.11", "1.12"]])]


async def validate_requested_firmware(
        chip: Chip,
        version: PathVersion,
        session: Annotated[AsyncSession, Depends(get_session)],
        blacklist: Blacklist = []) -> FirmwareRead:
    version_v = _parse_version(version)
    blacklist_v = [_parse_specific_version(v) for v in blacklist]
    firmware_opt = await get_firmware(session, chip, version_v, blacklist_v)
    firmware = get_or_404(firmware_opt)
    update_is_latest = {"is_latest": version_v == "latest"}
    return FirmwareRead.model_validate(firmware, update=update_is_latest)


async def validate_owned_version(
        Owned_Version: Annotated[str | None, Header(pattern=r"^\d+\.\d+$",
                                                    examples=["1.11"])] = None
) -> SpecificVersion | None:
    return None if Owned_Version is None else _parse_version(Owned_Version)


async def get_firmware(session: Session, chip: Chip, version: Version,
                       blacklist: list[SpecificVersion] = []):
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
