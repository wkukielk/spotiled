from .apikeys import get_api_key_svc, security
from app.database import get_session

from fastapi_api_key import ApiKey
from sqlalchemy.ext.asyncio import AsyncSession
from fastapi import Depends, HTTPException

from typing import Annotated


async def create_key(session: Annotated[AsyncSession, Depends(get_session)],
                     name: str) -> tuple[ApiKey, str]:
    svc = await get_api_key_svc(session)
    key, secret = await svc.create(name)
    return key, secret


async def authorize(apikey: Annotated[ApiKey, Depends(security)]):
    if apikey is None:
        raise HTTPException(status_code=401, detail="invalid api key")
    return apikey
