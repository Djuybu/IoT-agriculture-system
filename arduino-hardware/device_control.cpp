#include "device_control.h"

int getPinByDeviceId(int deviceId) {
  switch(deviceId) {
    case 1: return LED_TEMP;
    case 2: return LED_MIST;
    case 3: return LED_HUMI;
    case 4: return LED_PUMP;
    case 5: return LED_LIGHT;
    default: return -1;
  }
}

void setDeviceState(int deviceId, const char* status) {
  int pin = getPinByDeviceId(deviceId);
  if (pin != -1) {
    bool state = (strcmp(status, "ON") == 0 || strcmp(status, "TURN_ON") == 0);
    digitalWrite(pin, state ? HIGH : LOW);
    Serial.printf("Device %d set to %s (Pin %d)\n", deviceId, status, pin);
  }
}
