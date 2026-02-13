from sqlmodel import Field, SQLModel, Index
from enum import Enum
from sqlalchemy.ext.hybrid import hybrid_property
from sqlalchemy import cast, String
from pydantic import computed_field


class Chip(str, Enum):
    ESP32S3 = "esp32s3"


class Release(str, Enum):
    STABLE = "stable"
    DEV = "dev"


class Firmware(SQLModel, table=True):
    __table_args__ = (
        Index("ix_firmware_version_sort", "major", "minor"),
    )
    id: int | None = Field(default=None, primary_key=True)
    path: str
    major: int
    minor: int
    chip: Chip

    @computed_field(return_type=str)
    def version(self) -> str:
        return f"{self.major}.{self.minor}"
