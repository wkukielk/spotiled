from app.esp.models import Firmware
from app.database import get_session
from sqlmodel import select
import asyncio


async def main():
    gen = get_session()
    session = await anext(gen)
    r = await session.execute(select(Firmware).where(Firmware.chip == "esp32s3"))
    print(r.all())


if __name__ == '__main__':
    asyncio.run(main())
