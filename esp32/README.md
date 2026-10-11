#### Dev and Release

App can be built in Release or Dev mode. To configure it, run:

```
idf.py menuconfig
```

then select *Project Configuration* and pick desired build type.
That command requires (stub) `firmware/nvs/config.csv` to be present.
Refer to the next section.

#### Configuration files

Dev and release use different configuration files that get flashed to 'cfg' nvs partition.

Create `firmware/nvs/devconfig.csv` and `firmware/nvs/config.csv` files.
See the format these files should have:
https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/storage/nvs_partition_gen.html

For options needed, refer to `firmware/nvs/example_config.csv` or `config` module's source code.

#### SSL cert

Dev and Release also use different CA files. In release it's ISRG Root 2,
in Dev it's self-signed locally generated cert, the same which 'proxy' uses.

Create that cert (refer to proxy's README) and create a symbolic link to it in `components/config` directory.
There should be ISRG cert and your local cert sitting next to each other in that directory.

#### Encrypted nvs

For now, there is no flash enryption in Dev nor Release build, but "encnvs" partition is ultimately
supposed to be encrypted because it contains e.g. and api key (see `firmware/partitions.csv`).

Similarly to "cfg" partition, "encnvs" is populated based on `firmware/nvs/encnvs.csv`. This file can
be generated automatically, refer to backend's README.

#### Compile

```
idf.py set-target esp32s3
idf.py build
```
