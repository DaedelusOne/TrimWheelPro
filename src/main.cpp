#include <Arduino.h>
#include "LibPrintf.h"
#include "ResponsiveAnalogRead.h"

#define ANALOG_PIN A0
#define DELAY 50
#define MAX_ANALOG 1023.0
#define MAX_HALL_ANGLE_PER_TURN 360

//The difference value that cannot be exceeded. Used to detect the sensor has been fully rotated or not
#define FULL_ROTATION_DIFF_THRESHOLD (MAX_HALL_ANGLE_PER_TURN / 100) * 60

volatile bool isConnected = false;
volatile int prevAngle = 0;
ResponsiveAnalogRead sensorReader(ANALOG_PIN, true);

int readAnalogue() {
    sensorReader.update();
    return sensorReader.getValue() / MAX_ANALOG * MAX_HALL_ANGLE_PER_TURN;
}

__attribute__((unused))
void setup() {
    Serial.begin(115200);
    prevAngle = readAnalogue();
}

int getDifference(int oldDegrees, int newDegrees) {
    int diff = newDegrees - oldDegrees;
    if (diff > FULL_ROTATION_DIFF_THRESHOLD) diff = diff - newDegrees;
    if (diff < -FULL_ROTATION_DIFF_THRESHOLD) diff = oldDegrees + diff;
    prevAngle = newDegrees;

    return diff;
}

bool handleIncoming(String data) {
    if (data.startsWith("START")) {
        isConnected = true;
    }
    else if (data.startsWith("END")) {
        isConnected = false;
    }
    else {// unknown data
        return false;
    }
    return true;
}

__attribute__((unused))
void loop() {
    if(Serial.available())
        handleIncoming(Serial.readString());

    int difference = getDifference(prevAngle, readAnalogue());
    if (difference != 0) {
        if (isConnected) {
            printf("NEWDIFF|%d\n", difference);
        }
    }

    delay(DELAY);
}
