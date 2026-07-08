# Soft Robotic Exoglove for Hand Rehabilitation
Arduino sketch that connects a WiFiNINA-based board to a WiFi network and an MQTT broker, then publishes periodic test messages and (optionally) channel setpoint commands.

## Overview

This sketch is the embedded counterpart to a Python MQTT publisher used for testing communication with an MQTT broker (`rasticvm.internal`). It:

1. Connects to WiFi using credentials stored outside version control (`arduino_secrets.h`).
2. Connects to an MQTT broker over port `1883`.
3. Publishes a heartbeat/test message every 3 seconds to `racuum/racuum-ACE9/test`.
4. Provides a `setChannelValues()` function to publish channel setpoint commands to `racuum/racuum-ACE9/setpoint` (currently commented out in `loop()`).

## Hardware Requirements

- An Arduino board with a **WiFiNINA-compatible** WiFi module, e.g.:
  - Arduino MKR WiFi 1010
  - Arduino Nano 33 IoT
  - Arduino Uno WiFi Rev2
- **2.4GHz WiFi network** (WiFiNINA modules do not support 5GHz)

## Dependencies

Install these via **Arduino IDE → Sketch → Include Library → Manage Libraries**:

| Library | Purpose |
|---|---|
| `WiFiNINA` | WiFi connectivity for NINA-based boards |
| `MQTT` (by 256dpi / "arduino-mqtt") | MQTT client (`MQTTClient` class) |
| `SPI` | Required by WiFiNINA internally (bundled with Arduino core) |

> **Note:** There are several Arduino MQTT libraries with similar names. This sketch uses **`MQTTClient`** from the `arduino-mqtt` library by 256dpi — not `PubSubClient` and not the official `ArduinoMqttClient` (`MqttClient`). Their APIs are not interchangeable.

## Project Structure
connecter/
├── connecter.ino          # Main sketch
├── arduino_secrets.h       # WiFi credentials (gitignored — not included in repo)
└── README.md

## Setup

### 1. Create `arduino_secrets.h`

This file is **not committed to version control** and must be created locally in the same directory as `connecter.ino`:

```cpp
#define SECRET_SSID "yourNetworkName"
#define SECRET_PASS "yourNetworkPassword"
```

### 2. Configure MQTT settings

Edit the constants at the top of `connecter.ino` as needed:

```cpp
const char* BROKER = "rasticvm.internal";
const int   PORT   = 1883;
const char* COMMAND_TOPIC = "racuum/racuum-ACE9/setpoint";
const char* TEST_TOPIC    = "racuum/racuum-ACE9/test";
const char* CLIENT_ID     = "berenakpinar";
```

> **DNS note:** `rasticvm.internal` is a hostname, not an IP address. The Arduino must be on a network whose DHCP-assigned DNS server can resolve `.internal` names. If the board fails to connect to the broker (but WiFi connects fine), try replacing `BROKER` with the broker's raw IP address as a diagnostic step.

### 3. Upload

Select the correct board and port under **Tools**, then upload as normal. Open the Serial Monitor at **9600 baud** to view connection logs.

## Behavior

### WiFi connection (`connectToWIFI()`)

Repeatedly calls `WiFi.begin(ssid, pass)` until `wifiStatus == WL_CONNECTED`, then prints the assigned IP address.

### MQTT connection (`connectToMQTT()`)

Calls `client.begin(BROKER, PORT, wifiClient)` then loops on `client.connect(CLIENT_ID)` until successful.

### Main loop

Every `PUBLISH_INTERVAL` (3000 ms), publishes a JSON heartbeat to `TEST_TOPIC`:

```json
{"test": "ping", "count": <msgCount>, "time": <millis()>}
```

at QoS 1. If the MQTT connection drops, `loop()` re-runs `connectToWIFI()` and `connectToMQTT()` before continuing.

### Channel setpoint commands (`setChannelValues()`)

Publishes to `COMMAND_TOPIC` in the form:

```json
{"channels": {"<channelID>": <pressureCapacity>}}
```

Currently invoked only from commented-out lines in `loop()`:

```cpp
// setChannelValues("1", 95);
// delay(3000);
// setChannelValues("1", 0);
// delay(3000);
```

Uncomment to enable actual channel actuation alongside the heartbeat.

## Known Issues / TODO

- [ ] **Compile-breaking bug:** the sketch currently contains duplicate, self-referential declarations:
```cpp
  const char ssid[] = ssid   // invalid — remove this line
  const char pass[] = pass;  // invalid — remove this line
```
  These must be deleted; `ssid`/`pass` are already declared correctly via `SECRET_SSID`/`SECRET_PASS` above them.
- [ ] `setChannelValues()` formats `channelID` (a `String`) using `%d` in `snprintf`, which is incorrect and will not produce the intended JSON. Should use `%s` with `channelID.c_str()`.
- [ ] `setup()` calls `connectToWIFI()` / `connectToMQTT()` inside the `while (!Serial)` loop, which only makes sense if a serial monitor is expected to be attached before connecting. On battery/standalone deployment (no USB serial monitor), this loop may never exit, and WiFi/MQTT will not be attempted. Consider moving these calls outside the `while (!Serial)` block.
- [ ] `connectToWIFI()` has no `delay()` between retry attempts, unlike `connectToMQTT()`. Consider adding a short delay to avoid hammering `WiFi.begin()`.
- [ ] No QoS/retain configuration exposed via constants — currently hardcoded per call site (`false, 1`).

## Security Notes

- `arduino_secrets.h` must **never** be committed. Confirm it's listed in `.gitignore`.
- MQTT broker connection is unauthenticated and unencrypted (plain `WiFiClient` on port 1883, no username/password, no TLS). For anything beyond local/lab testing, consider:
  - Enabling `client.connect(CLIENT_ID, username, password)` on the broker
  - Using `WiFiSSLClient` with TLS on port 8883 if the broker supports it

## Reference: Python Equivalent

This sketch mirrors the behavior of a companion Python script (`paho-mqtt`, Python 3.12) used for local testing against the same broker and topics. Message formats (JSON payloads, topic names) are kept identical between both implementations to simplify cross-testing.
