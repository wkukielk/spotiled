#!/bin/sh

set -e

uv run alembic upgrade head

uv run fastapi run app/main.py
