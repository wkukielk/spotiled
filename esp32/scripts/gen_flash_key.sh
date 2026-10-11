#!/bin/sh
set -e

# Generates a host-side flash encryption key for development mode.
# 256 bits -> XTS-AES-128 (esp32s3, esp32c3), 512 bits -> XTS-AES-256 (esp32s3 only).

KEYLEN=${1:-256}
KEY_DIR="$(cd "$(dirname "$0")/.." && pwd)/private"
KEY_FILE="${KEY_DIR}/flash_encryption_key.bin"

if [ "$KEYLEN" != "256" ] && [ "$KEYLEN" != "512" ]; then
    echo "Usage: $0 [256|512]"
    exit 1
fi

if [ -f "$KEY_FILE" ]; then
    echo "Error: ${KEY_FILE} already exists, refusing to overwrite."
    echo "Devices burned with this key can only be reflashed with it."
    exit 1
fi

mkdir -p "$KEY_DIR"
espsecure generate-flash-encryption-key --keylen "$KEYLEN" "$KEY_FILE"
chmod 600 "$KEY_FILE"

echo "Flash encryption key (${KEYLEN} bits) -> ${KEY_FILE}"
