from sqlalchemy.ext.asyncio import AsyncSession
from sqlmodel import select

from .models import Device
from app.database import get_session
from app.api_keys.services import authorize

from typing import Annotated
from fastapi import Depends, HTTPException
from fastapi_api_key import ApiKey


async def create_device(session: Annotated[AsyncSession, Depends(get_session)],
                        key_id: str) -> Device:
    device = Device(api_key_id=key_id)
    session.add(device)
    return device


async def get_device(session: Annotated[AsyncSession, Depends(get_session)],
                     apikey: Annotated[ApiKey, Depends(authorize)]):
    query = select(Device).where(Device.api_key_id == apikey.id_).limit(1)
    device = (await session.execute(query)).first()
    if device is None:
        raise HTTPException(status_code=404, detail="device not found")
