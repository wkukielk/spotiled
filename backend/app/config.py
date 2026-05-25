from pathlib import Path
from typing import Tuple, Type
from pydantic_settings import (
    BaseSettings,
    PydanticBaseSettingsSource,
    SettingsConfigDict,
    YamlConfigSettingsSource
)
import os

dev = os.getenv("DEV", "OFF")
cfg_file = "devconfig.yaml" if dev == "ON" else "config.yaml"


class Settings(BaseSettings):
    firmware_root_folder: Path
    api_keys_pepper: str

    model_config = SettingsConfigDict(yaml_file=cfg_file)

    @classmethod
    def settings_customise_sources(
        cls,
        settings_cls: Type[BaseSettings],
        init_settings: PydanticBaseSettingsSource,
        env_settings: PydanticBaseSettingsSource,
        dotenv_settings: PydanticBaseSettingsSource,
        file_secret_settings: PydanticBaseSettingsSource,
    ) -> Tuple[PydanticBaseSettingsSource, ...]:
        return (
            init_settings,
            env_settings,
            YamlConfigSettingsSource(settings_cls),
            dotenv_settings,
        )


settings = Settings()
