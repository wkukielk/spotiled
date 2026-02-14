from typing import Annotated

from fastapi import Path, Header
from sqlmodel import Session, select, desc

from .models import Firmware, Chip, Version, SpecificVersion


def _parse_version(version: str) -> Version:
    if version == "latest":
        return "latest"
    major, minor = version.split('.')
    return SpecificVersion(int(major), int(minor))


type PathVersion = Annotated[str, Path(pattern=r"^\d+\.\d+|latest$",
                                       examples=["1.11", "latest"])]


def validate_path_version(version_str: PathVersion) -> Version:
    return _parse_version(version_str)


HeaderVersion = {"pattern": r"^\d+\.\d+$",
                 "alias": "X-Firmware-Version",
                 "examples": ["1.11"]}


def validate_header_version(
        version_hdr: Annotated[str | None, Header(**HeaderVersion)] = None) \
        -> Version | None:
    return None if version_hdr is None \
        else _parse_version(version_hdr)


def get_latest_firmware(session: Session, chip: Chip) -> Firmware | None:
    return session.exec(
        select(Firmware)
        .where(Firmware.chip == chip)
        .order_by(desc(Firmware.major), desc(Firmware.minor))
    ).first()


def get_firmware(session: Session, chip: Chip, ve: Version) -> Firmware | None:
    mj, mn = ve
    clause = Firmware.chip == chip, Firmware.major == mj, Firmware.minor == mn
    return session.exec(select(Firmware).where(*clause)).first()
