#!/bin/sh


if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <config_file>"
    exit 1
fi

CONFIG_FILE=$1
ENC_FILE=$2

read_config_key() {
    FILE=$1
    KEY=$2
    awk -F, -v k="$KEY" '$1 == k {print $4}' "$FILE"
}

BACKEND_URL=`read_config_key $CONFIG_FILE 'BACKEND_URL'`
API_KEY=`read_config_key $ENC_FILE 'API_KEY'`

URL="$BACKEND_URL/api/esp/firmware/esp32s3/latest"

curl -k -v --output spotiled.bin \
     -X GET "$URL" \
     -H "Authorization: Bearer $API_KEY" \
     -H "Owned-Version: 0.9"
