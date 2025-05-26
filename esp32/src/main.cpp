#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <time.h>
#include <FS.h>
#include <LittleFS.h>

#include <Wire.h>
#include <VL53L0X.h>
#include <MPU6050.h>
#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
 #include <avr/power.h>   // Required for 16 MHz Adafruit Trinket
#endif

#include "./motor_control/motor_control.h"
#include "./mqtt/mqtt_handler.h"
#include "./uart/uart_handler.h"
#include "./credentials.h"
#include "./position_tracker/position_tracker.h"


/*

PIN SETUP           GPIO

- I2C SCL           = 21
- I2C SDA           = 22

- VL53L0X 1 XSHUT   = 18
- VL53L0X 2 XSHUT   = 19

- LED ring          = 10

- Motor AIN1        = 0
- Motor AIN2        = 1
- Motor BIN1        = 3
- Motor BIN2        = 2

- PWM motor A       = 20
- PWM motor B       = 23

- UART RX           = 4
- UART TX           = 5

POWER/GROUND

esp 3V3 -> TOF left, TOF right, MPU
esp GND -> TOF left, TOF right, MPU

*/


VL53L0X sensorLeft;
VL53L0X sensorRight;
bool distanceSensorsInitialized;
const long distanceInterval = 2;    // how often to read the VL53L0X sensors
const long mpuInterval = 30;   // how often to poll & print the MPU
const long printInterval = 500;

// In globals:
unsigned long lastDistanceRead = 0;
unsigned long lastMPURead = 0;
unsigned long lastPrint = 0;

#define I2C_SCL 21
#define I2C_SDA 22

const uint8_t XSHUT_LEFT_PIN  = 18;
const uint8_t XSHUT_RIGHT_PIN = 19;

bool i2cBusy = false;
unsigned long lastI2CTransaction = 0;
const unsigned long I2C_TRANSACTION_DELAY = 5; // ms between I2C transactions

MPU6050 mpu(0x68);
bool mpuInitialized;
int16_t ax, ay, az;
int16_t gx, gy, gz;

PositionTracker positionTracker;

float x, y, heading;
int tileX, tileY;

#define LIGHT_PIN 10
#define NUMPIXELS 16   // NeoPixel ring size

Adafruit_NeoPixel pixels(NUMPIXELS, LIGHT_PIN, NEO_GRB + NEO_KHZ800);

const char *ntpServer = "pool.ntp.org";
const long gmtOffsetSec = 0;
const int daylightOffsetSec = 0;

// MQTT Settings
int mqttPort = 8883;
String espClientId = "esp32-client-" + String(WiFi.macAddress());

// Global MQTT and SSL Client
WiFiClientSecure espClient;
PubSubClient client(espClient);

// Global drive/steer variables
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
    configTime(gmtOffsetSec, daylightOffsetSec, ntpServer);
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

void i2cBusUnstick() {
  pinMode(I2C_SCL, OUTPUT);
  pinMode(I2C_SDA, INPUT_PULLUP);
  for (int i = 0; i < 9; i++) {
    digitalWrite(I2C_SCL, HIGH);
    delayMicroseconds(5);
    digitalWrite(I2C_SCL, LOW);
    delayMicroseconds(5);
  }
  pinMode(I2C_SDA, INPUT_PULLUP);
  digitalWrite(I2C_SCL, HIGH);
  delayMicroseconds(5);
  pinMode(I2C_SCL, INPUT_PULLUP);
}

