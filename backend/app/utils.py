from fastapi import HTTPException


def get_or_404[T](obj: T | None, message="Resource not found") -> T:
    if obj is None:
        raise HTTPException(status_code=404, detail=message)
    return obj
