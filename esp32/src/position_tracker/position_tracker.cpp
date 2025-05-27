#include "position_tracker.h"
#include "../uart/uart_handler.h"
#include "../mqtt/mqtt_handler.h"

// For ±250 °/s full-scale, the MPU gives 131 LSB per °/s:
static constexpr float GYRO_LSB_PER_DPS = 131.0f;
// And to convert degrees to radians:
static constexpr float DPS_TO_RAD = PI / 180.0f;

float lastHeading = 0.0f;
float MIN_HEADING_CHANGE = 0.0f * DPS_TO_RAD;

int lastLeftDistance;
int lastRightDistance;

const int differenceLeftThreshold = 12;
const int differenceRightThreshold = 6;

int prevLeftTicks;
int prevRightTicks;

// bool startHeadingCaptured = false;
// float startHeading = 0.0f;    // radians

float headingScale = 90.0f / 70.8f;
// float headingScale = 90.0f / 68.0f;

extern String currentRoadTile;

PositionTracker::PositionTracker()
    : x(0.0), y(0.0), heading(0.0),
      leftTicks(0), rightTicks(0),
      leftLastTickTime(0), rightLastTickTime(0),
      gyroHeading(0.0), gyroCalibration(0.0),
      gyroIsCalibrated(false),
      calibSum(0), calibCount(0),
      tileX(0), tileY(0),
      leftDetector(74, 77, 1),
      rightDetector(36, 39, 1) {
}

void PositionTracker::begin() {
    resetPosition();
    lastUpdateTime = millis();
}

void PositionTracker::resetPosition() {
    x = TILE_SIZE/2.0f;
    y = TILE_SIZE/2.0f;
    heading = 0.0;
    leftTicks = 0;
    rightTicks = 0;

    gyroHeading = 0.0;
    gyroIsCalibrated = false;
    calibSum = 0;
    calibCount = 0;

    startHeadingCaptured = false;
    startHeading = 0.0f;
    zeroCaptureTime = 0;

    lastUpdateTime = millis();

    tileX = 0;
    tileY = 0;
}

struct HysteresisDetector {
    int highThreshold;      // Distance must go below this to trigger "blade in"
    int lowThreshold;       // Distance must go above this to trigger "blade out"
    bool bladeInZone;
    unsigned long lastTickTime;
    unsigned long minTickInterval;

    HysteresisDetector(int high, int low, unsigned long minInterval);
};

// HysteresisDetector::HysteresisDetector(int high, int low, unsigned long minInterval)
//     : highThreshold(high), lowThreshold(low), bladeInZone(false),
//       lastTickTime(0), minTickInterval(minInterval) {}

PositionTracker::HysteresisDetector::HysteresisDetector(int high, int low, unsigned long minInterval)
    : highThreshold(high), lowThreshold(low), bladeInZone(false),
      lastTickTime(0), minTickInterval(minInterval) {}


// Replace your existing sensor variables with these:
// HysteresisDetector leftDetector(60, 70, 5);   // Adjust thresholds as needed
// HysteresisDetector rightDetector(36, 42, 5);  // Adjust thresholds as needed

// Simple hysteresis detection function
bool PositionTracker::detectTick(HysteresisDetector& detector, int distance) {
    unsigned long currentTime = millis();
    bool tickDetected = false;

    if (!detector.bladeInZone) {
        // Waiting for blade to enter (distance to decrease)
        if (distance <= detector.highThreshold) {
            detector.bladeInZone = true;
            Serial.printf("Blade entered zone at distance %d\n", distance);
        }
    } else {
        // Blade is in zone, waiting for it to leave
        if (distance >= detector.lowThreshold) {
            // Blade has left - register tick if enough time has passed
            if (currentTime - detector.lastTickTime >= detector.minTickInterval) {
                tickDetected = true;
                detector.lastTickTime = currentTime;
                Serial.printf("TICK! Distance: %d\n", distance);
            }
            detector.bladeInZone = false;
        }
    }

    return tickDetected;
}

bool PositionTracker::detectTickWindowed(HysteresisDetector& det,
                                         std::deque<int>& window,
                                         int newDistance) {
  // 1) maintain window
  if (window.size() >= WINDOW_SIZE) window.pop_front();
  window.push_back(newDistance);

  // 2) compute min/max
  int mn = window.front(), mx = window.front();
  for (int d : window) { mn = min(mn,d); mx = max(mx,d); }

  // 3) if span crosses your hysteresis thresholds, pass into detectTick()
  if (det.bladeInZone) {
    if (mx >= det.lowThreshold) {
      return detectTick(det, newDistance);
    }
  } else {
    if (mn <= det.highThreshold) {
      return detectTick(det, newDistance);
    }
  }
  return false;
}



