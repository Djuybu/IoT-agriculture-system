#include <ArduinoJson.h>
#include "DHT.h"
#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <HTTPClient.h>

extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}

// Include custom headers
#include "config.h"
#include "device_control.h"
#include "sensor_utils.h"

// --- GLOBALS ---
DHT dht(DHTPIN, DHTTYPE);
AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

unsigned long lastSensorRead = 0;

// --- PROTOTYPES ---
void connectToWifi();
void connectToMqtt();
void syncDevicesOnStartUp();
void WiFiEvent(WiFiEvent_t event);
void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void sendAck(const int deviceId, const int actionId, const boolean success);
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);


// --- MQTT & WIFI CALLBACKS ---

void connectToWifi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void WiFiEvent(WiFiEvent_t event) {
  switch(event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("WiFi connected. IP: ");
      Serial.println(WiFi.localIP());
      connectToMqtt();
      syncDevicesOnStartUp();
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      xTimerStop(mqttReconnectTimer, 0);
      xTimerStart(wifiReconnectTimer, 0);
      break;
  }
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT. Subscribing...");
  mqttClient.subscribe(MQTT_TOPIC_CONTROL, 0);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

void sendAck(const int deviceId, const int actionId, const boolean success) {
  StaticJsonDocument<128> ackDoc;
  ackDoc["deviceId"] = deviceId;
  ackDoc["actionId"] = actionId;
  ackDoc["status"] = success ? "SUCCESS" : "FAILED";

  char ackBuffer[128];
  serializeJson(ackDoc, ackBuffer);
  mqttClient.publish(MQTT_TOPIC_ACK, 0, false, ackBuffer);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  char message[len + 1];
  memcpy(message, payload, len);
  message[len] = '\0';

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);
  if (error) {
    Serial.print("JSON parse failed: ");
    Serial.println(error.f_str());
    return;
  }

  const int actionId = doc["actionId"];
  const int deviceId = doc["deviceId"];
  const char* action = doc["action"];

  if (!action) return;

  bool success = true;
  setDeviceState(deviceId, action); // Use the centralized function

  sendAck(deviceId, actionId, success);
}

// --- STARTUP SYNC ---

void syncDevicesOnStartUp() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = String(BACKEND_BASE_URL) + DEVICES_STATUS_ENDPOINT;

  Serial.printf("Syncing devices from %s\n", url.c_str());
  http.begin(url);

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      JsonArray devices = doc["data"].as<JsonArray>();
      for (JsonObject device : devices) {
        setDeviceState(device["id"], device["status"]);
      }
    } else {
      Serial.print("JSON parse error during sync: ");
      Serial.println(error.f_str());
    }
  } else {
    Serial.printf("Sync failed, HTTP error: %d\n", httpCode);
  }
  http.end();
}

// --- MAIN ---

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(LED_TEMP, OUTPUT);
  pinMode(LED_HUMI, OUTPUT);
  pinMode(LED_LIGHT, OUTPUT);
  pinMode(LED_PUMP, OUTPUT);
  pinMode(LED_MIST, OUTPUT);

  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, (TimerCallbackFunction_t)connectToMqtt);
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, (TimerCallbackFunction_t)connectToWifi);

  WiFi.onEvent(WiFiEvent);
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials("myuser", "123456");

  connectToWifi();
}

void loop() {
  if (millis() - lastSensorRead >= SENSOR_INTERVAL) {
    lastSensorRead = millis();
    publishSensorData(dht, &mqttClient); // Pass dht and mqttClient by reference/pointer
  }
}
