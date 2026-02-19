import pytest_asyncio

from app.main import app
from app.database import get_session
from app.esp.models import *   # noqa: F401, F403

from sqlalchemy.ext.asyncio import (
    create_async_engine,
    async_sessionmaker,
    AsyncSession
)

from sqlalchemy.pool import StaticPool
from sqlmodel import SQLModel

from fastapi.testclient import TestClient
from httpx import AsyncClient, ASGITransport
from fastapi import FastAPI

from sqlmodel import select


@pytest_asyncio.fixture(scope="session", loop_scope="session")
async def engine():
    engine = create_async_engine(
        "sqlite+aiosqlite:///:memory:",
        connect_args={"check_same_thread": False},
        poolclass=StaticPool)
    async with engine.begin() as conn:
        print(f"Tables found in metadata: {SQLModel.metadata.tables.keys()}")
        await conn.run_sync(SQLModel.metadata.create_all)

    yield engine

    await engine.dispose()


@pytest_asyncio.fixture
async def session(engine):
    async_session = async_sessionmaker(engine, class_=AsyncSession,
                                       expire_on_commit=False)
    async with async_session() as session:
        yield session


def mock_session(app: FastAPI, session: AsyncSession):
    async def get_session_override():
        yield session

    app.dependency_overrides[get_session] = get_session_override


@pytest_asyncio.fixture
async def aclient(session: AsyncSession):
    mock_session(app, session)
    config = {"transport": ASGITransport(app=app),
              "base_url": "http://localhost"}
    async with AsyncClient(**config) as ac:
        yield ac
    app.dependency_overrides.clear()
