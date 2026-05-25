import asyncio
import sys

from app.api_keys.apikeys import get_api_key_svc
from app.database import get_session, engine

from fastapi_api_key import ApiKey
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy.ext.asyncio import AsyncSession
from fastapi import Depends

from typing import Annotated


async def verify_key(session: Annotated[AsyncSession, Depends(get_session)],
                     key: str) -> tuple[ApiKey, str]:
    svc = await get_api_key_svc(session)
    return await svc.verify_key(key)


async def main():
    key = sys.argv[1]
    async with AsyncSession(engine) as session:
        key_obj = await verify_key(session, key)
    print(key_obj)


if __name__ == "__main__":
    asyncio.run(main())
