from app.esp.models import Chip, Firmware


def get_fware(major: int, minor: int, path="app.bin"):
    c = Chip.ESP32S3
    return Firmware(relative_path=path, major=major, minor=minor, chip=c)
