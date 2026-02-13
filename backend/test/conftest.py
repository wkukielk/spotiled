import pytest

from app.main import app
from app.database import get_session

from sqlalchemy.pool import StaticPool
from sqlmodel import SQLModel, create_engine, Session

from fastapi.testclient import TestClient
from httpx import AsyncClient, ASGITransport
from fastapi import FastAPI


@pytest.fixture(scope="session")
def engine():
    engine = create_engine("sqlite:///:memory:",
                           connect_args={"check_same_thread": False},
                           poolclass=StaticPool)
    SQLModel.metadata.create_all(engine)
    return engine


@pytest.fixture
def session(engine):
    with Session(engine) as session:
        yield session


def mock_session(app: FastAPI, session: Session):
    def get_session_override():
        return session

    app.dependency_overrides[get_session] = get_session_override


@pytest.fixture
def client(session: Session):
    mock_session(app, session)
    with TestClient(app) as client:
        yield client
    app.dependency_overrides.clear()


@pytest.fixture
async def aclient(session: Session):
    mock_session(app, session)
    config = {"transport": ASGITransport(app=app),
              "base_url": "http://localhost"}
    async with AsyncClient(**config) as ac:
        yield ac
    app.dependency_overrides.clear()
