#ifndef SENSOR_UTILS_H
#define SENSOR_UTILS_H

#include <Arduino.h>
#include "DHT.h"
#include "config.h"

float luxCalculate(int analogValue);
int calculateMoisturePercent(int rawValue);
void publishSensorData(DHT& dht, void* mqttClientPtr);

#endif
