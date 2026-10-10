from sqlmodel import Field, SQLModel, Index
from enum import Enum
from typing import Annotated, NamedTuple, Literal, Self
from pydantic import computed_field
from functools import total_ordering


class SpecificVersion(NamedTuple):
    major: int
    minor: int


type Version = SpecificVersion | Literal["latest"]


class Chip(str, Enum):
    ESP32S3 = "esp32s3"


class Release(str, Enum):
    STABLE = "stable"
    DEV = "dev"


@total_ordering
class Firmware(SQLModel, table=True):
    id: Annotated[int | None, Field(default=None, primary_key=True)]
    relative_path: str
    major: int
    minor: int
    chip: Chip

    @computed_field(return_type=str)
    @property
    def version_str(self) -> str:
        return f"{self.major}.{self.minor}"

    @computed_field(return_type=SpecificVersion)
    @property
    def version(self) -> SpecificVersion:
        return SpecificVersion(self.major, self.minor)

    def __lt__(a, b: Self):
        return a.minor < b.minor if a.major == b.major else a.major < b.major

    __table_args__ = (
        Index("ix_firmware_version_sort", "major", "minor"),
    )


class RequestedFirmware(NamedTuple):
    firmware: Firmware
    is_latest: bool = False
