from typing import Annotated

from fastapi import Path, Header, Depends, Query
from pydantic import StringConstraints
from sqlalchemy.ext.asyncio import AsyncSession

from app.database import get_session
from app.utils import get_or_404
from .models import SpecificVersion, Version, RequestedFirmware, Chip
from . import services
from .utils import parse_specific_version

type PathVersion = Annotated[str, Path(pattern=r"^(\d+\.\d+|latest)$",
                                       examples=["1.11", "latest"])]

type VersionStr = Annotated[str, StringConstraints(pattern=r"^\d+\.\d+$")]
type Blacklist = Annotated[list[VersionStr], Query(examples=[["1.11", "1.12"]])]


def _parse_version(version: str) -> Version:
    if version == "latest":
        return "latest"
    return parse_specific_version(version)


async def validate_requested_firmware(
        chip: Chip,
        version: PathVersion,
        session: Annotated[AsyncSession, Depends(get_session)],
        blacklist: Blacklist = []) -> RequestedFirmware:
    version_v = _parse_version(version)
    blacklist_v = [parse_specific_version(v) for v in blacklist]
    firmware = await services.get_firmware(session, chip, version_v,
                                           blacklist_v)
    return RequestedFirmware(get_or_404(firmware),
                             is_latest=version_v == "latest")


async def validate_owned_version(
        owned_version: Annotated[VersionStr | None, Header(examples=["1.11"])] = None
) -> SpecificVersion | None:
    return None if owned_version is None else parse_specific_version(owned_version)
