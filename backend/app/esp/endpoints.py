from typing import Annotated
from fastapi import APIRouter, Depends, Response, status
from fastapi.responses import FileResponse
from sqlmodel import Session

from .services import (
    get_latest_firmware,
    get_firmware,
    validate_path_version,
    validate_header_version as v_hdr_version
)
from .models import Chip, Version

from app.config import settings
from app.database import get_session


router = APIRouter()


@router.get("/firmware/{chip}/{version_str}")
async def firmware(
        chip: Chip,
        version: Annotated[Version, Depends(validate_path_version)],
        x_firmware_version: Annotated[Version | None, Depends(v_hdr_version)],
        session: Annotated[Session, Depends(get_session)]):
    latest = get_latest_firmware(session, chip)
    if x_firmware_version and x_firmware_version >= latest.version:
        return Response(status_code=status.HTTP_304_NOT_MODIFIED)
    firmware = get_firmware(session, chip, version) if version != "latest" \
        else latest
    return FileResponse(settings.firmware_root_folder / firmware.relative_path)
