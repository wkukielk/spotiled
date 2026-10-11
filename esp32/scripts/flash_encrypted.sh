#!/bin/sh
set -e

# Encrypts the built images on host with the key from gen_flash_key.sh and flashes the ciphertext.
# Also generates encnvs from enc.csv encrypted with NVS keys from gen_nvs_keys.sh and flashes
# it together with the keys (nvs_key partition, flash-encrypted).
# Only for devices already running with flash encryption enabled - the very first flash
# must be plaintext (idf.py flash), so the bootloader encrypts it in place on first boot.

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <port>"
    exit 1
fi

PORT=$1
ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
KEY_FILE="${ROOT_DIR}/private/flash_encryption_key.bin"
NVS_KEYS_FILE="${ROOT_DIR}/private/nvs_keys.bin"
ENC_CSV="${ROOT_DIR}/firmware/nvs/enc.csv"
FLASHER_ARGS="${BUILD_DIR}/flasher_args.json"
PARTITION_TABLE="${BUILD_DIR}/partition_table/partition-table.bin"
ENC_DIR="${BUILD_DIR}/encrypted"

if [ ! -f "$KEY_FILE" ]; then
    echo "Error: ${KEY_FILE} not found, run gen_flash_key.sh first."
    exit 1
fi

if [ ! -f "$NVS_KEYS_FILE" ]; then
    echo "Error: ${NVS_KEYS_FILE} not found, run gen_nvs_keys.sh first."
    exit 1
fi

if [ ! -f "$FLASHER_ARGS" ]; then
    echo "Error: ${FLASHER_ARGS} not found, build the project first."
    exit 1
fi

# Lines of "<offset> <encrypted> <file>" for each image, then chip and write-flash args
IMAGES=`python3 -c "
import json
a = json.load(open('$FLASHER_ARGS'))
for v in a.values():
    if isinstance(v, dict) and 'offset' in v:
        print(v['offset'], v['encrypted'], v['file'])
"`
CHIP=`python3 -c "import json; print(json.load(open('$FLASHER_ARGS'))['extra_esptool_args']['chip'])"`
WRITE_FLASH_ARGS=`python3 -c "import json; print(' '.join(json.load(open('$FLASHER_ARGS'))['write_flash_args']))"`

rm -rf "$ENC_DIR"
mkdir -p "$ENC_DIR"

FLASH_FILES=""
for IMAGE in `echo "$IMAGES" | tr ' ' ','`; do
    OFFSET=`echo "$IMAGE" | cut -d, -f1`
    ENCRYPTED=`echo "$IMAGE" | cut -d, -f2`
    FILE=`echo "$IMAGE" | cut -d, -f3`

    if [ "$ENCRYPTED" = "true" ]; then
        OUT="${ENC_DIR}/`basename "$FILE"`"
        espsecure encrypt-flash-data --aes-xts --keyfile "$KEY_FILE" --address "$OFFSET" \
            --output "$OUT" "${BUILD_DIR}/${FILE}"
        FLASH_FILES="${FLASH_FILES} ${OFFSET} ${OUT}"
    else
        FLASH_FILES="${FLASH_FILES} ${OFFSET} ${BUILD_DIR}/${FILE}"
    fi
done

partition_info() {
    python3 "${IDF_PATH}/components/partition_table/parttool.py" --partition-table-file "$PARTITION_TABLE" \
        get_partition_info --partition-name "$1" --info "$2"
}

# NVS encryption is done by nvs_partition_gen, the image is not flash-encrypted
ENCNVS_OFFSET=`partition_info encnvs offset`
python3 "${IDF_PATH}/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py" \
    encrypt "$ENC_CSV" "${ENC_DIR}/encnvs.bin" `partition_info encnvs size` --inputkey "$NVS_KEYS_FILE"
FLASH_FILES="${FLASH_FILES} ${ENCNVS_OFFSET} ${ENC_DIR}/encnvs.bin"

NVS_KEY_OFFSET=`partition_info nvs_key offset`
espsecure encrypt-flash-data --aes-xts --keyfile "$KEY_FILE" --address "$NVS_KEY_OFFSET" \
    --output "${ENC_DIR}/nvs_keys.bin" "$NVS_KEYS_FILE"
FLASH_FILES="${FLASH_FILES} ${NVS_KEY_OFFSET} ${ENC_DIR}/nvs_keys.bin"

esptool --chip "$CHIP" --port "$PORT" write-flash $WRITE_FLASH_ARGS $FLASH_FILES