bool setupDistanceSensors() {
  Serial.println("Setting up distance sensors...");

  // // Initialize I2C with explicit pins and lower clock speed for stability
  // Wire.begin(I2C_SDA, I2C_SCL);
  // Wire.setClock(50000); // Reduced from 400kHz to 100kHz for better stability
  // Wire.setTimeout(1000); // Set timeout to 1 second

  pinMode(XSHUT_LEFT_PIN, OUTPUT);
  pinMode(XSHUT_RIGHT_PIN, OUTPUT);

  // Step 1: Disable both sensors and wait longer
  digitalWrite(XSHUT_LEFT_PIN, LOW);
  digitalWrite(XSHUT_RIGHT_PIN, LOW);
  delay(100); // Increased delay

  // Step 2: Enable only RIGHT sensor first
  digitalWrite(XSHUT_RIGHT_PIN, HIGH);
  delay(100); // Increased delay

  // Check if right sensor is responsive before proceeding
  Wire.beginTransmission(0x29);
  if (Wire.endTransmission() != 0) {
    Serial.println("Right sensor not detected at default address");
    return false;
  }

  sensorRight.setBus(&Wire);
  sensorRight.setTimeout(1000);
  if (!sensorRight.init()) {
    Serial.println("Sensor right init failed");
    return false;
  }

  // Step 3: Change address of RIGHT to 0x31
  sensorRight.setAddress(0x31);
  Serial.println("Sensor right address changed to 0x31");
  delay(50); // Increased delay

  // Verify the address change worked
  Wire.beginTransmission(0x31);
  if (Wire.endTransmission() != 0) {
    Serial.println("Right sensor address change failed");
    return false;
  }

  // Step 4: Enable LEFT sensor
  digitalWrite(XSHUT_LEFT_PIN, HIGH);
  delay(100); // Increased delay

  // Check if left sensor is responsive
  Wire.beginTransmission(0x29);
  if (Wire.endTransmission() != 0) {
    Serial.println("Left sensor not detected at default address");
    return false;
  }

  sensorLeft.setBus(&Wire);
  sensorLeft.setTimeout(1000);
  if (!sensorLeft.init()) {
    Serial.println("Sensor left init failed");
    return false;
  }

  // Step 5: Set LEFT to 0x30
  sensorLeft.setAddress(0x30);
  Serial.println("Sensor left address changed to 0x30");
  delay(50);

  // Verify the address change worked
  Wire.beginTransmission(0x30);
  if (Wire.endTransmission() != 0) {
    Serial.println("Left sensor address change failed");
    return false;
  }

  // Configure both sensors for better performance
  sensorLeft.setMeasurementTimingBudget(50000); // 50ms timing budget
  sensorRight.setMeasurementTimingBudget(50000);

  Serial.println("Both distance sensors initialized successfully");
  return true;
}



bool setupMPU() {
  Serial.println("Initializing MPU...");
  mpu.initialize();
  mpu.setClockSource(MPU6050_CLOCK_PLL_XGYRO);
  mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
  mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
  mpu.setSleepEnabled(false);
  Serial.println("Testing MPU6050 connection...");
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed");
    return false;
  }
  Serial.println("MPU6050 connection successful");
  return true;
}



void recoverI2C() {
  Serial.println("Attempting I2C recovery...");
  // Manually unstick any hung I2C devices
  i2cBusUnstick();
  // Reinitialize the hardware I2C bus
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(100000);
  Wire.setTimeout(1000);
  delay(50);

  // Reinitialize sensors on the freshly reset bus
  distanceSensorsInitialized = setupDistanceSensors();
  mpuInitialized             = setupMPU();
  Serial.println("I2C recovery complete");
}


bool safeI2CTransaction(std::function<bool()> txn) {
    unsigned long now = millis();
    if (now - lastI2CTransaction < I2C_TRANSACTION_DELAY) {
        delay(I2C_TRANSACTION_DELAY - (now - lastI2CTransaction));
    }
    bool ok = txn();
    lastI2CTransaction = millis();
    if (!ok) {
        Serial.println("I2C transaction failed, recovering bus...");
        recoverI2C();
        return false;
    }
    return true;
}


