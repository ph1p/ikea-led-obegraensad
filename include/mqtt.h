#pragma once

#include "constants.h"

#ifdef ENABLE_MQTT

// MQTT client with Home Assistant discovery. Everything runs from loop(), the
// broker settings come from the config (Web GUI → Settings).
void initMqtt();
void mqttLoop();
// call after the MQTT settings changed, reconnects with the new values
void mqttReconfigure();
bool mqttIsConnected();

#endif
