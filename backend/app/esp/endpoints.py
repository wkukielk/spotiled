from typing import Annotated
from fastapi import APIRouter, Depends, Response, status
from fastapi.responses import FileResponse
from sqlmodel import Session

from .services import (
    get_latest_firmware,
    get_firmware,
    validate_requested_version as validate_req_ver,
    validate_owned_version as validate_owned_ver
)
from .models import Chip, Version

from app.config import settings
from app.database import get_session


router = APIRouter()


@router.get("/firmware/{chip}/{version_str}")
async def firmware(
        chip: Chip,
        requested_version: Annotated[Version, Depends(validate_req_ver)],
        owned_version: Annotated[Version | None, Depends(validate_owned_ver)],
        session: Annotated[Session, Depends(get_session)]):
    latest = get_latest_firmware(session, chip)
    if owned_version and owned_version >= latest.version:
        return Response(status_code=status.HTTP_304_NOT_MODIFIED)
    firmware = get_firmware(session, chip, requested_version) \
        if requested_version != "latest" else latest
    return FileResponse(settings.firmware_root_folder / firmware.relative_path)
