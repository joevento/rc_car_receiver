#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

const char* OBC_BT_NAME = "ESP32_Car";
bool bt_connected = false;

struct motor_command_t {
  uint8_t motor_a_speed;
  bool    motor_a_direction;
  uint8_t motor_b_speed;
  bool    motor_b_direction;
};

struct rf_frame_t {
  uint8_t  magic;
  uint8_t  version;
  uint8_t  scan_id;
  uint8_t  fragment_id;
  uint8_t  payload_len;
  uint16_t crc;
  uint8_t  payload[25];
} __attribute__((packed));

motor_command_t nextCmd;

void btCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_OPEN_EVT) {
    Serial.println("Connected to OBC!");
    bt_connected = true;
  } else if (event == ESP_SPP_CLOSE_EVT) {
    Serial.println("Disconnected from OBC!");
    bt_connected = false;
  }
}

void setup() {
  Serial.begin(1000000);
  
  SerialBT.register_callback(btCallback);
  
  if (!SerialBT.begin("ESP32_Receiver", true)) {
    Serial.println("Bluetooth init failed!");
    while(1);
  }
  
  Serial.println("Bluetooth initialized. Connecting to OBC...");
  
  nextCmd = {0, true, 0, true};
  
  connectToOBC();
}

const char* OBC_BT_ADDRESS = "e8:db:84:04:52:d6"; // Your server's MAC from logs

void connectToOBC() {
  Serial.print("Connecting to ");
  Serial.println(OBC_BT_ADDRESS);
  
  uint8_t addr[6];
  sscanf(OBC_BT_ADDRESS, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
         &addr[0], &addr[1], &addr[2], &addr[3], &addr[4], &addr[5]);
  
  if (SerialBT.connect(addr)) {
    Serial.println("Connected!");
    bt_connected = true;
  } else {
    Serial.println("Failed!");
    bt_connected = false;
  }
}

void loop() {
  if (!bt_connected) {
    Serial.println("Attempting to reconnect...");
    connectToOBC();
    delay(2000);
    return;
  }
  
  if (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case 'v':
        nextCmd.motor_a_speed = 255;
        nextCmd.motor_b_speed = 255;
        nextCmd.motor_a_direction = true;
        nextCmd.motor_b_direction = true;
        break;
      case 'w':
        nextCmd.motor_a_speed = 255;
        nextCmd.motor_b_speed = 255;
        nextCmd.motor_a_direction = false;
        nextCmd.motor_b_direction = false;
        break;
      case 'x':
        nextCmd.motor_a_speed = 255;
        nextCmd.motor_b_speed = 255;
        nextCmd.motor_a_direction = false;
        nextCmd.motor_b_direction = true;
        break;
      case 'y':
        nextCmd.motor_a_speed = 255;
        nextCmd.motor_b_speed = 255;
        nextCmd.motor_a_direction = true;
        nextCmd.motor_b_direction = false;
        break;
      case 'z':
        nextCmd.motor_a_speed = 0;
        nextCmd.motor_b_speed = 0;
        nextCmd.motor_a_direction = false;
        nextCmd.motor_b_direction = false;
        break;
      default:
        break;
    }
    
    if (bt_connected && SerialBT.connected()) {
      SerialBT.write((uint8_t*)&nextCmd, sizeof(nextCmd));
    }
  }

  if (SerialBT.available()) {
    uint8_t buffer[32];
    int len = SerialBT.readBytes(buffer, sizeof(buffer));
    if (len > 0) {
      Serial.write(buffer, len);
    }
  }
  
  delay(10);
}