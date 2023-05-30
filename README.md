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

## Useful commands

### journalctl

[journalctl](https://www.freedesktop.org/software/systemd/man/journalctl.html) outputs the log entries stored in the journal.

```console
journalctl -u guise -b --output cat -f
```
