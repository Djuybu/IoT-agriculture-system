#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <WiFi.h>

// --- PINS ---
#define DHTPIN 4
#define DHTTYPE DHT11
#define LDRPIN 35
#define MOISTURE_PIN 34

#define LED_MIST 5
#define LED_PUMP 18
#define LED_TEMP 19
#define LED_HUMI 21
#define LED_LIGHT 22

//#define PRESSURE_PIN 33

// --- WIFI & MQTT ---
#define MQTT_PORT 1884

// #define WIFI_SSID "Hoang Van 2.4G"
// #define WIFI_PASSWORD "vietanh2004"
// #define MQTT_HOST IPAddress(192,168,1,5)

#define WIFI_SSID "GVanh"
#define WIFI_PASSWORD "vanh2004"
#define MQTT_HOST IPAddress(10,151,183,143)


// --- API / BACKEND ---
#define BACKEND_BASE_URL "http://10.151.183.143:8081"
#define DEVICES_STATUS_ENDPOINT "/api/devices/status"

// --- MQTT TOPICS ---
#define ROOM_ID "room100"
#define MQTT_PUB_DATA "home/" ROOM_ID "/data"
#define MQTT_TOPIC_CONTROL "home/" ROOM_ID "/control"
#define MQTT_TOPIC_ACK "home/" ROOM_ID "/ack"

// --- SENSOR CALIBRATION ---
const int MOISTURE_AIR_VALUE = 3200;
const int MOISTURE_WATER_VALUE = 1500;

// --- TIMING ---
const unsigned long SENSOR_INTERVAL = 2000;

#endif
