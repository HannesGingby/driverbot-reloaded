#include "position_tracker.h"

static long prevLeftTicks  = 0;
static long prevRightTicks = 0;

PositionTracker::PositionTracker()
    : x(0.0), y(0.0), heading(0.0),
      leftTicks(0), rightTicks(0),
      leftLastTickTime(0), rightLastTickTime(0),
      leftPrevDistance(-1), rightPrevDistance(-1) {
}

void PositionTracker::begin() {
    resetPosition();
}

void PositionTracker::resetPosition() {
    x = 0.0;
    y = 0.0;
    heading = 0.0;
    leftTicks = 0;
    rightTicks = 0;
    leftPrevDistance = -1;
    rightPrevDistance = -1;
}

bool PositionTracker::isTickDetected(int currentDistance, int previousDistance) {
    // Skip if this is the first reading
    if (previousDistance < 0) {
        return false;
    }

    // Calculate the absolute difference between current and previous readings
    int diff = abs(currentDistance - previousDistance);

    // Check if the difference exceeds our threshold
    return (diff >= DETECTION_THRESHOLD);
}

void PositionTracker::processSensorReadings(int leftDistance, int rightDistance) {
    unsigned long currentTime = millis();
    bool leftTickDetected = false;
    bool rightTickDetected = false;

    // Check for left wheel tick
    if (isTickDetected(leftDistance, leftPrevDistance)) {
        // Debounce to prevent multiple tick detections from a single event
        if (currentTime - leftLastTickTime > TICK_DEBOUNCE_TIME) {
            leftTicks++;
            leftLastTickTime = currentTime;
            leftTickDetected = true;
        }
    }

    // Check for right wheel tick
    if (isTickDetected(rightDistance, rightPrevDistance)) {
        // Debounce to prevent multiple tick detections from a single event
        if (currentTime - rightLastTickTime > TICK_DEBOUNCE_TIME) {
            rightTicks++;
            rightLastTickTime = currentTime;
            rightTickDetected = true;
        }
    }

    // Update previous distance readings
    leftPrevDistance = leftDistance;
    rightPrevDistance = rightDistance;

    // Update position if we detected ticks
    if (leftTickDetected || rightTickDetected) {
        updatePosition();
    }
}

void PositionTracker::updatePosition() {
    long dLT = leftTicks - prevLeftTicks;
    long dRT = rightTicks - prevRightTicks;
    prevLeftTicks = leftTicks;
    prevRightTicks = rightTicks;

    float dL = dLT * DISTANCE_PER_TICK;
    float dR = dRT * DISTANCE_PER_TICK;

    // Compute odometry
    float Δs = (dR + dL) * 0.5f;
    float Δθ = (dR - dL) / TRACK_WIDTH;
    float θ_mid = heading + Δθ * 0.5f;

    // Update position
    x += Δs * cos(θ_mid);
    y += Δs * sin(θ_mid);
    heading += Δθ;

    int newTileX = static_cast<int>( floor( x / TILE_SIZE ) );
    int newTileY = static_cast<int>( floor( y / TILE_SIZE ) );

    if ( newTileX != tileX || newTileY != tileY ) {
      tileX = newTileX;
      tileY = newTileY;
    }


    // // Calculate distance moved by each track
    // float leftDistance = leftTicks * DISTANCE_PER_TICK;
    // float rightDistance = rightTicks * DISTANCE_PER_TICK;

    // // Calculate distance moved by center point
    // float distanceMoved = (leftDistance + rightDistance) / 2.0;

    // // Calculate change in heading (turning angle)
    // float deltaHeading = (rightDistance - leftDistance) / TRACK_WIDTH;

    // // Update heading
    // heading += deltaHeading;

    // // Keep heading in [0, 2π] range
    // while (heading >= 2 * PI) heading -= 2 * PI;
    // while (heading < 0) heading += 2 * PI;

    // // Update X and Y position
    // x += distanceMoved * cos(heading);
    // y += distanceMoved * sin(heading);
}

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
