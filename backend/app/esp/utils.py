from .models import SpecificVersion


def parse_specific_version(version: str) -> SpecificVersion:
    major, minor = version.split('.')
    return SpecificVersion(int(major), int(minor))
