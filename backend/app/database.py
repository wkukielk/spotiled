from sqlmodel import SQLModel
from sqlalchemy.ext.asyncio import (
    create_async_engine,
    async_sessionmaker,
    AsyncSession
)
from .esp.models import *                                 # noqa: F401, F403
from fastapi_api_key.repositories.sql import ApiKeyModel  # noqa: F401
from .devices.models import *                             # noqa: F401, F403

from fastapi_api_key.repositories.sql import Base as ApiKeyBase


sqlite_file_name = "database.db"
sqlite_url = f"sqlite+aiosqlite:///{sqlite_file_name}"

engine = create_async_engine(sqlite_url, future=True)
session_maker = async_sessionmaker(
    engine,
    class_=AsyncSession,
    expire_on_commit=False,
)


async def get_session():
    async with session_maker() as session:
        async with session.begin():
            yield session


for table in ApiKeyBase.metadata.tables.values():
    table.tometadata(SQLModel.metadata)

target_metadata = SQLModel.metadata
