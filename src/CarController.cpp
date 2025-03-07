#include "CarController.h"

// Constructor
CarController::CarController(int speed, int correctionFactor)
{
    baseSpeed = speed;
    this->correctionFactor = correctionFactor;
}

// Initialize the motor pins and sensors
void CarController::init()
{
    pinMode(in1, OUTPUT); // right motor
    pinMode(in2, OUTPUT);
    pinMode(in3, OUTPUT); // left motor
    pinMode(in4, OUTPUT);
    pinMode(enA, OUTPUT); // right motor
    pinMode(enB, OUTPUT); // left motor

    // Initialize the sensors
    leftUltrasonic.init();
    rightUltrasonic.init();
    timeOfFlight.init();
    gyroscope.init();
}

void CarController::moveForward()
{
    static float prevTime = millis();

    static float currentAngle = 0;

    unsigned long startTime = millis();
    unsigned long duration = 5000; // 5 seconds duration

    while (millis() - startTime < duration)
    {
        unsigned long currentTime = millis();

        int leftDistance = leftUltrasonic.getDistance();   // Distance from the left wall
        int rightDistance = rightUltrasonic.getDistance(); // Distance from the right wall

        // Get the current angle from the gyroscope
        currentAngle = gyroscope.getAngle(currentTime, &prevTime, currentAngle);

        // Desired distance from each wall is 2cm (with a tolerance of 1cm)
        int targetDistance = 2;
        int tolerance = 1;

        int leftSpeed = baseSpeed;
        int rightSpeed = baseSpeed;

        // Adjust speed based on the distance from the walls
        if (leftDistance < targetDistance - tolerance)
        {
            // Too close to the left wall, slow down left motor or speed up right motor
            leftSpeed = baseSpeed - 50;
            rightSpeed = baseSpeed + 50;
        }
        else if (leftDistance > targetDistance + tolerance)
        {
            // Too far from the left wall, speed up left motor or slow down right motor
            leftSpeed = baseSpeed + 50;
            rightSpeed = baseSpeed - 50;
        }

        if (rightDistance < targetDistance - tolerance)
        {
            // Too close to the right wall, slow down right motor or speed up left motor
            leftSpeed = baseSpeed + 50;
            rightSpeed = baseSpeed - 50;
        }
        else if (rightDistance > targetDistance + tolerance)
        {
            // Too far from the right wall, speed up right motor or slow down left motor
            leftSpeed = baseSpeed - 50;
            rightSpeed = baseSpeed + 50;
        }

        // Adjust speed based on the gyroscope angle to maintain straight movement
        if (currentAngle > 5) // Adjust the threshold angle as needed
        {
            // If the car is veering to the right, correct by slowing down the right motor
            leftSpeed += 50;
            rightSpeed -= 50;
        }
        else if (currentAngle < -5) // Adjust the threshold angle as needed
        {
            // If the car is veering to the left, correct by slowing down the left motor
            leftSpeed -= 50;
            rightSpeed += 50;
        }

        Serial.print("Angle: ");
        Serial.println(currentAngle);
        Serial.print("Left Distance: ");
        Serial.println(leftDistance);
        Serial.print("Right Distance: ");
        Serial.println(rightDistance);
        Serial.print("Left Speed: ");
        Serial.println(leftSpeed);
        Serial.print("Right Speed: ");
        Serial.println(rightSpeed);

        // Move the car forward with the adjusted speeds
        // digitalWrite(in1, HIGH);
        // digitalWrite(in2, LOW);
        // digitalWrite(in3, HIGH);
        // digitalWrite(in4, LOW);

        // analogWrite(enA, leftSpeed);
        // analogWrite(enB, rightSpeed);

        // Small delay to allow sensor readings to update
        delay(10);
    }

    // Stop the car after 5 seconds
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    digitalWrite(in3, LOW);
    digitalWrite(in4, LOW);
}

// Function to move forward with balancing
void CarController::forward(int leftDistance, int rightDistance)
{
    int error = leftDistance - rightDistance;
    int speedLeft = baseSpeed - (error * correctionFactor);
    int speedRight = baseSpeed + (error * correctionFactor);

    speedLeft = constrain(speedLeft, 0, 255);
    speedRight = constrain(speedRight, 0, 255);

    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);

    analogWrite(enA, speedLeft);
    analogWrite(enB, speedRight);
}

// Function to test forward movement
void CarController::forwardTest(int speed)
{
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);

    analogWrite(enA, speed);
    analogWrite(enB, speed);
}

