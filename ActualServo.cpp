
#include <Servo.h>


Servo tunnelServo;
Servo releaseServo;


const int TUNNEL_SERVO_PIN = 9;
const int RELEASE_SERVO_PIN = 10;

// needs adjusted
const int TUNNEL_OPEN = 0;
const int TUNNEL_CLOSED = 90;

const int RELEASE_OPEN = 90;
const int RELEASE_CLOSED = 0;

// Timing
const unsigned long RELEASE_INTERVAL = 15000;
const unsigned long RELEASE_DURATION = 400;

unsigned long lastReleaseTime = 0;
unsigned long releaseStartTime = 0;

// Variables
int lostBalls = 0;
bool releasing = false;
bool gameOver = false;

void setup() {
  Serial.begin(9600);

  tunnelServo.attach(TUNNEL_SERVO_PIN);
  releaseServo.attach(RELEASE_SERVO_PIN);

  // Initial positions
  tunnelServo.write(TUNNEL_OPEN);
  releaseServo.write(RELEASE_CLOSED);

  lastReleaseTime = millis();
}

void loop() {
  unsigned long currentTime = millis();

  // Servo 1: Close tunnel 
  if (lostBalls >= 5 && !gameOver) {
    gameOver = true;

    tunnelServo.write(TUNNEL_CLOSED);
    releaseServo.write(RELEASE_CLOSED);

    releasing = false;

    Serial.println("GAME OVER!");
  }

  // Servo 2
  if (!gameOver) {

    if (!releasing &&
        currentTime - lastReleaseTime >= RELEASE_INTERVAL) {

      releaseServo.write(RELEASE_OPEN);

      releasing = true;
      releaseStartTime = currentTime;
      lastReleaseTime = currentTime;

      Serial.println("Ball released!");
    }

    // Close servo after 400 ms
    if (releasing &&
        currentTime - releaseStartTime >= RELEASE_DURATION) {

      releaseServo.write(RELEASE_CLOSED);
      releasing = false;
    }
  }
}
