from typing import Annotated, Any
from fastapi import APIRouter, Depends, Response, status
from fastapi.responses import FileResponse

from .dependencies import (
    validate_requested_firmware as val_req_fware,
    validate_owned_version as val_owned_ver
)

from .models import SpecificVersion, RequestedFirmware
from . import services
from app.api_keys.services import authorize


router = APIRouter()

responses: dict[int | str, dict[str, Any]] = {
    304: {"description": "You own the latest stable version"}
}


@router.get("/firmware/{chip}/{version}", responses=responses,
            dependencies=[Depends(authorize)])
async def firmware(
        req_firmware: Annotated[RequestedFirmware, Depends(val_req_fware)],
        owned_version: Annotated[SpecificVersion | None, Depends(val_owned_ver)]):
    if services.is_up_to_date(req_firmware, owned_version):
        return Response(status_code=status.HTTP_304_NOT_MODIFIED)
    return FileResponse(services.firmware_path(req_firmware.firmware))