// Function to turn left by a specific angle
void CarController::turnLeft(int angle)
{
    int turnSpeed = map(angle, 0, 180, baseSpeed, 0);

    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH); // Reverse left motor
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW); // Forward right motor

    analogWrite(enA, turnSpeed);
    analogWrite(enB, baseSpeed);
}

// Function to turn right by a specific angle
void CarController::turnRight(int angle)
{
    int turnSpeed = map(angle, 0, 180, baseSpeed, 0);

    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW); // Forward left motor
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH); // Reverse right motor

    analogWrite(enA, baseSpeed);
    analogWrite(enB, turnSpeed);
}

// Function to stop the car
void CarController::stop()
{
    analogWrite(enA, 0);
    analogWrite(enB, 0);
}

// Function to identify if there is a wall in front
bool CarController::wallFront()
{
    int distance = timeOfFlight.getDistance(0); // Use sensor 0 (front)
    return distance < frontThreshold;           // Adjust threshold as needed
}

// Function to identify if there is a wall on the left
bool CarController::wallLeft()
{
    int distance = leftUltrasonic.getDistance();
    return distance < leftThreshold; // Adjust threshold as needed
}

// Function to identify if there is a wall on the right
bool CarController::wallRight()
{
    int distance = rightUltrasonic.getDistance();
    return distance < rightThreshold; // Adjust threshold as needed
}

// Function to identify if there is a wall at the back
bool CarController::wallBack()
{
    int distance = timeOfFlight.getDistance(1); // Use sensor 1 (back)
    return distance < backThreshold;            // Adjust threshold as needed
}

// Function to calculate the current cell in the maze
int CarController::getCurrentCell()
{
    int frontDistance = timeOfFlight.getDistance(0); // Front sensor
    int backDistance = timeOfFlight.getDistance(1);  // Back sensor

    // Example logic to determine the current cell based on distances
    if (frontDistance < 100 && backDistance < 100)
    {
        return 1; // Example: Current cell ID 1
    }
    else if (frontDistance < 100)
    {
        return 2; // Example: Current cell ID 2
    }
    else if (backDistance < 100)
    {
        return 3; // Example: Current cell ID 3
    }
    else
    {
        return 0; // Default cell (no walls nearby)
    }
}

void CarController::printTimeOfFlightValues()
{
    int frontDistance = timeOfFlight.getDistance(0); // Front sensor
    int backDistance = timeOfFlight.getDistance(1);  // Back sensor

    Serial.print("Front Sensor Distance: ");
    Serial.print(frontDistance / 10);
    Serial.println(" cm");

    Serial.print("Back Sensor Distance: ");
    Serial.print(backDistance / 10);
    Serial.println(" cm");
}

// Functions to move the car in the maze
void CarController::moveNorth(int direction)
{
    if (direction == 0)
    {
        forwardTest(baseSpeed); // Move forward
    }
    else if (direction == 1)
    {
        turnLeft(90);
        forwardTest(baseSpeed);
    }
    else if (direction == 2)
    {
        turnLeft(180);
        forwardTest(baseSpeed);
    }
    else if (direction == 3)
    {
        turnRight(90);
        forwardTest(baseSpeed);
    }
}

void CarController::moveEast(int direction)
{
    if (direction == 0)
    {
        turnRight(90);
        forwardTest(baseSpeed);
    }
    else if (direction == 1)
    {
        forwardTest(baseSpeed);
    }
    else if (direction == 2)
    {
        turnLeft(90);
        forwardTest(baseSpeed);
    }
    else if (direction == 3)
    {
        turnLeft(180);
        forwardTest(baseSpeed);
    }
}

void CarController::moveSouth(int direction)
{
    if (direction == 0)
    {
        turnLeft(180);
        forwardTest(baseSpeed);
    }
    else if (direction == 1)
    {
        turnRight(90);
        forwardTest(baseSpeed);
    }
    else if (direction == 2)
    {
        forwardTest(baseSpeed);
    }
    else if (direction == 3)
    {
        turnLeft(90);
        forwardTest(baseSpeed);
    }
}

void CarController::moveWest(int direction)
{
    if (direction == 0)
    {
        turnLeft(90);
        forwardTest(baseSpeed);
    }
    else if (direction == 1)
    {
        turnLeft(180);
        forwardTest(baseSpeed);
    }
    else if (direction == 2)
    {
        turnRight(90);
        forwardTest(baseSpeed);
    }
    else if (direction == 3)
    {
        forwardTest(baseSpeed);
    }
}