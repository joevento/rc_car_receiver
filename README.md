# ESP32 Bluetooth Receiver for RC Car OBC

Simple ESP32 firmware that acts as a Bluetooth Classic bridge between a host computer and the ESP32 RC Car OBC. Receives keyboard commands over USB serial and transmits motor control packets to the OBC via Bluetooth SPP.

## Features

- Bluetooth Classic SPP client connecting to the OBC
- USB serial interface for host control (115200 baud)
- Simple keyboard-based motor control
- Automatic reconnection on disconnect
- Bidirectional data relay (commands to OBC, data from OBC to host)

## Hardware Requirements

- ESP32 development board with USB serial
- USB cable for serial connection to host computer

## Command Protocol

Commands are single-character ASCII sent over serial:

- `v`: Backward (both motors reverse, full speed)
- `w`: Forward (both motors forward, full speed)
- `x`: Spin right (motor A reverse, motor B forward)
- `y`: Spin left (motor A forward, motor B reverse)
- `z`: Stop (both motors off)

Reason for such weird command characters is that WASD ascii characters might be present in the lidar data (sensor angle) which is being dumped into the same serial, leading to wrong commands being sent to the obc.

## Motor Command Structure

```cpp
struct motor_command_t {
  uint8_t motor_a_speed;    // 0-255 PWM duty
  bool    motor_a_direction; // true=forward, false=reverse
  uint8_t motor_b_speed;
  bool    motor_b_direction;
};
```

Packed struct sent as raw bytes over Bluetooth to OBC.

## Configuration

Update the OBC Bluetooth MAC address in the code:

```cpp
const char* OBC_BT_ADDRESS = "e8:db:84:04:52:d6";
```

Find your OBC's MAC address from its serial output during initialization.

## Build and Flash

1. Install Arduino IDE with ESP32 board support
2. Install BluetoothSerial library (included with ESP32 core)
3. Select board: ESP32 Dev Module
4. Set upload speed: 921600
5. Flash to ESP32

## Usage

1. Power on OBC ESP32 (it will start advertising as "ESP32_Car")
2. Power on receiver ESP32
3. Open serial monitor at 115200 baud
4. Wait for "Connected to OBC!" message
5. Send control commands (v/w/x/y/z), our project had a visualisation and control software developed seperately which mapped WASD to vwxy and would display the lidar data on screen

Monitor will show connection status and relay any data from OBC (LiDAR frames, CRC16-CCITT, etc.).

## Connection Flow

1. Receiver initializes Bluetooth in master mode
2. Attempts connection to hardcoded MAC address, for whatever reason connection using the BT name would not work
3. On success, sets `bt_connected = true`
4. On disconnect, enters reconnection loop (2-second intervals)
5. Commands only sent when connection active

## Pin Assignments

None required beyond USB serial (GPIO 1/3 typically used by USB-UART bridge).

## Data Relay

Any data received from OBC via Bluetooth is forwarded to USB serial for host processing. This allows viewing LiDAR frames or debug output from the OBC.

## Typical Use Case

```
Host PC ──USB Serial──> Receiver ESP32 ──Bluetooth SPP──> OBC ESP32
         (keyboard)                    (motor commands)

Host PC <──USB Serial── Receiver ESP32 <──Bluetooth SPP── OBC ESP32
         (LiDAR data)                   (sensor data)
```

## Troubleshooting

- "Failed to connect": Check OBC MAC address and ensure OBC is powered on
- No response to commands: Verify `bt_connected` is true, check serial monitor
- Bluetooth pairing issues: Clear paired devices on both ESP32s, power cycle
- Serial not working: Check baud rate (115200), ensure correct USB port selected

## Limitations

- Hardcoded MAC address (update in code for different OBC)
- No command queueing (rapid keypresses may drop commands)
- 10 ms loop delay adds minor latency
- Fixed speed values (255 = full speed)

## Extending

- Add variable speed control
- Implement command history/macros

## Compatible With

This receiver is designed to work with the ESP32 RC Car OBC firmware (link to other repo)

## License

MIT
