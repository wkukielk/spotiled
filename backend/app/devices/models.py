from sqlmodel import Field, SQLModel, Index, Relationship

from ..api_keys.models import ApiKeyModel


class Device(SQLModel, table=True):
    id: int | None = Field(default=None, primary_key=True)
    api_key_id: str | None = Field(foreign_key="api_keys.id", unique=True)

    api_key: ApiKeyModel = Relationship(back_populates="device")

    __table_args__ = (
        Index("ix_device_api_key", "api_key_id"),
    )
