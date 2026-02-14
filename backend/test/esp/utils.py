from app.esp.models import Chip, Firmware


def get_firmware(major: int, minor: int):
    chip = Chip.ESP32S3
    return Firmware(path="app.bin", major=major, minor=minor, chip=chip)
