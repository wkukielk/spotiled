#### env variables
App requires some secrets to function, e.g. API keys' library hasher's pepper.
Docker compose requires `backend.env` file, create it.
It must contain the following keys, which, for the sake of development, may be set to arbitrary values:

- API_KEYS_PEPPER

Now, each time you start working, before running the program or executing any script, you should
export these variables to your shell:

```console
    . ./export.sh
```

#### Database

To prefill the database:
- cd into `backend`
```console
    ./scripts/setupdb.sh
```
