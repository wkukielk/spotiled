#### config file
Depending on the value of `DEV` env variable, app uses either `config.yaml` or `devconfig.yaml`.
See `example_config.yaml`.

#### Database

To prefill the database:
- cd into `backend`
```console
    ./scripts/setupdb.sh
```
It'll write the api key to $root/esp32/firmware/nvs/enc.csv, which idf.py flashes to the device.
