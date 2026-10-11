#!/bin/bash

# Self-signed certificate for local development, laid out like certbot's output

CERT_DIR="$(dirname "$0")/../certbot/conf/live/localhost"
mkdir -p "$CERT_DIR"

CERT_FILE="$CERT_DIR/fullchain.pem"
KEY_FILE="$CERT_DIR/privkey.pem"

if [ ! -f "$CERT_FILE" ]; then
    openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
      -keyout "$KEY_FILE" \
      -out "$CERT_FILE" \
      -subj "/C=PL/ST=Mazowieckie/L=Warszawa/O=Dev/CN=localhost"
    # nginx runs as uid 101; fine for a throwaway dev key
    chmod 644 "$KEY_FILE"
fi
