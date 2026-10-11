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

ARG APP_UID
ARG APP_GID
RUN test -n "$APP_UID" && test -n "$APP_GID" \
    && groupadd --non-unique --gid ${APP_GID} app \
    && useradd --non-unique --uid ${APP_UID} --gid ${APP_GID} --create-home app \
    && chown -R app:app db/ \
    && chown -R app:app data/

ENV UV_NO_SYNC=1

USER app

ENTRYPOINT ["./entrypoint.sh"]
