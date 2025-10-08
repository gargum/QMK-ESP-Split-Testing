/*********************************************************************************
 * ESP-Now-Serial-Bridge
 *
 * ESP8266 based serial bridge for transmitting serial data between two microcontrollers
 *
 * The primary purpose of this sketch is to enable wireless split communication in QMK
 *
 * Range is easily better than regular WiFi, however an external antenna may be
 *   required for truly long range messaging, to combat obstacles/walls, and/or
 *   to achieve success in an area saturated with 2.4GHz traffic.
 *   
 *
 * When uploading the sketch, be sure to define your pulldown pin as appropriate.
 *
 * -- Gareth Gummow - October 2025
 *
 * Based this example - https://github.com/nopnop2002/ESP8266-UART-to-UART-Bridge/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files.
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
*********************************************************************************/

#include <ESP8266WiFi.h>
#include <espnow.h>

#define PULLDOWN     D1 // default has been chosen to work on the WeMos D1 Mini board and the barebones ESP-12F
#define LED_PIN      D2 // default has been chosen to correspond to the builtin LED pin on the WeMos D1 Mini

// Uncomment this line to enable the debug functionality
//#define DEBUG

// new structure for pairing
typedef struct struct_pairing {       
    uint8_t msgType;
    uint8_t espnowChannel; // not used
    uint8_t buffer_length;
    uint8_t buffer[128];
} struct_pairing;

uint8_t PairingAddress[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
enum PairingStatus {NOT_PAIRED, PAIRED, };
enum MessageType {PAIRING_REQUEST, DATA_REQUEST,};
PairingStatus pairingStatus = NOT_PAIRED;
char incomingBuffer[128];
bool incomingPacket = false;
unsigned int lastRequestMillis = 0;

// callback when data is sent from Master to Slave
void OnDataSent(uint8_t *mac_addr, uint8_t status) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
    mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    #ifdef DEBUG
      Serial1.print("Last Packet Sent to: "); Serial1.println(macStr);
      Serial1.print("Last Packet Send Status: "); Serial1.println(status == 0 ? "Delivery Success" : "Delivery Fail");
    #endif
  if (status != 0) {
    #ifdef DEBUG
      Serial1.print("Packet Sent to: ");
      Serial1.print(macStr);
      Serial1.println(" Delivery Fail");
    #endif
    pairingStatus = NOT_PAIRED;
    memset(PairingAddress, 0xFF, sizeof(PairingAddress));
  }
}

// callback when data is recv from Master
void OnDataRecv(uint8_t *mac_addr, uint8_t *data, uint8_t data_len) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
    mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    #ifdef DEBUG
      Serial1.print("Packet Recv from: "); Serial1.println(macStr);
    #endif

  struct_pairing pairingData;
  memcpy(&pairingData, data, data_len);
  if (pairingData.msgType == PAIRING_REQUEST) {
    //Serial1.println("PAIRING_REQUEST");
    int ret;
    ret = esp_now_is_peer_exist(mac_addr);
    #ifdef DEBUG
      Serial1.print("esp_now_is_peer_exist=");
      Serial1.println(ret);
    #endif
    if (ret < 0) {
      #ifdef DEBUG
        Serial1.println("esp_now_is_peer_exist fail");
      #endif
      return;
    }
    if (ret > 0) {
      //Serial1.println("Already paired");
      pairingStatus = PAIRED;
      for (int i=0;i<6;i++) PairingAddress[i] = mac_addr[i];
      return;
    }
    
    ret = esp_now_add_peer(mac_addr, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
    if (ret != 0) {
      #ifdef DEBUG
        Serial1.println("esp_now_add_peer fail");
      #endif
      return;
    } else {
      #ifdef DEBUG
        Serial1.println("esp_now_add_peer succes");
      #endif
      pairingStatus = PAIRED ; 
      for (int i=0;i<6;i++) PairingAddress[i] = mac_addr[i];
    }

  } else {  
      #ifdef DEBUG
        Serial1.println("DATA_REQUEST");
        Serial1.print("buffer_length=");
        Serial1.println(pairingData.buffer_length);
        Serial1.println((char*)pairingData.buffer);
      #endif
    memset(incomingBuffer, 0, sizeof(incomingBuffer));
    incomingPacket = true;
    memcpy(incomingBuffer, pairingData.buffer, pairingData.buffer_length);
  }
}



void setup() {
  delay(1000);
  pinMode(PULLDOWN, OUTPUT);
  digitalWrite(PULLDOWN, LOW);
  #ifdef DEBUG
    Serial1.begin(115200); // for Debug print
  #endif
  Serial.begin(115200); // You can change
  //Set device in STA mode to begin with
  WiFi.mode(WIFI_STA);
  #ifdef DEBUG
    Serial1.println("ESPNow/Basic/Master Example");
  #endif
  // This is the mac address of the Master in Station Mode
  #ifdef DEBUG
    Serial1.print("STA MAC: "); Serial1.println(WiFi.macAddress());
  #endif
  
  // Init ESP-NOW
  if (esp_now_init() != 0) {
    #ifdef DEBUG
      Serial1.println("Error initializing ESP-NOW");
    #endif
    return;
  }

  // Set ESP-NOW Role
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

}

void pairing() {
  if (millis() - lastRequestMillis < 1000) return;

  #ifdef DEBUG
    Serial1.print("pairingStatus=");
    Serial1.print(pairingStatus);
    Serial1.print(" PairingAddress=");
  #endif
  for (int i=0;i<sizeof(PairingAddress);i++) {
    #ifdef DEBUG
      Serial1.print(PairingAddress[i], HEX);
      Serial1.print(" ");
    #endif
  }
  #ifdef DEBUG
    Serial1.println();
  #endif

  struct_pairing pairingData;
  pairingData.msgType = PAIRING_REQUEST;
  esp_now_send(PairingAddress, (uint8_t *) &pairingData, sizeof(pairingData));
  lastRequestMillis = millis();
}

void loop() {
  // Update pairing
  pairing();

  if(Serial.available()) {
    uint8_t buf[128];
    uint8_t iofs=0;
    while(Serial.available()) {
      buf[iofs++] = Serial.read(); // read char from UART
      buf[iofs] = 0;
    }
    
    Serial.print((char *)buf);
    if (pairingStatus == PAIRED) {
      struct_pairing pairingData;
      pairingData.msgType = DATA_REQUEST;
      memset(pairingData.buffer, 0, sizeof(pairingData.buffer));
      memcpy(pairingData.buffer, buf, iofs);
      pairingData.buffer_length = iofs;    
      esp_now_send(PairingAddress, (uint8_t *) &pairingData, sizeof(pairingData));     
    } // end if
  }

  if (incomingPacket) {
    #ifdef DEBUG
      Serial1.print("<--[");
      Serial1.print(incomingBuffer);
      Serial1.println("]");
    #endif
    Serial.print(incomingBuffer);
    incomingPacket = false;
  }

}
