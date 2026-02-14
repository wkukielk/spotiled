from app.esp.services import (
    validate_requested_version,
    validate_owned_version,
    get_latest_firmware,
    get_firmware
)
from app.esp.models import Chip, SpecificVersion

from sqlmodel import Session


def test_validate_owned_version():
    v = validate_owned_version("1.1")
    assert v == SpecificVersion(1, 1)


def test_validate_owned_version_none():
    v = validate_owned_version()
    assert v is None


def test_validate_requested_version():
    v = validate_requested_version("1.1")
    assert v == SpecificVersion(1, 1)


def test_validate_requested_version_latest():
    v = validate_requested_version("latest")
    assert v == "latest"


def test_get_latest(session: Session, add_firmware):
    fw = get_latest_firmware(session, Chip.ESP32S3)
    assert fw.chip == Chip.ESP32S3
    assert fw.version_str == "2.3"


def test_get_firmware(session: Session, add_firmware):
    fw = get_firmware(session, Chip.ESP32S3, SpecificVersion(1, 4))
    assert fw.chip == Chip.ESP32S3
    assert fw.version_str == "1.4"