bool PositionTracker::isTickDetectedWindow(int currentDistance, bool isLeft) {
    // choose the right buffer and threshold
    auto &window     = isLeft ? leftWindow  : rightWindow;
    int    threshold = isLeft ? differenceLeftThreshold : differenceRightThreshold;

    // 1) push the new reading (and pop if we’re full)
    if (window.size() >= WINDOW_SIZE) window.pop_front();
    window.push_back(currentDistance);

    // 2) find min/max in the window
    int minD = window.front(), maxD = window.front();
    for (int d : window) {
        if (d < minD) minD = d;
        if (d > maxD) maxD = d;
    }

    // 3) if span ≥ threshold, it’s a tick
    if (maxD - minD >= threshold) {
        // reset buffer so we don't immediately retrigger on the same jump
        window.clear();
        window.push_back(currentDistance);
        return true;
    }

    return false;
}


bool PositionTracker::isTickDetected(int currentDistance, bool isLeft) {
    unsigned long currentTime = millis();

    if (isLeft) {
        if (currentDistance - lastLeftDistance >= differenceLeftThreshold) {
            lastLeftDistance = currentDistance;
            return true;
        }

        lastLeftDistance = currentDistance;
    } else {
        if (currentDistance - lastRightDistance >= differenceRightThreshold) {
            lastRightDistance = currentDistance;
            return true;
        }

        lastRightDistance = currentDistance;
    }

    return false;
}

int distancePrintInterval = 500;
int lastDistancePrint = millis();

void PositionTracker::processLeftDistance(int leftDistance) {
    if (leftDistance < 0) return; // Invalid reading

    unsigned long currentTime = millis();

    // Check for tick detection
    bool leftTickDetected = isTickDetectedWindow(leftDistance, true);
    // bool leftTickDetected = detectTick(leftDetector, leftDistance);
    // bool leftTickDetected = detectTickWindowed(leftDetector, leftWindow, leftDistance);

    if (leftTickDetected) {
        leftTicks++;
        Serial.printf("LEFT TICK at distance: %d, Total: %lu\n", leftDistance, leftTicks);
        updatePosition(currentTime);
    }

    lastUpdateTime = currentTime;
}

void PositionTracker::processRightDistance(int rightDistance) {
    if (rightDistance < 0) return; // Invalid reading

    unsigned long currentTime = millis();

    // Check for tick detection
    bool rightTickDetected = isTickDetectedWindow(rightDistance, false);
    // bool rightTickDetected = detectTick(rightDetector, rightDistance);
    // bool rightTickDetected = detectTickWindowed(rightDetector, rightWindow, rightDistance);

    if (rightTickDetected) {
        rightTicks++;
        Serial.printf("RIGHT TICK at distance: %d, Total: %lu\n", rightDistance, rightTicks);
        updatePosition(currentTime);
    }

    lastUpdateTime = currentTime;
}

float getAdjustedDistancePerTick() const {
    // Adjust based on tile type
    if (currentRoadTile == "straight") {
        return baseDistancePerTick * 1.0f;
    } else if (currentRoadTile == "left_turn") {
        return baseDistancePerTick * 0.8f;  // Turning is slower
    } else if (currentRoadTile == "right_turn") {
        return baseDistancePerTick * 0.8f;  // Turning is slower
    } else {
        return baseDistancePerTick;  // default
    }
}


