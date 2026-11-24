from ..database import get_session
from app.config import settings


from sqlalchemy.ext.asyncio import AsyncSession
from fastapi import Depends

from fastapi_api_key import ApiKeyService
from fastapi_api_key.hasher.argon2 import Argon2ApiKeyHasher
from fastapi_api_key.repositories.sql import SqlAlchemyApiKeyRepository
from fastapi_api_key.api import create_depends_api_key


hasher = Argon2ApiKeyHasher(pepper=settings.api_keys_pepper)


async def get_api_key_svc(
        async_session: AsyncSession = Depends(get_session)) -> ApiKeyService:
    repo = SqlAlchemyApiKeyRepository(async_session)
    return ApiKeyService(repo=repo, hasher=hasher)


security = create_depends_api_key(get_api_key_svc)
