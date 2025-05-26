#ifndef MQTT_HANDLER_H
#define MQTT_HANDLER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// MQTT handling functions
void connectMQTT();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void sendEspData(float x = NAN, float y = NAN, float heading = NAN, int tileX = INT_MIN, int tileY = INT_MIN, const char* roadTile = nullptr);

#endif // MQTT_HANDLER_H
