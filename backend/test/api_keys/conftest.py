import pytest_asyncio

from sqlalchemy.ext.asyncio import AsyncSession

from app.api_keys.services import create_key


@pytest_asyncio.fixture
async def apikey(session: AsyncSession):
    key, secret = await create_key(session, "test")
    await session.commit()
    return secret
