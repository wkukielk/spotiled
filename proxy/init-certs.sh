#!/bin/bash

# One-time issuance of the first Let's Encrypt certificate, run before starting the proxy.
# Requires DOMAIN in .env and certbot/ owned by uid 101:
#   mkdir -p certbot/conf certbot/www && sudo chown -R 101:101 certbot
# Usage: proxy/init-certs.sh <email>

set -e

EMAIL="${1:?usage: $0 <email>}"

cd "$(dirname "$0")/.."
source .env

# nginx isn't running yet, so certbot answers the challenge itself on port 80
docker compose run --rm -p 80:8080 certbot certonly \
    --standalone --http-01-port 8080 \
    -d "$DOMAIN" --email "$EMAIL" --agree-tos --no-eff-email
