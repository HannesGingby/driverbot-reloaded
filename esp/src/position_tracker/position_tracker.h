#ifndef POSITION_TRACKER_H
#define POSITION_TRACKER_H

#include <Arduino.h>

class PositionTracker {
private:
    static constexpr float TRACK_LENGTH = 455.0;   // mm
    static constexpr float TRACK_WIDTH = 200.0;   // Distance between tracks
    static constexpr float DISTANCE_PER_TICK = 30.3;   // mm

    static constexpr int DETECTION_THRESHOLD = 20;   // mm, threshold for detecting trigger wheel ticks
    static constexpr int TICK_DEBOUNCE_TIME = 1;   // ms, minimum time between tick detections

    static constexpr float TILE_SIZE = 200.0;   // mm

    // Position tracking variables
    float x;   // X position in mm
    float y;   // Y position in mm
    float heading;   // Heading in radians

    int tileX;   // X position in tiles
    int tileY;   // Y position in tiles

    // Tick counting
    unsigned long leftTicks;
    unsigned long rightTicks;
    unsigned long leftLastTickTime;
    unsigned long rightLastTickTime;

    // Previous distance readings
    int leftPrevDistance;
    int rightPrevDistance;

    // Function to update position based on new tick counts
    void updatePosition();

public:
    PositionTracker();

    // Initialize the tracker
    void begin();

    // Reset position to origin (0,0,0)
    void resetPosition();

    // Process new distance readings from sensors
    void processSensorReadings(int leftDistance, int rightDistance);

    // Get position and orientation information
    float getX() const;
    float getY() const;
    float getHeading() const;
    float getHeadingDegrees() const;
    int getTileX() const;
    int getTileY() const;

    // Get distance traveled by each track
    float getLeftDistance() const;
    float getRightDistance() const;

    // Get tick counts
    unsigned long getLeftTicks() const;
    unsigned long getRightTicks() const;

    // Calculate trigger wheel tick from distance reading
    bool isTickDetected(int currentDistance, int previousDistance);
};

#endif // POSITION_TRACKER_H
