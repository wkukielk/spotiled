FROM python:3.12-slim
COPY --from=ghcr.io/astral-sh/uv:latest /uv /uvx /bin/

RUN apt update \
    && apt upgrade -y \
    && apt install -y curl

WORKDIR /app
COPY pyproject.toml uv.lock ./
RUN uv sync --frozen --no-dev --no-install-project

COPY . .
RUN uv sync --frozen --no-dev

EXPOSE 8000

HEALTHCHECK --start-interval=1s --start-period=5s \
    CMD curl -fsS http://localhost:8000/ || exit 1

RUN chmod +x entrypoint.sh

ENTRYPOINT ["./entrypoint.sh"]
