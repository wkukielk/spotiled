from typing import Annotated
from fastapi import APIRouter, Depends, Response, status
from fastapi.responses import FileResponse
from fastapi_api_key import ApiKey

from .services import (
    validate_requested_firmware as val_req_fware,
    validate_owned_version as val_owned_ver
)
from .models import Version, FirmwareRead
from app.api_keys.services import authorize

from app.config import settings


router = APIRouter()


@router.get("/firmware/{chip}/{version_str}")
async def firmware(
        req_firmware: Annotated[FirmwareRead, Depends(val_req_fware)],
        owned_version: Annotated[Version | None, Depends(val_owned_ver)],
        apikey: Annotated[ApiKey, Depends(authorize)]):
    is_update_request = req_firmware.is_latest and owned_version is not None
    if is_update_request and owned_version >= req_firmware.version:
        return Response(status_code=status.HTTP_304_NOT_MODIFIED)
    path = settings.firmware_root_folder / req_firmware.relative_path
    return FileResponse(path)
