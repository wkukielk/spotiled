from .utils import get_fware


def test_version():
    firmware = get_fware(1, 2)
    assert firmware.version_str == "1.2"


def test_lt1():
    firmware1 = get_fware(1, 2)
    firmware2 = get_fware(1, 3)
    assert firmware1 < firmware2
    assert not (firmware2 < firmware1)


def test_lt_equal():
    firmware1 = get_fware(1, 2)
    firmware2 = get_fware(1, 2)
    assert not (firmware1 < firmware2)
    assert not (firmware2 < firmware1)


def test_lt2():
    firmware1 = get_fware(2, 1)
    firmware2 = get_fware(1, 2)
    assert not (firmware1 < firmware2)
    assert firmware2 < firmware1