void trackPosition() {
    if (millis() - lastDistanceRead >= distanceInterval) {
        if (positionTracker.shouldRegisterTicks) {
            lastDistanceRead = millis();
            if (distanceSensorsInitialized) {
                safeI2CTransaction([&]() {
                    int d = sensorLeft.readRangeSingleMillimeters();
                    // Serial.printf("Left: %d\n", d);
                    if (!sensorLeft.timeoutOccurred() && d < 8000) {
                        positionTracker.processLeftDistance(d);
                        return true;
                    }
                    return false;
                });
                delay(10);
                safeI2CTransaction([&]() {
                    int d = sensorRight.readRangeSingleMillimeters();
                    // Serial.printf("Right: %d\n", d);
                    if (!sensorRight.timeoutOccurred() && d < 8000) {
                        positionTracker.processRightDistance(d);
                        return true;
                    }
                    return false;
                });
            } else {
                static unsigned long lastAttempt = 0;
                if (millis() - lastAttempt > 10000) {
                    lastAttempt = millis();
                    Serial.println("Attempting to reinitialize sensors...");
                    distanceSensorsInitialized = setupDistanceSensors();
                }
            }
        }
    }

    if (mpuInitialized && millis() - lastMPURead >= mpuInterval) {
        unsigned long now = millis();
        float dt = float(now - lastMPURead);
        lastMPURead = now;
        safeI2CTransaction([&]() {
            mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
            positionTracker.processMPUData(gz, dt);
            return true;
        });
    }

    if (millis() - lastPrint >= printInterval && positionTracker.isGyroCalibrated()) {
        lastPrint = millis();
        x = positionTracker.getX();
        y = positionTracker.getY();
        heading = positionTracker.getHeadingDegrees();
        Serial.printf("Position: (%.1f mm, %.1f mm) @ %.1f°\n", x, y, heading);
    }
}


void scanI2C() {
    Serial.println("Running I2C scan…");
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(100000); // Use same slow speed as main setup

    int deviceCount = 0;
    for (byte address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        byte error = Wire.endTransmission();

        if (error == 0) {
            Serial.printf(" ➜ Device found at 0x%02X\n", address);
            deviceCount++;
        }
        delay(10); // Small delay between scans
    }

    Serial.printf("I2C scan done. Found %d devices.\n\n", deviceCount);
}



void setup() {
    delay(500);
    Serial.begin(115200);   // Debug serial
    while (!Serial) { /* wait */ }

    setupUART();   // UART serial
    delay(1000);

    // Initialize WiFi and time sync
    setupWifi();
    syncTime();

    i2cBusUnstick();
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setClock(100000);
    Wire.setTimeout(100);

    ledcAttach(MOTOR_A_PWM_PIN, 1000, 8);
    ledcAttach(MOTOR_B_PWM_PIN, 1000, 8);

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(MOTOR_AIN1_PIN, OUTPUT);
    pinMode(MOTOR_AIN2_PIN, OUTPUT);
    pinMode(MOTOR_BIN1_PIN, OUTPUT);
    pinMode(MOTOR_BIN2_PIN, OUTPUT);

    digitalWrite(MOTOR_AIN1_PIN, LOW);
    digitalWrite(MOTOR_AIN2_PIN, LOW);
    digitalWrite(MOTOR_BIN1_PIN, LOW);
    digitalWrite(MOTOR_BIN2_PIN, LOW);

    ledcWrite(MOTOR_A_PWM_PIN, 0);
    ledcWrite(MOTOR_B_PWM_PIN, 0);

    // SSL setup and MQTT client
    espClient.setCACert(ca_cert);
    client.setKeepAlive(60);
    client.setServer(mqttServer, mqttPort);
    client.setCallback(mqttCallback);

    // Initialize the distance sensors
    distanceSensorsInitialized = setupDistanceSensors();
    mpuInitialized = setupMPU();

    // Light
    #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
    clock_prescale_set(clock_div_1);
    #endif

    pixels.begin();
    for(int i=0; i<NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(255, 255, 200));
        pixels.show();
    }

    if (distanceSensorsInitialized && mpuInitialized) {
        positionTracker.begin();
    }

    // Blink LED to indicate that the board is ready
    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
    }
}


void loop() {
    yield();

    if (!client.connected()) {
        connectMQTT();
    }
    client.loop();
    trackPosition();

    if (positionTracker.isGyroCalibrated()) {
        processUART();
    }

    // delay(10);
}
