#### config file
Depending on the value of `DEV` env variable, app uses either `config.yaml` or `devconfig.yaml`.
See `example_config.yaml`.

#### env

Modify `backend.env` appropriately. `APP_UID` and `APP_GID` are for app user in docker container.
They should be the same as those which `firmware_root_folder` owner has.

#### Database

To prefill the database:
- cd into `backend`
```console
    ./scripts/setupdb.sh
```
It'll write the api key to $root/esp32/firmware/nvs/enc.csv, which idf.py flashes to the device.
