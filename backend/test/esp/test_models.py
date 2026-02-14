from .utils import get_firmware


def test_version():
    firmware = get_firmware(1, 2)
    assert firmware.version == "1.2"


def test_lt1():
    firmware1 = get_firmware(1, 2)
    firmware2 = get_firmware(1, 3)
    assert firmware1 < firmware2
    assert not (firmware2 < firmware1)


def test_lt_equal():
    firmware1 = get_firmware(1, 2)
    firmware2 = get_firmware(1, 2)
    assert not (firmware1 < firmware2)
    assert not (firmware2 < firmware1)


def test_lt2():
    firmware1 = get_firmware(2, 1)
    firmware2 = get_firmware(1, 2)
    assert not (firmware1 < firmware2)
    assert firmware2 < firmware1
