#!/bin/sh
set -e

# Burns the host-generated flash encryption key (see gen_flash_key.sh) into eFuses.
# Irreversible: the device can only be flashed with this key afterwards.
# 256-bit key -> XTS_AES_128_KEY in one block,
# 512-bit key -> XTS_AES_256_KEY split by espefuse over BLOCK and the next one (esp32s3 only).

if [ "$#" -lt 1 ] || [ "$#" -gt 2 ]; then
    echo "Usage: $0 <port> [block (default BLOCK_KEY0)]"
    exit 1
fi

PORT=$1
BLOCK=${2:-BLOCK_KEY0}
KEY_FILE="$(cd "$(dirname "$0")/.." && pwd)/private/flash_encryption_key.bin"

if [ ! -f "$KEY_FILE" ]; then
    echo "Error: ${KEY_FILE} not found, run gen_flash_key.sh first."
    exit 1
fi

case `wc -c < "$KEY_FILE" | tr -d ' '` in
    32) PURPOSE=XTS_AES_128_KEY ;;
    64) PURPOSE=XTS_AES_256_KEY ;;
    *)
        echo "Error: unexpected key size in ${KEY_FILE}"
        exit 1
        ;;
esac

espefuse --port "$PORT" burn-key "$BLOCK" "$KEY_FILE" "$PURPOSE"
