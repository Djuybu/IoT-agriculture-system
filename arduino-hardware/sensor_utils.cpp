#include "sensor_utils.h"
#include <ArduinoJson.h>
#include <AsyncMqttClient.h>

float luxCalculate(int analogValue) {
  const float VCC = 3.3;
  const float R_PULLDOWN = 10000;
  const float GAMMA = 0.7;
  const float RL10 = 50;

  float voltage = analogValue / 4095.0 * VCC;
  if (voltage >= VCC) voltage = VCC - 0.001;
  float resistance = R_PULLDOWN * (voltage / (VCC - voltage));
  float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));
  return lux;
}

int calculateMoisturePercent(int rawValue) {
  int moisturePercent = map(rawValue, MOISTURE_AIR_VALUE, MOISTURE_WATER_VALUE, 0, 100);
  return constrain(moisturePercent, 0, 100);
}

void publishSensorData(DHT& dht, void* mqttClientPtr) {
  AsyncMqttClient* mqttClient = (AsyncMqttClient*)mqttClientPtr;

  // khi cần return data để kiểm tra và bật tắt thiết bị theo cảnh báo. Ta có thể
  // tạo 1 struct SensorData, sau đó return SensorData này về loop(). Trong loop(), 
  // sẽ gọi các hàm điều khiển thiết bị (nằm trong device_control.h). Phần body
  // của các hàm này nằm trong device_control.cpp

//  int pressure = analogRead(PRESSURE_PIN);
//  doc["pressure"] = pressure;

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float lux = luxCalculate(analogRead(LDRPIN));
  int moisturePercent = calculateMoisturePercent(analogRead(MOISTURE_PIN));

  StaticJsonDocument<200> doc;
  doc["temp"] = isnan(t) ? 0 : t;
  doc["humidity"] = isnan(h) ? 0 : h;
  doc["lux"] = lux;
  doc["moisture"] = moisturePercent;

  char buffer[200];
  serializeJson(doc, buffer);

  if (mqttClient->connected()) {
    mqttClient->publish(MQTT_PUB_DATA, 0, false, buffer);
    Serial.printf("Published: %s\n", buffer);
  }
}
