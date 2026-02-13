from typing import Annotated
from fastapi import Header, APIRouter, Depends
from sqlmodel import Session
from .utils import parse_version
from app.database import get_session


router = APIRouter()


@router.get("/firmware")
async def firmware(x_firmware_version: Annotated[str | None, Header()] = None,
                   session: Session = Depends(get_session)):
    version = parse_version(x_firmware_version)
    return version
