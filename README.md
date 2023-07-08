# Guise - User Session Daemon

## Install

Extract the release zip containing the `guised` linux executable and `guise.service` [systemd unit file](https://www.freedesktop.org/software/systemd/man/systemd.unit.html).

```console
sudo chmod a+x guised
sudo cp guised /usr/local/sbin/
sudo cp guise.service /usr/local/lib/systemd/system/
```

* Make systemd reload all .service-files

```console
systemctl daemon-reload
```

* Check that systemd has read the .service-file:

```console
systemctl status guise --output cat
```

It should report something similar to:

```console
○ guise.service - Conclave UDP Relay
     Loaded: loaded (/usr/local/lib/systemd/system/guise.service; disabled; preset: disabled)
     Active: inactive (dead)
```

* Make sure it is started, if machine is rebooted in the future:

```console
systemctl enable guise
```

* Start it now:

```console
systemctl start guise
```

## Users.txt

Must be a `users.txt` in the current directory when the daemon starts. One line for each user with the format:

```txt
ID NAME PASSWORD_HASH ROLES
```

* **`ID`**. Integer in base 10 that can fit inside an unsigned 64 bit, minimum of one character and not zero. (example `936597104`). Any number works as long as it is unique within the file. You can generate a number from:

    ```console
    od -vAn -N8 -tu8 < /dev/urandom
    ```

* **`NAME`**. up to 32 characters of an utf8 encoded string.
* **`PASSWORD_HASH`**. Secret private password hash, must be exactly 16 lower case hexadecimal characters (example `9a55f43afb476c91`). It is recommended to generate it using SHA-256:

    ```console
    shasum -a 256 <<< "MYPASSWORD" | cut -c -16
    ```

* **`RULES`**. Authorized rules:
  * `-`: no role
  * `q`: query
  * `l`: listen

### Example `users.txt`:

```txt
12009295865568971531 YourUsername 9a55f43afb476c91 ql
```


## Useful commands

### journalctl

[journalctl](https://www.freedesktop.org/software/systemd/man/journalctl.html) outputs the log entries stored in the journal.

```console
journalctl -u guise -b --output cat -f
```
