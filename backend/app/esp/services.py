from typing import Annotated

from fastapi import Path, Header
from sqlmodel import Session, select, desc

from .models import Firmware, Chip, Version, SpecificVersion


def _parse_version(version: str) -> Version:
    if version == "latest":
        return "latest"
    major, minor = version.split('.')
    return SpecificVersion(int(major), int(minor))


PathVersion = Path(pattern=r"^\d+\.\d+|latest$", examples=["1.11"])
HeaderVersion = Header(pattern=r"^\d+\.\d+|latest$", examples=["1.11"])


def validate_path_version(version: Annotated[str, PathVersion]) -> Version:
    return _parse_version(version)


def validate_header_version(
        version: Annotated[str | None, HeaderVersion] = None
        ) -> Version | None:
    return _parse_version(version) if version is not None else None


def get_latest_firmware(session: Session, chip: Chip) -> Firmware | None:
    return session.exec(
        select(Firmware)
        .where(Firmware.chip == chip)
        .order_by(desc(Firmware.major), desc(Firmware.minor))
    ).first()


def get_firmware(session: Session, chip: Chip, ve: Version) -> Firmware | None:
    mj, mn = ve
    c = Firmware.chip == chip and Firmware.major == mj and Firmware.minor == mn
    return session.exec(select(Firmware).where(c)).first()
