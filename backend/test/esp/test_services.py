from app.esp.services import (
    validate_requested_firmware,
    validate_owned_version,
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


def test_validate_requested_firmware(session: Session, add_firmware):
    firmware = validate_requested_firmware(Chip.ESP32S3, "1.4", session)
    assert firmware.chip == Chip.ESP32S3
    assert firmware.version_str == "1.4"
    assert firmware.is_latest is False


def test_validate_requested_firmware_latest(session: Session, add_firmware):
    firmware = validate_requested_firmware(Chip.ESP32S3, "latest", session)
    assert firmware.chip == Chip.ESP32S3
    assert firmware.version_str == "2.3"
    assert firmware.is_latest is True


def test_get_firmware(session: Session, add_firmware):
    fw = get_firmware(session, Chip.ESP32S3, SpecificVersion(1, 4))
    assert fw.chip == Chip.ESP32S3
    assert fw.version_str == "1.4"


def test_get_latest_firmware(session: Session, add_firmware):
    fw = get_firmware(session, Chip.ESP32S3, "latest")
    assert fw.chip == Chip.ESP32S3
    assert fw.version_str == "2.3"


def test_get_firmware_none(session: Session):
    assert get_firmware(session, Chip.ESP32S3, SpecificVersion(1, 1)) is None
    assert get_firmware(session, Chip.ESP32S3, "latest") is None
