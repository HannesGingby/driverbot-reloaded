#ifndef POSITION_TRACKER_H
#define POSITION_TRACKER_H

#include <Arduino.h>
#include <math.h>
#include <deque>

class PositionTracker {
public:
    PositionTracker();

    void begin();
    void resetPosition();

    void processLeftDistance(int leftDistance);
    void processRightDistance(int rightDistance);
    void processMPUData(int16_t rawGyroZ, float deltaTimeMs);

    float getX() const;
    float getY() const;
    float getHeading() const;
    float getHeadingDegrees() const;
    float getLeftDistance() const;
    float getRightDistance() const;
    unsigned long getLeftTicks() const;
    unsigned long getRightTicks() const;
    int getTileX() const;
    int getTileY() const;
    bool isGyroCalibrated() const;

    void setShouldRegisterTicks(bool shouldRegister);
    bool shouldRegisterTicks = true;
private:
    bool isTickDetected(int currentDistance, bool isLeft);
    void updatePosition(unsigned long currentTime);

    static constexpr size_t WINDOW_SIZE = 20;
    std::deque<int> leftWindow, rightWindow;


    bool isTickDetectedWindow(int currentDistance, bool isLeft);

    struct HysteresisDetector {
        int highThreshold;
        int lowThreshold;
        bool bladeInZone;
        unsigned long lastTickTime;
        unsigned long minTickInterval;

        HysteresisDetector(int high, int low, unsigned long minInterval = 50);
    };

    HysteresisDetector leftDetector;
    HysteresisDetector rightDetector;

    // In your PositionTracker class, add:
    bool detectTick(HysteresisDetector& detector, int distance);
    bool detectTickWindowed(HysteresisDetector& detector, std::deque<int>& window, int newDistance);

    // Robot configuration constants
    static constexpr float WHEEL_BASE = 145.0;        // Distance between wheels in mm
    // static constexpr float DISTANCE_PER_TICK = 31;   // mm traveled per encoder tick
    static constexpr float DISTANCE_PER_TICK = 31;   // mm traveled per encoder tick
    // static constexpr float DISTANCE_PER_TICK = 30;   // mm traveled per encoder tick

    static constexpr float TILE_SIZE = 256.0;          // mm per tile

    // Gyro calibration
    static constexpr int CALIB_SAMPLES = 100;         // Number of samples for calibration
    static constexpr float GYRO_DEADBAND = 10.0f;     // Ignore small gyro movements when stationary

    bool   startHeadingCaptured = false;
    float  startHeading = 0.0f;  // radians
    unsigned long zeroCaptureTime = 0;  // millis() at calibration

    unsigned long lastUpdateTime;

    // Position tracking variables
    float x = TILE_SIZE/2.0f, y = TILE_SIZE/2.0f;           // Position in mm
    float heading;        // Heading in radians [0, 2π)

    // Odometry variables
    unsigned long leftTicks, rightTicks;
    unsigned long leftLastTickTime, rightLastTickTime;
    int leftPrevDistance, rightPrevDistance;

    // Gyro variables
    float gyroHeading;       // Heading from gyro integration
    float gyroCalibration;   // Gyro zero-rate calibration
    bool gyroIsCalibrated;   // Flag to indicate if calibration is complete

    // Calibration variables
    float calibSum;          // Sum for calculating gyro bias
    int calibCount;          // Counter for calibration samples

    // Tile position
    int tileX, tileY;       // Current tile coordinates
};

extern PositionTracker positionTracker;

#endif // POSITION_TRACKER_H

