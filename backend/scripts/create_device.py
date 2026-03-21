import asyncio

from sqlalchemy.ext.asyncio import AsyncSession

from app.database import engine

from app.api_keys.services import create_key
from app.devices.services import create_device


async def main():
    async with AsyncSession(engine) as session:
        key, secret = await create_key(session, "dev")
        await create_device(session, key.id_)
        await session.commit()
    print(secret)


if __name__ == "__main__":
    asyncio.run(main())
