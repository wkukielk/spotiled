#!/bin/sh

rm -rf db
mkdir -p db
alembic upgrade head
python3 -m scripts.create_firmware 1_0.bin 1.0

KEY=`python3 -m scripts.create_device`
echo 'key,type,encoding,value' > ../esp32/firmware/nvs/enc.csv
echo 'storage,namespace,,' >> ../esp32/firmware/nvs/enc.csv
echo "API_KEY,data,string,${KEY}" >> ../esp32/firmware/nvs/enc.csv
