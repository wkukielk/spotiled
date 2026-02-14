from app.esp.services import (
    validate_path_version,
    validate_header_version
)
from app.esp.models import SpecificVersion


def test_validate_header_version():
    v = validate_header_version("1.1")
    assert v == SpecificVersion(1, 1)


def test_validate_header_version_latest():
    v = validate_header_version("latest")
    assert v == "latest"


def test_validate_header_version_none():
    v = validate_header_version()
    assert v is None


def test_validate_path_version():
    v = validate_path_version("1.1")
    assert v == SpecificVersion(1, 1)


def test_validate_path_version_latest():
    v = validate_path_version("latest")
    assert v == "latest"
