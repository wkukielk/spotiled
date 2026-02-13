from typing import Annotated
from fastapi import Header, APIRouter
from .utils import parse_version

type OptHeader = Annotated[str | None, Header()]


router = APIRouter()


@router.get("/firmware")
async def firmware(x_firmware_version: Annotated[str | None, Header()] = None):
    version = parse_version(x_firmware_version)
    return version
