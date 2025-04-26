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

#include "Adafruit_NeoPixel.h"
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define LIGHT_PIN 12 // D6

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 16 // Popular NeoPixel ring size

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, LIGHT_PIN, NEO_GRB + NEO_KHZ800);

// #define DELAYVAL 500 // Time (in milliseconds) to pause between pixels


const char *ntpServer = "pool.ntp.org";
const long gmtOffsetSec = 0;
const int daylightOffsetSec = 0;

// MQTT Settings
int mqttPort = 8883;
String espClientId = "esp8266-client-" + String(WiFi.macAddress());

// Global MQTT and SSL Client
BearSSL::WiFiClientSecure espClient;
PubSubClient client(espClient);

// Global JSON variables
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

    // Set up SSL and MQTT client
    espClient.setTrustAnchors(new BearSSL::X509List(ca_cert));
    client.setKeepAlive(60);
    client.setServer(mqttServer, mqttPort);
    client.setCallback(mqttCallback);


    // Light

    // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
    // Any other board, you can remove this part (but no harm leaving it):
    #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
    #endif
    // END of Trinket-specific code.

    pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
    for(int i=0; i<NUMPIXELS; i++) { // For each pixel...
        // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
        // Here we're using a moderately bright green color:
        pixels.setPixelColor(i, pixels.Color(255, 255, 200));
        pixels.show();   // Send the updated pixel colors to the hardware.
    }

}

void loop() {
    if (!client.connected()) {
        connectMQTT();
    }
    client.loop();
    processUART();


    // // Light
    // pixels.clear(); // Set all pixel colors to 'off'

    // // The first NeoPixel in a strand is #0, second is 1, all the way up
    // // to the count of pixels minus one.
    // for(int i=0; i<NUMPIXELS; i++) { // For each pixel...

    //   // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    //   // Here we're using a moderately bright green color:
    //   pixels.setPixelColor(i, pixels.Color(150, 150, 150));

    //   pixels.show();   // Send the updated pixel colors to the hardware.

    //   // delay(DELAYVAL); // Pause before next pass through loop
    // }
}
