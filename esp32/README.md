#### Dev and Release

App can be built in Release or Dev mode. To configure it, run:

```
idf.py menuconfig
```

then select *Project Configuration* and pick desired build type.

#### Configuration files

They use different configuration files that get flashed to 'cfg' partition.

Create `nvs/devconfig.csv` and `nvs/config.csv` files.
See the format these files should have:
https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/storage/nvs_partition_gen.html

For options needed, refer to `config` module's source code.

#### SSL cert

Dev and Release also use different CA files. In release it's letsencrypt's ISRG Root 1,
in Dev it's self-signed locally generated cert, the same which 'proxy' uses.

Create that cert (refer to proxy's README) and create a symbolic link to it in `components/config` directory.
There should be letsencrypt's cert and your local cert sitting next to each other in that directory.
