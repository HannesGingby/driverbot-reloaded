#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <time.h>
#include <TZ.h>
#include <FS.h>
#include <LittleFS.h>
#include <CertStoreBearSSL.h>
#include <SoftwareSerial.h>

#include "./motor_control/motor_control.h"
#include "./mqtt/mqtt_handler.h"
#include "./uart/uart_handler.h"

#include "./credentials.h"

const char *ntpServer = "pool.ntp.org";
const long gmtOffsetSec = 0;
const int daylightOffsetSec = 0;

// MQTT Settings
int mqttPort = 8883;
String espClientId = "esp8266-client-" + String(WiFi.macAddress());

// Global MQTT and SSL Client
BearSSL::WiFiClientSecure espClient;
PubSubClient client(espClient);

// Global JSON Variables (shared by MQTT and Motor Control)
// String direction = "";
// String steer = "";
// int speedPercentage = 0;
// int steerAngle = 0;
int driveDirection = 0;   // 0 = stop, 1 = forward, -1 = backward
int steerDirection = 0;   // 0 = stop, 1 = right, -1 = left
int driveSpeedPercentage = 0;
int steerSpeedPercentage = 0;

void setupWifi() {
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    int timeout = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        timeout++;
        if (timeout > 20) {  // 10 second timeout
            Serial.println("\nWiFi connection timeout - restarting");
            ESP.restart();
        }
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("Signal strength (RSSI): ");
    Serial.println(WiFi.RSSI());
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

// Synchronize time with NTP server, required for the certificate validation
void syncTime() {
    configTime(TZ_Europe_Stockholm, ntpServer);
    Serial.print("Waiting for NTP time sync: ");
    time_t now = time(nullptr);
    int timeout = 0;
    while (now < 8 * 3600 * 2) {
        delay(1000);
        Serial.print(".");
        now = time(nullptr);
        timeout++;
        if (timeout > 20) {
            Serial.println("\nNTP sync timeout reached.");
            break;
        }
    }
    if (now >= 8 * 3600 * 2) {
        Serial.println("\nNTP time synchronized.");
    } else {
        Serial.println("\nFailed to synchronize NTP time.");
    }
    Serial.println();

    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    Serial.printf("%s %s", tzname[0], asctime(&timeinfo));
}

void checkNTPPort() {
    WiFiClient testClient;
    Serial.println("Testing NTP port connectivity...");
    if (testClient.connect("pool.ntp.org", 123)) {
        Serial.println("NTP port seems open");
        testClient.stop();
    } else {
        Serial.println("NTP port might be blocked!");
    }
}

void setup() {
    delay(500);
    serial.begin(115200);
    Serial.begin(115200);
    delay(500);

    // Initialize WiFi and time sync
    setupWifi();
    syncTime();

    // LED and Motor/Servo Setup
    pinMode(LED_BUILTIN, OUTPUT);

    pinMode(MOTOR_1_DIR_PIN, OUTPUT);
    pinMode(MOTOR_1_SPEED_PIN, OUTPUT);
    pinMode(MOTOR_2_DIR_PIN, OUTPUT);
    pinMode(MOTOR_2_SPEED_PIN, OUTPUT);

    //analogWrite(MOTOR_PIN_RIGHT_SPEED, 1);
    //digitalWrite(MOTOR_PIN_RIGHT_DIR, 1);

    // servo.attach(SERVO_PIN);

    // Set up SSL and MQTT client
    espClient.setTrustAnchors(new BearSSL::X509List(ca_cert));
    client.setKeepAlive(60);
    client.setServer(mqttServer, mqttPort);
    client.setCallback(mqttCallback);
}

void loop() {
    if (!client.connected()) {
        connectMQTT();
    }
    client.loop();
    processUART();
}
