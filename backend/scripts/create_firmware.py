import argparse
import asyncio
import sys

from sqlalchemy.ext.asyncio import AsyncSession

from app.database import engine

from app.esp.models import Firmware, Chip
from app.esp.services import get_firmware, firmware_path
from app.esp.utils import parse_specific_version


async def main():
    parser = argparse.ArgumentParser(description="Add a firmware entry")
    parser.add_argument("relative_path",
                        help="path relative to the firmware root folder")
    parser.add_argument("version", type=parse_specific_version,
                        help="MAJOR.MINOR, e.g. 1.0")
    parser.add_argument("--chip", type=Chip, default=Chip.ESP32S3,
                        choices=list(Chip))
    args = parser.parse_args()

    async with AsyncSession(engine) as session:
        if await get_firmware(session, args.chip, args.version):
            sys.exit(f"firmware {args.version.major}.{args.version.minor} "
                     f"for {args.chip.value} already exists")

        firmware = Firmware(relative_path=args.relative_path,
                            major=args.version.major, minor=args.version.minor,
                            chip=args.chip)
        session.add(firmware)
        await session.commit()
        await session.refresh(firmware)

    if not firmware_path(firmware).is_file():
        print(f"warning: {firmware_path(firmware)} does not exist",
              file=sys.stderr)
    print(firmware.id)


if __name__ == "__main__":
    asyncio.run(main())
