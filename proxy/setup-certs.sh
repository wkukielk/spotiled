#!/bin/bash

mkdir -p ssl/

CERT_FILE="ssl/nginx.crt"
KEY_FILE="ssl/nginx.key"

if [ ! -f "$CERT_FILE" ]; then
    openssl req -x509 -nodes -days 365 -newkey rsa:2048 \
      -keyout "$KEY_FILE" \
      -out "$CERT_FILE" \
      -subj "/C=PL/ST=Mazowieckie/L=Warszawa/O=Dev/CN=localhost"
fi
