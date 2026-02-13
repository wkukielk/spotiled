from dataclasses import dataclass


@dataclass
class Version:
    major: int
    minor: int


def parse_version(version_str: str):
    major, minor = version_str.split('.')
    return Version(int(major), int(minor))
