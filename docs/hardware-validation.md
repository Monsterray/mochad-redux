# Hardware Validation

Use this checklist to make CM19A and CM15A reports repeatable. The goal is to
capture enough detail for maintainers to tell whether a problem is hardware,
USB passthrough, startup, TCP, RF receive, or shutdown behavior.

## Test Matrix

Run the checks that match the hardware you have:

| Controller | Runtime | Required for v0.4.x |
| --- | --- | --- |
| CM19A | Docker | Yes |
| CM19A | Native foreground | Yes |
| CM19A | Native systemd | Helpful |
| CM15A | Docker | Helpful |
| CM15A | Native foreground | Helpful |
| CM15A | Native systemd | Helpful |

## Record Before Testing

Capture this information before starting:

- `mochad-redux` version or commit SHA
- Host hardware model
- Operating system and version
- CPU architecture
- Kernel version
- Runtime: Docker, foreground command, or systemd service
- Controller model: CM19A or CM15A
- USB device listing for the controller
- Whether any kernel module was blacklisted, especially `ati_remote`

Useful commands:

```sh
uname -a
lsusb
```

If `lsusb` is not installed:

```sh
sudo apt install usbutils
```

## Foreground Startup Test

Run in the foreground so startup logs are visible:

```sh
./mochad -d
```

Expected startup milestones:

```text
[STARTUP] mochad-redux v0.4.0 starting
[STARTUP] TCP configuration bind=0.0.0.0 main=enabled:1099 xml=enabled:1100 openremote=enabled:1101
[USB] initializing libusb
[USB] libusb initialized
[USB] looking for CM15A/CM19A controller
[USB] controller found model=CM19A
[USB] endpoints ready in=0x81 out=0x02
[USB] controller initialized
[USB] transfers started
[STARTUP] signal handlers installed signals=SIGINT,SIGTERM,SIGQUIT
[USB] poll descriptors ready count=...
[TCP] listener ready name=main address=0.0.0.0 port=1099 family=ipv4 dual_stack=not_applicable
[TCP] listener ready name=xml address=0.0.0.0 port=1100 family=ipv4 dual_stack=not_applicable
[TCP] listener ready name=openremote address=0.0.0.0 port=1101 family=ipv4 dual_stack=not_applicable
[TCP] services configured address=0.0.0.0 main=enabled:1099 xml=enabled:1100 openremote=enabled:1101
[STARTUP] mochad is running
```

`Found CM15A` is expected when testing a CM15A.

## TCP Connection Test

In another terminal, connect to port 1099:

```sh
nc localhost 1099
```

If testing from another machine, replace `localhost` with the host IP address.

For IPv6 validation, start `mochad` with an IPv6 bind address:

```sh
./mochad -d --bind ::
```

Then connect over IPv6:

```sh
nc -6 ::1 1099
```

Record whether the log reports `family=ipv6` for all three listeners and
whether `dual_stack=enabled`, `dual_stack=failed`, or `dual_stack=unknown`.
If the host allows dual-stack IPv6 listeners, IPv4 clients may also be able to
connect while bound to `::`. If not, use `--bind 0.0.0.0` for IPv4 or
`--bind ::` for IPv6 and validate the selected mode explicitly.

## RF Receive Test

While `nc` is connected, press buttons on an X10 RF remote.

Expected output looks like:

```text
02/03 19:27:40 Rx RF HouseUnit: K4 Func: On
02/03 19:27:44 Rx RF HouseUnit: K6 Func: Off
02/03 19:27:46 Rx RF House: K Func: Dim
```

Record:

- House code and unit button pressed
- Command pressed: On, Off, Dim, Bright, or other
- Exact `nc` output
- Whether repeated button presses are consistently received

## CM19A RF Transmit to SC546A Chime

This check validates the generic `rf <house><unit> on` transmit path with an
SC546A chime as the receiving load. It does not add or require an
SC546A-specific protocol mode in `mochad-redux`.

Required hardware:

- CM19A connected to the host running `mochad-redux`.
- TM751 or compatible X10 RF transceiver on the same house code.
- SC546A chime set to the test house/unit address.

Example assumes the SC546A is set to `A2`. Adjust the address to match the
physical dials before recording evidence.

Start `mochad-redux` in the foreground:

```sh
./mochad -d
```

In another terminal, connect to the main TCP listener:

```sh
nc localhost 1099
```

Run and record each command:

```text
rf A2 on
rf A2 on
rf A2 off
rf A3 on
rf B2 on
```

Expected physical behavior:

- `rf A2 on`: SC546A chimes.
- Repeated `rf A2 on`: SC546A chimes again for each intentional command.
- `rf A2 off`: ignored by the SC546A.
- `rf A3 on`: ignored when the SC546A unit dial is set to A2.
- `rf B2 on`: ignored when the SC546A house dial is set to A.

Expected daemon behavior:

- The TCP stream may report transmitted RF activity such as
  `Tx RF HouseUnit: A2 Func: On`.
- A successful CM19A USB transmit means the command was handed to the
  controller for RF transmission. It is transmitted, not physically confirmed.
- The daemon must not claim that the SC546A chimed unless the tester records
  that physical observation.

Record:

- CM19A model detection logs.
- TM751 and SC546A house/unit settings.
- Exact TCP commands sent.
- Exact TCP output and daemon logs.
- Physical result for each command: chime heard, ignored, or uncertain.

## Shutdown Test

Stop the foreground process with `Ctrl+C`.

Expected shutdown milestones:

```text
[SHUTDOWN] detaching controller model=CM19A
[SHUTDOWN] requested by SIGINT (2)
[TCP] closing listener name=main fd=...
[TCP] closing listener name=xml fd=...
[TCP] closing listener name=openremote fd=...
[SHUTDOWN] releasing USB resources
[SHUTDOWN] complete
[SHUTDOWN] terminated
```

`detaching CM15A` is expected when testing a CM15A.

## Restart Test

Start `mochad` again after shutdown and repeat the TCP connection test.

The restart passes if:

- The controller is detected again.
- Port 1099 accepts a new `nc` connection.
- RF events are still received.
- No USB claim error appears.

## Docker Notes

When testing Docker, record the exact run or Compose configuration and confirm
the USB device is passed into the container.

The same behavior should be visible through container logs:

```sh
docker logs <container-name>
```

## Reporting Results

For successful hardware validation, open a Hardware validation issue and paste:

- The recorded system and controller details
- Startup logs
- `nc` RF receive output
- Shutdown logs
- Restart result

For failures, open a Bug report and include:

- The smallest reproduction steps
- Logs from startup through failure
- Whether the same hardware works with another runtime
- Whether `ati_remote` or another kernel driver claimed the device
