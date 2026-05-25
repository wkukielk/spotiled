#### env variables
App requires some secrets to function, e.g. API keys' library hasher's pepper.

Now, each time you start working, before running the program or executing any script, you should
export these variables to your shell:

```console
    . ./export.sh
```

You can control the configuration of the app by modifying `DEV = ON | OFF` variable.

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
