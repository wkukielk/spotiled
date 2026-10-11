#!/bin/bash

# Renews certificates close to expiry (no-op otherwise) and reloads nginx.
# Run from cron, e.g.: 17 3,15 * * * /path/to/spotiled/proxy/renew-certs.sh

set -e

cd "$(dirname "$0")/.."

docker compose run --rm certbot renew --webroot -w /var/www/certbot
docker compose exec proxy nginx -s reload
