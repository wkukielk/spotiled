from typing import Annotated

from fastapi import Path, Header, Depends
from sqlmodel import Session, select, desc
from sqlalchemy.ext.asyncio import AsyncSession

from app.database import get_session
from app.utils import get_or_404
from .models import Firmware, FirmwareRead, Chip, Version, SpecificVersion


def _parse_version(version: str) -> Version:
    if version == "latest":
        return "latest"
    major, minor = version.split('.')
    return SpecificVersion(int(major), int(minor))


type PathVersion = Annotated[str, Path(pattern=r"^\d+\.\d+|latest$",
                                       examples=["1.11", "latest"])]


async def validate_requested_firmware(
        chip: Chip,
        version_str: PathVersion,
        session: Annotated[AsyncSession, Depends(get_session)]) \
            -> FirmwareRead:
    version = _parse_version(version_str)
    firmware = get_or_404(await get_firmware(session, chip, version))
    update_is_latest = {"is_latest": version == "latest"}
    return FirmwareRead.model_validate(firmware, update=update_is_latest)


HeaderVersion = {"pattern": r"^\d+\.\d+$",
                 "alias": "X-Firmware-Version",
                 "examples": ["1.11"]}


async def validate_owned_version(
            version_hdr: Annotated[str | None, Header(**HeaderVersion)] = None
        ) -> SpecificVersion | None:
    return None if version_hdr is None \
        else _parse_version(version_hdr)


async def get_firmware(session: Session, chip: Chip, version: Version):
    if version == "latest":
        return await _get_latest_firmware(session, chip)
    else:
        return await _get_firmware_by_version(session, chip, version)


async def _get_latest_firmware(session: AsyncSession, chip: Chip) \
        -> Firmware | None:
    cursor = await session.execute(
        select(Firmware)
        .where(Firmware.chip == chip)
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
