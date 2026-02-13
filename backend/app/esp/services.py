from sqlmodel import Session, select, desc
from .models import Firmware, Chip


def get_latest_firmware(session: Session, chip: Chip) -> Firmware | None:
    return session.exec(
        select(Firmware)
        .where(Firmware.chip == chip)
        .order_by(desc(Firmware.major), desc(Firmware.minor))
    ).first()
