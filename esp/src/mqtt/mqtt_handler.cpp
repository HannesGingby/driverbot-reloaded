#include <ESP8266WiFi.h>
#include "mqtt_handler.h"
#include "../motor_control/motor_control.h"  // For motor control functions

#include "../credentials.h"

// The MQTT client is defined in main.cpp
extern PubSubClient client;

// Global JSON fields defined in main.cpp
extern int driveDirection;
extern int steerDirection;
extern int driveSpeedPercentage;
extern int steerSpeedPercentage;


void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.println("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

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
            steer(steerSpeed, true);  // Right
        } else if (steerDirection == -1) {
            steer(steerSpeed, false);  // Left
        }

        if (driveDirection == 0 && steerDirection == 0) {
            // When not steering or driving
            isSteering = false;
            stopMotors();   // Stop
        } else if (driveDirection == 0 && steerDirection != 0) {
            // When steering
            isSteering = true;
        }
    }

    /*
    if (strcmp(topic, "ping") == 0) {
        Serial.println("Ping message received, sending response");
        client.publish("ping", "1");
    }
    */
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
