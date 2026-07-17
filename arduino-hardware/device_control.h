#ifndef DEVICE_CONTROL_H
#define DEVICE_CONTROL_H

#include "config.h"

int getPinByDeviceId(int deviceId);
void setDeviceState(int deviceId, const char* status);

#endif
