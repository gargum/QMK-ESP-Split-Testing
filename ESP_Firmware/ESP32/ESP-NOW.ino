/*********************************************************************************
 * ESP-Now-Serial-Bridge
 *
 * ESP32 based serial bridge for transmitting serial data between two microcontrollers
 *
 * The primary purpose of this sketch is to enable wireless split communication in QMK
 *
 * Range is easily better than regular WiFi, however an external antenna may be
 *   required for truly long range messaging, to combat obstacles/walls, and/or
 *   to achieve success in an area saturated with 2.4GHz traffic.
 *   
 * The BLINK_ON_* macros should be somewhat self-explanatory.  If your board has a built-in
 *   LED (or you choose to wire an external one), it can indicate ESP-Now activity as
 *   defined by the macros you choose to enable.
 *
 * When uploading the sketch, be sure to define LEFT or RIGHT as appropriate
 *   before compiling.
 *
 * -- Gareth Gummow - October 2025
 *
 * Based this example - https://github.com/yuri-rage/ESP-Now-Serial-Bridge
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files.
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
*********************************************************************************/


#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

#define LEFT // LEFT or RIGHT

// Remember to define your MAC addresses appropriately

#ifdef LEFT
  #define RECVR_MAC {0x20, 0x6E, 0xF1, 0x69, 0xD7, 0x5C} //This should be set to RIGHT's MAC Address  
#else
  #define RECVR_MAC {0x20, 0x6E, 0xF1, 0x69, 0xE5, 0x48} //This should be set to LEFT's MAC Address
#endif

#define WIFI_CHAN  9 // 12-13 only legal in US in lower power mode, do not use 14 outside Japan
#define BAUD_RATE  115200
#define TX_PIN     21 // default is pin 1 on NodeMCU footprint and pin 21 on Xiao footprint
#define RX_PIN     20 // default is pin 3 on NodeMCU footprint and pin 20 on Xiao footprint
#define PULLDOWN   3 // default has been chosen to work on both NodeMCU and Xiao footprints
#define SER_PARAMS SERIAL_8N1 // SERIAL_8N1: start/stop bits, no parity

#define BUFFER_SIZE 250 // max of 250 bytes is supported by ESP-NOW
#define BLINK_TIME  10

//#define DEBUG // for additional serial messages (may interfere with other messages)
//#define LED_ACTIVE_LOW
//#define BLINK_ON_SEND
//#define BLINK_ON_SEND_SUCCESS
//#define BLINK_ON_RECV

const uint8_t broadcastAddress[] = RECVR_MAC;

// wait for double the time between bytes at this serial baud rate before sending a packet
// this *should* allow for complete packet forming when using packetized serial comms
const uint32_t timeout_micros = (int)(1.0 / BAUD_RATE * 1E6) * 20;

uint8_t buf_recv[BUFFER_SIZE];
uint8_t buf_send[BUFFER_SIZE];
uint8_t buf_size = 0;
uint32_t send_timeout = 0;

esp_now_peer_info_t peerInfo;  // scope workaround for arduino-esp32 v2.0.1

#if defined(DEBUG) || defined(BLINK_ON_SEND_SUCCESS) || defined(BLINK_ON_SEND)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  #ifdef DEBUG
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Send success");
  } else {
  Serial.println("Send failed");
  }
  #endif
  #ifdef BLINK_ON_SEND
  BlinkLED();
  #endif
  #ifdef BLINK_ON_SEND_SUCCESS
  if (status == ESP_NOW_SEND_SUCCESS) {
    BlinkLED();
    return;
  }
  #endif
}
#endif

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  #ifdef BLINK_ON_RECV
  BlinkLED();
  #endif
  memcpy(&buf_recv, incomingData, sizeof(buf_recv));
  Serial.write(buf_recv, len);
  #ifdef DEBUG
  Serial.print("\n Bytes received: ");
  Serial.println(len);
  #endif
}

#if defined(BLINK_ON_SEND) || defined(BLINK_ON_SEND_SUCCESS) || defined(BLINK_ON_RECV)
void BlinkLED() {
  #ifdef LED_ACTIVE_LOW
    digitalWrite(LED_BUILTIN, LOW);
    delay(BLINK_TIME);
    digitalWrite(LED_BUILTIN, HIGH);
  #else
    digitalWrite(LED_BUILTIN, HIGH);
    delay(BLINK_TIME);
    digitalWrite(LED_BUILTIN, LOW);
  #endif
}
#endif
 
void setup() {
  pinMode(PULLDOWN, OUTPUT);
  digitalWrite(PULLDOWN, LOW);
  #if defined(BLINK_ON_SEND) || defined(BLINK_ON_SEND_SUCCESS) || defined(BLINK_ON_RECV)
    pinMode(LED_BUILTIN, OUTPUT);
  #endif
  Serial.begin(BAUD_RATE, SER_PARAMS, RX_PIN, TX_PIN);
  Serial.println(send_timeout);
  WiFi.mode(WIFI_STA);

  #ifdef DEBUG
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.print("ESP32 MAC Address: ");
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
  #endif
  
  if (esp_wifi_set_channel(WIFI_CHAN, WIFI_SECOND_CHAN_NONE) != ESP_OK) {
    #ifdef DEBUG
    Serial.println("Error changing WiFi channel");
    #endif
    return;
  }

  if (esp_now_init() != ESP_OK) {
    #ifdef DEBUG
    Serial.println("Error initializing ESP-NOW");
    #endif
    return;
  }

  #if defined(DEBUG) || defined(BLINK_ON_SEND_SUCCESS)
  esp_now_register_send_cb(OnDataSent);
  #endif
  
  // esp_now_peer_info_t peerInfo;  // scope workaround for arduino-esp32 v2.0.1
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = WIFI_CHAN;  
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    #ifdef DEBUG
    Serial.println("Failed to add peer");
    #endif
    return;
  }

  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}

void loop() {

  // read up to BUFFER_SIZE from serial port
  if (Serial.available()) {
    while (Serial.available() && buf_size < BUFFER_SIZE) {
      buf_send[buf_size] = Serial.read();
      send_timeout = micros() + timeout_micros;
      buf_size++;
    }
  }

  // send buffer contents when full or timeout has elapsed
  if (buf_size == BUFFER_SIZE || (buf_size > 0 && micros() >= send_timeout)) {
    #ifdef BLINK_ON_SEND
    BlinkLED();
    #endif
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &buf_send, buf_size);
    buf_size = 0;
    #ifdef DEBUG
    if (result == ESP_OK) {
      Serial.println("Sent!");
    }
    else {
      Serial.println("Send error");
    }
    #endif
  }

}
