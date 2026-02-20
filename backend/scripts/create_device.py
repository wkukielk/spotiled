import asyncio

from fastapi_api_key import ApiKey
from sqlalchemy.ext.asyncio import AsyncSession

from app.database import engine
from app.devices.models import Device
from app.api_keys.apikeys import get_api_key_svc


async def create_key(session: AsyncSession) -> tuple[ApiKey, str]:
    svc = await get_api_key_svc(session)
    key, secret = await svc.create(name="dev")
    print(f"id: {key}\nkey: {secret}")
    return key, secret


async def create_device(session: AsyncSession, key_id):
    device = Device(api_key_id=key_id)
    session.add(device)
    return device


async def main():
    async with AsyncSession(engine) as session:
        key, secret = await create_key(session)
        await create_device(session, key.id_)
        await session.commit()


if __name__ == "__main__":
    asyncio.run(main())