void PositionTracker::updatePosition(unsigned long currentTime) {
    // Serial.println("Updating position");

    long dLT = leftTicks - prevLeftTicks;
    long dRT = rightTicks - prevRightTicks;
    prevLeftTicks = leftTicks;
    prevRightTicks = rightTicks;

    if (dLT == 0 && dRT == 0) {
      // Serial.println("No movement, not updating position");
      return; // No movement
    }

    float dpt = getAdjustedDistancePerTick();  // dynamic distance per tick
    float dL = dLT * dpt;
    float dR = dRT * dpt;


    // // Time-weighted position update
    // static unsigned long lastPositionUpdate = 0;
    // float timeFactor = 1.0f;

    // if (lastPositionUpdate > 0) {
    //     float deltaTime = (currentTime - lastPositionUpdate) / 1000.0f;
    //     // Apply time-based weighting to reduce impact of sporadic readings
    //     if (deltaTime > 0.1f) { // More than 100ms since last update
    //         timeFactor = min(1.0f, 0.1f / deltaTime); // Reduce impact of old readings
    //     }
    // }
    // lastPositionUpdate = currentTime;

    // // Apply time weighting to movement
    // dL *= timeFactor;
    // dR *= timeFactor;

    float distance = (dL + dR) / 2.0f;

    // float wheelHeadingChange = (dR - dL) / WHEEL_BASE;

    // Use wheel-based heading change for small movements
    // Use gyro heading for larger corrections
    // float currentHeading = heading + wheelHeadingChange / 2.0f; // Midpoint heading

    float currentHeading = lastHeading;

    if (fabs(heading - lastHeading) > MIN_HEADING_CHANGE) {
        currentHeading = gyroHeading;
        lastHeading = heading;
    }
    // Update position using current heading (from gyro)
    // Don't calculate heading from wheel differences - use gyro heading only
    x += distance * cos(currentHeading);
    y += distance * sin(currentHeading);


    // Update tile position with hysteresis to prevent oscillation
    static float tileHysteresis = TILE_SIZE * 0.1f; // 10% hysteresis
    int newTileX = static_cast<int>(floor((x + tileHysteresis) / TILE_SIZE));
    int newTileY = static_cast<int>(floor((y + tileHysteresis) / TILE_SIZE));

    if (newTileX != tileX || newTileY != tileY) {
        tileX = newTileX;
        tileY = newTileY;
        Serial.printf("New tile: (%d, %d) at position (%.2f, %.2f)\n", tileX, tileY, x, y);

        sendPosition(tileX, tileY);
        sendEspData(x, y, heading, positionTracker.getTileX(), positionTracker.getTileY(), currentRoadTile);
    }
}

void PositionTracker::processMPUData(int16_t rawGyroZ, float deltaTimeMs) {
    // Enhanced gyro processing with time-based filtering
    if (!gyroIsCalibrated) {
        if (calibCount == 0) Serial.println("Calibrating gyro...");

        calibSum += rawGyroZ;
        calibCount += 1;
        if (calibCount >= CALIB_SAMPLES) {
            gyroCalibration = calibSum / float(CALIB_SAMPLES);
            gyroIsCalibrated = true;
            Serial.printf("Gyro bias calibrated: %.2f LSB\n", gyroCalibration);
            zeroCaptureTime = millis();
        }
        return;
    }

    // Apply enhanced filtering
    float unbiased = float(rawGyroZ) - gyroCalibration;

    // Dynamic deadband based on recent motion
    float adaptiveDeadband = GYRO_DEADBAND;

    if (fabs(unbiased) < adaptiveDeadband) {
        unbiased = 0;
    }

    float rateDps = unbiased / GYRO_LSB_PER_DPS;
    float rateRads = rateDps * DPS_TO_RAD;

    // Time-based integration with bounds checking
    float dt = deltaTimeMs * 0.001f;
    if (dt > 0.1f) dt = 0.1f; // Prevent large time jumps from causing issues

    gyroHeading += rateRads * dt;

    // Normalize heading
    if (gyroHeading >= 2*PI) gyroHeading -= 2*PI;
    else if (gyroHeading < 0) gyroHeading += 2*PI;

    // heading = gyroHeading;


    if (!startHeadingCaptured && (millis() - zeroCaptureTime) > 500) {
        startHeadingCaptured = true;
        startHeading = gyroHeading;
    }

    float relative = gyroHeading - startHeading;

    // Wrap into (-PI … +PI]:
    while (relative >  PI) relative -= 2*PI;
    while (relative <= -PI) relative += 2*PI;

    // Apply scale calibration gain
    relative *= headingScale;

    if      (relative >  PI) relative =  PI;
    else if (relative < -PI) relative = -PI;

    // if (fabs(relative) < (1.0 * DPS_TO_RAD))  // e.g. 1° of noise
    //     relative = 0;

    heading = relative;
}

// void PositionTracker::updatePosition() {
//     updatePosition(millis());
// }

float PositionTracker::getX() const {
    return x;
}

float PositionTracker::getY() const {
    return y;
}

float PositionTracker::getHeading() const {
    return heading;
}

float PositionTracker::getHeadingDegrees() const {
    return heading * 180.0 / PI;
}

float PositionTracker::getLeftDistance() const {
    return leftTicks * DISTANCE_PER_TICK;
}

float PositionTracker::getRightDistance() const {
    return rightTicks * DISTANCE_PER_TICK;
}

unsigned long PositionTracker::getLeftTicks() const {
    return leftTicks;
}

unsigned long PositionTracker::getRightTicks() const {
    return rightTicks;
}

int PositionTracker::getTileX() const {
    return tileX;
}

int PositionTracker::getTileY() const {
    return tileY;
}

bool PositionTracker::isGyroCalibrated() const {
    return gyroIsCalibrated;
}

void PositionTracker::setShouldRegisterTicks(bool shouldRegister) {
    shouldRegisterTicks = shouldRegister;
}

