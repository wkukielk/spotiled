from app.esp.utils import parse_version, Version


def test_parse_version():
    assert parse_version("1.2") == Version(1, 2)


async def test_get_firmware(aclient):
    headers = {"X-Firmware-Version": "1.2"}
    response = await aclient.get("/esp/firmware", headers=headers)
    assert response.status_code == 200
    assert response.json()["major"] == 1
    assert response.json()["minor"] == 2
