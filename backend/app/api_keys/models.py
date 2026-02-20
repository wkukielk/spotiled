from sqlalchemy.orm import relationship, Mapped

from sqlmodel import SQLModel
from sqlmodel.main import default_registry
from fastapi_api_key.repositories.sql import ApiKeyModelMixin

from typing import Optional, TYPE_CHECKING

if TYPE_CHECKING:
    from ..devices.models import Device

from sqlalchemy.orm import DeclarativeBase


class Base(DeclarativeBase):
    metadata = SQLModel.metadata
    registry = default_registry


class ApiKeyModel(Base, ApiKeyModelMixin):
    device: Mapped[Optional["Device"]] = relationship(back_populates="api_key")
