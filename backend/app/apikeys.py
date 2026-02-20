from .database import get_session, engine

import os

from sqlalchemy.ext.asyncio import AsyncSession
from fastapi import Depends, APIRouter

from fastapi_api_key import ApiKeyService
from fastapi_api_key.hasher.argon2 import Argon2ApiKeyHasher
from fastapi_api_key.repositories.sql import SqlAlchemyApiKeyRepository
from fastapi_api_key.api import create_api_keys_router, create_depends_api_key


pepper = os.getenv("API_KEYS_PEPPER")
hasher = Argon2ApiKeyHasher(pepper=pepper)


async def get_api_key_svc(
        async_session: AsyncSession = Depends(get_session)) -> ApiKeyService:
    repo = SqlAlchemyApiKeyRepository(async_session)

    # Necessary if you don't use your own DeclarativeBase
    await repo.ensure_table(async_engine=engine)

    return ApiKeyService(repo=repo, hasher=hasher)


security = create_depends_api_key(get_api_key_svc)


router = APIRouter(prefix="/api-keys", tags=["API Keys"])
router_api_keys = create_api_keys_router(get_api_key_svc, router=router)
