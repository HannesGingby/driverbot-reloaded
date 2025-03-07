#include <ESP8266WiFi.h>
#include "mqtt_handler.h"
#include "../motor_control/motor_control.h"  // For motor control functions

// The MQTT client is defined in main.cpp
extern PubSubClient client;

// Global JSON fields defined in main.cpp
extern int direction;
extern int steer;
extern int speedPercentage;
extern int steerAngle;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.println("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    if (strcmp(topic, "movement") == 0) {
        DynamicJsonDocument doc(512);
        deserializeJson(doc, payload, length);

        direction = doc["direction"].as<int>();
        steer = doc["steer"].as<int>();
        speedPercentage = doc["speed"];
        steerAngle = doc["steerAngle"];

        Serial.print("Received JSON: ");
        serializeJson(doc, Serial);
        Serial.println();

        int driveSpeed = (speedPercentage * maxMotorSpeed) / 100;

        if (direction == 1) {
            driveForward(driveSpeed);
        } else if (direction == -1) {
            driveBackward(driveSpeed);
        } else if (direction == 0) {
            stopDriving();
        }

        if (steer == 1) {
            steerRight(steerAngle, driveSpeed);
        } else if (steer == -1) {
            steerLeft(steerAngle, driveSpeed);
        } else if (steer == 0) {
            resetSteering();
        }
    }

    if (strcmp(topic, "ping") == 0) {
        Serial.println("Ping message received, sending response");
        client.publish("ping", "1");
    }
}

void connectMQTT() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection…");

        // Resolve broker hostname first
        extern const char* mqttServer;
        IPAddress brokerIP;
        if (WiFi.hostByName(mqttServer, brokerIP)) {
            Serial.print("Resolved broker IP: ");
            Serial.println(brokerIP);
        } else {
            Serial.println("Failed to resolve broker hostname. Retrying in 5 seconds...");
            delay(5000);
            continue;
        }

        extern const char* mqttUsername;
        extern const char* mqttPassword;
        extern String espClientId;
        if (client.connect(espClientId.c_str(), mqttUsername, mqttPassword)) {
            Serial.println("Connected to MQTT broker");
            client.subscribe("movement");
            client.subscribe("ping");
            sendEspData();
        } else {
            Serial.print("failed, rc = ");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void sendEspData() {
    // Now send a simple status message instead of position data.
    DynamicJsonDocument doc(256);
    doc["status"] = "online";
    char buffer[256];
    size_t n = serializeJson(doc, buffer);
    buffer[n] = '\0';
    client.publish("esp", buffer, n);
}
