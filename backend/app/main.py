from fastapi import FastAPI
from .esp.endpoints import router as esp_router

app = FastAPI()
app.include_router(esp_router, prefix="/esp")


@app.get("/")
async def root():
    return {"message": "Hello World"}
