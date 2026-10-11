#!/bin/sh
set -e

# Generates NVS encryption keys (an nvs_keys partition image) used to encrypt encnvs
# in RELEASE builds. The keys are flashed to the nvs_key partition alongside encnvs.

KEY_DIR="$(cd "$(dirname "$0")/.." && pwd)/private"
KEY_FILE="${KEY_DIR}/nvs_keys.bin"

if [ -f "$KEY_FILE" ]; then
    echo "Error: ${KEY_FILE} already exists, refusing to overwrite."
    echo "encnvs on flashed devices can only be read with these keys."
    exit 1
fi

mkdir -p "$KEY_DIR"
# generate-key always creates a keys/ dir in --outdir, keep it out of private/
TMP_DIR=`mktemp -d`
python3 "${IDF_PATH}/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py" \
    generate-key --keyfile "$KEY_FILE" --outdir "$TMP_DIR"
rm -rf "$TMP_DIR"
chmod 600 "$KEY_FILE"

echo "NVS keys -> ${KEY_FILE}"
