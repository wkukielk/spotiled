import pytest
from httpx import AsyncClient, ASGITransport
from app.main import app


@pytest.fixture
async def aclient():
    config = {"transport": ASGITransport(app=app),
              "base_url": "http://localhost"}
    async with AsyncClient(**config) as ac:
        yield ac
