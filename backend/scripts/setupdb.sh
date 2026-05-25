#!/bin/sh

rm -rf db
mkdir -p db
alembic upgrade head
sqlite3 db/database.db < scripts/preload.sql

KEY=`python3 -m scripts.create_device`
echo 'key,type,encoding,value' > ../esp32/firmware/nvs/enc.csv
echo 'storage,namespace,,' >> ../esp32/firmware/nvs/enc.csv
echo "API_KEY,data,string,${KEY}" >> ../esp32/firmware/nvs/enc.csv
