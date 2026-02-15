from fastapi import HTTPException
from typing import Any


def get_or_404(obj: Any | None, message="Resource not found"):
    if obj is None:
        raise HTTPException(status_code=404, detail=message)
    return obj
