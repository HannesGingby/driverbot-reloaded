#include <WiFi.h>
#include "mqtt_handler.h"
#include "../motor_control/motor_control.h"  // For motor control functions

#include "../credentials.h"

#include "../uart/uart_handler.h"

#include <cmath>  // For NAN
#include <limits.h> // For INT_MIN

// The MQTT client is defined in main.cpp
extern PubSubClient client;

// Global JSON fields defined in main.cpp
extern int driveDirection;
extern int steerDirection;
extern int driveSpeedPercentage;
extern int steerSpeedPercentage;

extern String taskCommand;
extern int startPosX;
extern int startPosY;
extern int mapSizeX;
extern int mapSizeY;


void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.println("] ");

    if (strcmp(topic, "movement") == 0) {
        DynamicJsonDocument doc(512);
        deserializeJson(doc, payload, length);

        if (doc.containsKey("driveDirection")) {
            driveDirection = doc["driveDirection"].as<int>();
        } else {
            driveDirection = 0;  // Default value
        }

        if (doc.containsKey("steerDirection")) {
            steerDirection = doc["steerDirection"].as<int>();
        } else {
            steerDirection = 0;
        }

        if (doc.containsKey("driveSpeed")) {
            driveSpeedPercentage = doc["driveSpeed"].as<int>();
        } else {
            driveSpeedPercentage = 100;
        }

        if (doc.containsKey("steerSpeed")) {
            steerSpeedPercentage = doc["steerSpeed"].as<int>();
        } else {
            steerSpeedPercentage = 50;
        }

        Serial.print("Received JSON: ");
        serializeJson(doc, Serial);
        Serial.println();

        int driveSpeed = (driveSpeedPercentage * maxMotorSpeed) / 100;
        int steerSpeed = (steerSpeedPercentage * maxMotorSpeed) / 100;

        if (driveDirection == 1) {
            drive(driveSpeed, true);    // Forward
        } else if (driveDirection == -1) {
            drive(driveSpeed, false);   // Backward
        }

        if (steerDirection == 1) {
            steer(steerSpeed, steerSpeed, true);  // Right
        } else if (steerDirection == -1) {
            steer(steerSpeed, steerSpeed, false);  // Left
        }

        if (driveDirection == 0 && steerDirection == 0) {
            stopMotors();   // Stop
        }
    // } else if (strcmp(topic, "ping") == 0) {
    //     Serial.println("Ping message received, sending response");
    //     client.publish("ping", "1");
    } else if (strcmp(topic, "task") == 0) {
        Serial.println("Task message received");

        // Parse the json task payload
        DynamicJsonDocument doc(512);
        deserializeJson(doc, payload, length);

        if (doc.containsKey("command")) {
            taskCommand = doc["command"].as<String>();
        }

        if (doc.containsKey("startPosX")) {
            startPosX = doc["startPosX"].as<int>();
        }

        if (doc.containsKey("startPosY")) {
            startPosY = doc["startPosY"].as<int>();
        }

        if (doc.containsKey("mapSizeX")) {
            mapSizeX = doc["mapSizeX"].as<int>();
        }

        if (doc.containsKey("mapSizeY")) {
            mapSizeY = doc["mapSizeY"].as<int>();
        }

        sendCommand(taskCommand, startPosX, startPosY, mapSizeX, mapSizeY);
    }
}


void connectMQTT() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection…");

        // Resolve broker hostname first
        IPAddress brokerIP;
        if (WiFi.hostByName(mqttServer, brokerIP)) {
            Serial.print("Resolved broker IP: ");
            Serial.println(brokerIP);
        } else {
            Serial.println("Failed to resolve broker hostname. Retrying in 5 seconds...");
            delay(5000);
            continue;
        }

        extern String espClientId;
        if (client.connect(espClientId.c_str(), mqttUsername, mqttPassword)) {
            Serial.println("Connected to MQTT broker");
            client.subscribe("movement");
            client.subscribe("ping");
            client.subscribe("task");
            client.subscribe("esp");
        } else {
            Serial.print("failed, rc = ");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}


void sendEspData(float x, float y, float heading, int tileX, int tileY, const String roadTile) {
    DynamicJsonDocument doc(256);

    // Add optional position data if provided
    if (!isnan(x)) doc["x"] = x;
    if (!isnan(y)) doc["y"] = y;
    if (!isnan(heading)) doc["heading"] = heading;

    // Add optional tile coordinates if provided
    if (tileX != INT_MIN) doc["tileX"] = tileX;
    if (tileY != INT_MIN) doc["tileY"] = tileY;

    // Add optional road tile type if provided
    if (roadTile != "") doc["roadTile"] = roadTile;

    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    buffer[n] = '\0';

    Serial.print("Sending ESP data: ");
    Serial.println(buffer);

    client.publish("esp", buffer, n);
}


// void sendEspData() {
//     DynamicJsonDocument doc(256);
//     doc["status"] = "online";
//     char buffer[256];
//     size_t n = serializeJson(doc, buffer);
//     buffer[n] = '\0';
//     client.publish("esp", buffer, n);
// }
