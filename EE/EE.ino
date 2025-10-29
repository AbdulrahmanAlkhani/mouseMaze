
#include <NewPing.h>
#include <ESP32Encoder.h>
#include "solver.h"
#include "queue.h"

// Left sensor 
#define TRIG_LEFT 5
#define ECHO_LEFT 17

// front sensor
#define TRIG_FRONT 16
#define ECHO_FRONT 4

// right sensor
#define TRIG_RIGHT 15
#define ECHO_RIGHT 2

#define MAX_DISTANCE 20

ESP32Encoder encoder1;
ESP32Encoder encoder2;

NewPing sonarLeft(TRIG_LEFT, ECHO_LEFT, MAX_DISTANCE);
NewPing sonarFront(TRIG_FRONT, ECHO_FRONT, MAX_DISTANCE);
NewPing sonarRight(TRIG_RIGHT, ECHO_RIGHT, MAX_DISTANCE);

// Left motor control
#define ENA 12
#define IN1 27
#define IN2 14

// right motor control
#define ENB 33
#define IN3 25
#define IN4 26

const float targetDistance = 6;

const float kp = 14;
const float kd = 20;
const float ki = 0.1;
float integral = 0;

const int baseDelay = 5;
int baseSpeed = 210;

float lastError = 0;

void setup() {

  Serial.begin(115200);

  // initialize the Left motor
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  // initialize the right motor
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
}

void loop() {
  // right - left - forward detection
  int iswallRight = wallRight();
  int iswallFront = wallFront();
  int iswallLeft = wallRight();

  Action act = slover();

  if (act == LEFT) {
    turnLeft();
  }
  else if (act == RIGHT) {
    turnRight();
  }
  else if (act == FORWARD) {
    moveF();
  }
  else if (act == BACK) {
    moveB();
  }

  moveF();

  // sensor reading in cm
  float leftDistance = sonarLeft.ping_cm();
  float frontDistance = sonarFront.ping_cm();
  float rightDistance = sonarRight.ping_cm();


  if (leftDistance == 0) leftDistance = MAX_DISTANCE;
  if (frontDistance >= 0 && frontDistance < 1) frontDistance = 30;
  if (rightDistance == 0) rightDistance = MAX_DISTANCE;


  // PID control
  float error = targetDistance - leftDistance;

  integral = 0.7 * integral + ki * error;

  float derivative = error - lastError;

  float control = kp * error + kd * derivative + integral;

  // adjust motor speeds
  int speedA = baseSpeed + control;
  int speedB = baseSpeed - control;

  // speeds constrains
  speedA = constrain(speedA, 90, 255);
  speedB = constrain(speedB, 90, 255);

  analogWrite(ENA, speedA);
  analogWrite(ENB, speedB);

  lastError = error;

  if (frontDistance < 4) {

    if (rightDistance > 10) {
      turnRight();
      StepHalfCell();

    } else if (leftDistance > 10) {
      turnLeft();
      StepHalfCell();
    } else if (leftDistance < 7 && rightDistance < 5) {
    
       Pause();
        moveB();
        delay(740);
        Pause();

        if (leftDistance > 3) {
          turnLeft();
           
        } 
        else if (rightDistance >3) 
        {
          turnRight();
          
        }
      }
    
  }
  else if (frontDistance > 20 )
  {
    if(leftDistance<=7&&rightDistance==20)
    {
      StepHalfCell();
      turnRight();
    }
  }

  // Serial.print(speedA);
  // Serial.print("   ");
  // Serial.println(speedB);
  // Serial.println("  ");
  // Serial.println(error);
  delay(baseDelay);
  Serial.println(rightDistance);
  Serial.println(leftDistance);
  Serial.println(frontDistance);
}

int wallRight() {
  float rightDistance = sonarRight.ping_cm();

  if (rightDistance > 0 && rightDistance < 5) {
    return 1;
  } else return 0;
}

int wallLeft() {
  float leftDistance = sonarLeft.ping_cm();
  if (leftDistance > 0 && leftDistance < 5) {
    return 1;
  } else return 0;
}

int wallFront() {
  float frontDistance = sonarFront.ping_cm();
  Serial.println(frontDistance);
  if (frontDistance > 0 && frontDistance < 5) {
    return 1;
  } else return 0;
}


void Pause()
{ 
     digitalWrite(IN1, LOW);
     digitalWrite(IN2, LOW);
     digitalWrite(IN3, LOW);
     digitalWrite(IN4, LOW);
  delay (400);
}
void turnRight() {

  Pause();

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, 255);
  analogWrite(ENB, 255);
  delay(500);

  Pause();
  moveF();
}
void StepHalfCell() {
  Pause();

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 240);
  analogWrite(ENB, 230);
  delay(360);

  Pause();
  moveF();
}
void turnLeft() {

  Pause();

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 255);
  analogWrite(ENB, 255);
  delay(490);
  Pause();
  moveF();
}

void moveF() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
    analogWrite(ENA, 240);
  analogWrite(ENB, 230);
}
void moveB() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
    analogWrite(ENA, 240);
  analogWrite(ENB, 230);
}
