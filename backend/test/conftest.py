import pytest_asyncio
import os

os.environ["API_KEYS_PEPPER"] = "test"

from app.main import app
from app.database import get_session
from app.esp.models import *   # noqa: F401, F403

from sqlalchemy.ext.asyncio import (
    create_async_engine,
    async_sessionmaker,
    AsyncSession
)

from sqlalchemy import event
from sqlalchemy.pool import StaticPool
from sqlmodel import SQLModel

from httpx import AsyncClient, ASGITransport
from fastapi import FastAPI


@pytest_asyncio.fixture(scope="session", loop_scope="session")
async def engine():
    engine = create_async_engine(
        "sqlite+aiosqlite:///:memory:",
        connect_args={"check_same_thread": False},
        poolclass=StaticPool)

    @event.listens_for(engine.sync_engine, "connect")
    def do_connect(dbapi_connection, connection_record):
        dbapi_connection.isolation_level = None

    @event.listens_for(engine.sync_engine, "begin")
    def do_begin(conn):
        conn.exec_driver_sql("BEGIN")

    async with engine.begin() as conn:
        await conn.run_sync(SQLModel.metadata.create_all)

    yield engine

    await engine.dispose()


@pytest_asyncio.fixture
async def session(engine):
    async with engine.connect() as conn:
        trans = await conn.begin()
        async_session = async_sessionmaker(
            bind=conn, class_=AsyncSession, expire_on_commit=False,
            join_transaction_mode="create_savepoint")
        async with async_session() as session:
            yield session
        await trans.rollback()


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
