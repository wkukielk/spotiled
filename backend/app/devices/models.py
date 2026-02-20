from sqlmodel import Field, SQLModel, Index, Relationship

from fastapi_api_key.repositories.sql import ApiKeyModel


class Device(SQLModel, table=True):
    id: int | None = Field(default=None, primary_key=True)
    api_key_id: str | None = Field(foreign_key="api_keys.id", unique=True)
    api_key: ApiKeyModel | None = Relationship(
        sa_relationship_kwargs={
            "backref": "device",
            "uselist": False,
            "primaryjoin": "Device.api_key_id == ApiKeyModel.id_"
        }
    )

    __table_args__ = (
        Index("ix_device_api_key", "api_key_id"),
    )
