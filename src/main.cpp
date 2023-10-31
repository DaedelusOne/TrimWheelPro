#include <Arduino.h>
#include "ResponsiveAnalogRead.h"
#include "Joystick.h"

#define ANALOG_PIN A0
#define DELAY 50
#define MAX_ANALOG 1023.0
#define DEGREES_PER_TURN 360
#define MAX_TURNS 4
#define MAX_RANGE DEGREES_PER_TURN * MAX_TURNS
#define MIN_RANGE 0

// The threshold value is used to detect if the sensor has been rotated fully and has crossed the point of 360/0 degrees
#define FULL_ROTATION_DIFF_THRESHOLD (DEGREES_PER_TURN / 100) * 60 // 60% of degrees per turn

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD,
                   0, 0,
                   false, false, false,
                   true, false, false,
                   false, false,
                   false, false, false);


volatile int prevAngle = 0;
volatile int joystickValue;
ResponsiveAnalogRead sensorReader(ANALOG_PIN, true);

int readAnalogue() {
    sensorReader.update();
    return sensorReader.getValue() / MAX_ANALOG * DEGREES_PER_TURN;
}

int getAngleDifference(int oldDegrees, int newDegrees) {
    int diff = newDegrees - oldDegrees;
    if (diff > FULL_ROTATION_DIFF_THRESHOLD) diff = diff - newDegrees;
    if (diff < -FULL_ROTATION_DIFF_THRESHOLD) diff = oldDegrees + diff;

    return diff;
}

__attribute__((unused))
void setup() {
    prevAngle = readAnalogue();
    joystickValue = (MAX_RANGE - MIN_RANGE) / 2;
    Joystick.begin();
    Joystick.setRxAxisRange(MIN_RANGE, MAX_RANGE);
    Joystick.setRxAxis(joystickValue);
}

__attribute__((unused))
void loop() {
    int currentAngle = readAnalogue();
    joystickValue += getAngleDifference(prevAngle, currentAngle);
    joystickValue = max(min(joystickValue, MAX_RANGE), MIN_RANGE);
    Joystick.setRxAxis(joystickValue);
    prevAngle = currentAngle;

    delay(DELAY);
}
