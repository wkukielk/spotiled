import pytest
from .utils import get_fware

from pyfakefs.fake_filesystem import FakeFilesystem
from sqlmodel import Session
from pathlib import Path

from app.config import settings


settings.firmware_root_folder = Path("/firmware/")


@pytest.fixture
def add_firmware(session: Session, fs: FakeFilesystem):
    firmwares = [get_fware(1, 4, "1_4.bin"),
                 get_fware(2, 2, "2_2.bin"),
                 get_fware(2, 3, "2_3.bin")]
    for f in firmwares:
        path = settings.firmware_root_folder / f.relative_path
        fs.create_file(path, contents=f"firmware {f.version_str}")
    session.add_all(firmwares)
