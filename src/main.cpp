#include <Arduino.h>
#include <Servo.h>
#include "Joystick.h"
#include "LibPrintf.h"
#include "EEPROM.h"
#include "ResponsiveAnalogRead.h"

#define SERVO_PIN 9
#define ANALOG_PIN A0
#define DELAY 100
#define MAX_ANALOG 1023.0
#define MAX_HALL_ANGLE_PER_TURN 360
#define MIN_SERVO_ANGLE 71
#define MAX_SERVO_ANGLE 109
//The difference value that cannot be exceeded. Used to detect the sensor has been fully rotated or not
#define FULL_ROTATION_DIFF_THRESHOLD (MAX_HALL_ANGLE_PER_TURN / 100) * 60

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD,
                   0, 0,                  // Button Count, Hat Switch Count
                   false, false  , false,     // X and Y, but no Z Axis
                   true, false, false,   // No Rx, Ry, or Rz
                   false, false,          // No rudder or throttle
                   false, false, false);  // No accelerator, brake, or steering

Servo servo;
volatile int minJoystickValue = 0;
volatile int maxJoystickValue = 1500;
volatile bool isConnected = false;
volatile int prevAngle = 0;
// 784 is about 1 degree on cessna 152 using -20 - 20 as the limits from simconnect values
volatile int joystickValue = 784;
ResponsiveAnalogRead sensorReader(ANALOG_PIN, true);

int readAnalogue() {
    sensorReader.update();
    return sensorReader.getValue() / MAX_ANALOG * MAX_HALL_ANGLE_PER_TURN;
}

__attribute__((unused))
void setup() {
    EEPROM.get(0, maxJoystickValue);
    Serial.begin(115200);
    servo.attach(SERVO_PIN);
    Joystick.begin();
    Joystick.setRxAxisRange(minJoystickValue, maxJoystickValue);
    prevAngle = readAnalogue();
    // -20 and 20 are the min and max trim angles in a cesna 152
    joystickValue = map(1, -20, 20, minJoystickValue, maxJoystickValue);
}

int getDifference(int oldDegrees, int newDegrees) {
    int diff = newDegrees - oldDegrees;
    if (diff > FULL_ROTATION_DIFF_THRESHOLD) diff = diff - newDegrees;
    if (diff < -FULL_ROTATION_DIFF_THRESHOLD) diff = oldDegrees + diff;
    prevAngle = newDegrees;

    return diff;
}

int limitJoystickValue(int oldValue, int angleDifference) {
    return min(maxJoystickValue, max(minJoystickValue, oldValue + angleDifference));
}

int getNewJoystickValue(int current, int newSensitivity) {
    int asPercentage = map(current, minJoystickValue, maxJoystickValue, 0, 100);
    return map(asPercentage, 0, 100, minJoystickValue, newSensitivity);
}

void setNewSensitivity(int sens){
    int newJoystickValue = getNewJoystickValue(joystickValue, sens);
    maxJoystickValue = sens;
    Joystick.setRxAxisRange(minJoystickValue, maxJoystickValue);
    joystickValue = newJoystickValue;
    printf("GETSENS|%i\n", maxJoystickValue);
}

bool handleIncoming(String data) {
    if (data.startsWith("START")) {
        isConnected = true;
        printf("GETSENS|%i\n", maxJoystickValue);
    }
    else if (data.startsWith("END")) {
        //if(isConnected)
          //  EEPROM.put(0, maxJoystickValue);
        isConnected = false;
    }
    else if (data.startsWith("SETSENS")) {
        setNewSensitivity(data.substring(8).toInt());
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
    joystickValue = limitJoystickValue(joystickValue,difference);
    servo.write(map(joystickValue, minJoystickValue, maxJoystickValue, MIN_SERVO_ANGLE, MAX_SERVO_ANGLE));
    Joystick.setRxAxis(joystickValue);

    delay(DELAY);
}
