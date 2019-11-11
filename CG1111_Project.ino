#include "MeMCore.h"
#include <PID_v1.h>

#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_G4  392
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_B4  494
#define NOTE_G5  784
#define NOTE_E5  659
#define NOTE_F5  698

#define IR_LEFT 0
#define IR_RIGHT 1
#define MIC1 2
#define MIC2 3
#define RGBWait 200
#define LDRWait 10

// Motors
MeDCMotor motorL(M1);
MeDCMotor motorR(M2);

// Sensors
MeLineFollower lineFinder(PORT_1);
MeLightSensor lightSensor(PORT_6);
MeUltrasonicSensor ultraSensor(PORT_2);
MeRGBLed led(13);

// Misc
MeBuzzer buzzer;

// Initial motor speeds
int motorSpeedLeft = 175;
int motorSpeedRight = 175;

int finished = 0;

// Floats to hold colour arrays
float colourArray[] = {0, 0, 0};
float whiteArray[] = {974, 807, 724};
float blackArray[] = {225, 167, 151};
float greyDiff[] = {749, 640, 573};

// Moves the mBot forward
void forward() {
  motorL.run(-motorSpeedLeft);
  motorR.run(motorSpeedRight);
}

// Turn the mBot left
void turnLeft() {
  motorL.run(motorSpeedLeft * 0.8);
  motorR.run(motorSpeedRight * 0.8);
  delay(400);
  motorL.stop();
  motorR.stop();
  motorL.run(-motorSpeedLeft);
  motorR.run(motorSpeedRight);
}

// Turn the mBot right
void turnRight() {
  motorL.run(-motorSpeedLeft * 0.8);
  motorR.run(-motorSpeedRight * 0.8);
  delay(400);
  motorL.stop();
  motorR.stop();
  motorL.run(-motorSpeedLeft);
  motorR.run(motorSpeedRight);
}

// U-turn the mBot
void uTurn() {
  motorL.run(motorSpeedLeft * 0.8);
  motorR.run(motorSpeedRight * 0.8);
  delay(800);
  motorL.stop();
  motorR.stop();
  delay(200);
  motorL.run(-motorSpeedLeft);
  motorR.run(motorSpeedRight);
}

// Turn mBot left, go forward, then left again
void turnLeft2() {
  motorL.run(motorSpeedLeft * 0.8);
  motorR.run(motorSpeedRight * 0.8);
  delay(400);
  motorL.stop();
  motorR.stop();

  while (ultraSensor.distanceCm() > 10.5) {
    motorL.run(-motorSpeedLeft);
    motorR.run(motorSpeedRight);
  }

  motorL.stop();
  motorR.stop();
  delay(500);
  turnLeft();
  delay(200);
}

// Turn the mBot right, go forward, then right away
void turnRight2() {
  motorL.run(-motorSpeedLeft * 0.8);
  motorR.run(-motorSpeedRight * 0.8);
  delay(400);
  motorL.stop();
  motorR.stop();

  while (ultraSensor.distanceCm() > 10.5) {
    motorL.run(-motorSpeedLeft);
    motorR.run(motorSpeedRight);
  }

  motorL.stop();
  motorR.stop();
  delay(500);
  turnRight();
  delay(200);
}

int isBlackLine() {
  int sensorState = lineFinder.readSensors();
  if (sensorState == S1_IN_S2_IN || sensorState == S1_IN_S2_OUT || sensorState == S1_OUT_S2_IN) {
    return 1;
  } else {
    return 0;
  }
}

void lightChallenge() {
  int colour;
  colour = getColour();

  switch (colour) {
    case 1: // Red - Turn left
      Serial.println("Red");
      turnLeft();
      break;
    case 2: // Green - Right turn
      Serial.println("Green");
      turnRight();
      break;
    case 5: // Yellow - 180 Turn
      Serial.println("Yellow");
      uTurn();
      break;
    case 4: // Purple - 2 left turns
      Serial.println("Purple");
      turnLeft2();
      break;
    case 3: // Blue - 2 right turns
      Serial.println("Blue");
      turnRight2();
      break;
    case 6: // Black (sound challenge/finish)
      Serial.println("Black");
      soundChallenge();
      break;
  }
}

void soundChallenge() {
  delay(2000);
  int mic_1, mic_2, highest1 = 0, highest2 = 0;

  for (int i = 1; i <= 10; i++) {
    mic_1 = analogRead(MIC1);
    mic_2 = analogRead(MIC2);
    if (mic_1 > highest1) {
      highest1 = mic_1;
    }
    if (mic_2 > highest2) {
      highest2 = mic_2;
    }
  }

  Serial.print("Highest 1: ");
  Serial.println(highest1);
  Serial.print("Highest 2: ");
  Serial.println(highest2);

  //  int mic_1 = analogRead(MIC1);
  //  int mic_2 = analogRead(MIC2);
  //  Serial.print("MIC1: ");
  //  Serial.println(mic_1);
  //  Serial.print("MIC2: ");
  //  Serial.println(mic_2);

  if (highest2 > 200) {
    Serial.println("turn right");
    turnLeft();
  } else if (highest2 > 145) {
    Serial.println("turn left");
    turnRight();
  } else {
    Serial.println("end maze");
    playSound();
    finished = 1;
  }

  delay(500);
}

void offLED() {
  led.setColor(0, 0, 0, 0);
  led.setColor(1, 0, 0, 0);
  led.show();
  delay(RGBWait);
}

void redLED() {
  led.setColor(0, 255, 0, 0);
  led.setColor(1, 255, 0, 0);
  led.show();
  delay(RGBWait);
}

void greenLED() {
  led.setColor(0, 0, 0, 255);
  led.setColor(1, 0, 0, 255);
  led.show();
  delay(RGBWait);
}

void blueLED() {
  led.setColor(0, 0, 255, 0);
  led.setColor(1, 0, 255, 0);
  led.show();
  delay(RGBWait);
}

int getAvgReading(int times) {
  int reading, total = 0;
  for (int i = 0; i < times; i++) {
    reading = lightSensor.read();
    total += reading;
    delay(LDRWait);
  }

  return total / times;
}

int getColour() {
  // Reset array
  for (int c = 0; c <= 2; c++) {
    colourArray[c] = 0;
  }

  offLED();
  redLED();
  colourArray[0] = getAvgReading(5);
  offLED();
  colourArray[0] = (colourArray[0] - blackArray[0]) / greyDiff[0] * 255;

  greenLED();
  colourArray[1] = getAvgReading(5);
  offLED();
  colourArray[1] = (colourArray[1] - blackArray[1]) / greyDiff[1] * 255;

  blueLED();
  colourArray[2] = getAvgReading(5);
  offLED();
  colourArray[2] = (colourArray[2] - blackArray[2]) / greyDiff[2] * 255;

  // Print out RGB values
//  for (int c = 0; c <= 2; c++) {
//    Serial.println(colourArray[c]);
//  }

  if (colourArray[0] < 60) {
    return 6; // black//
  } else if (colourArray[0] >= 120 && colourArray[2] > 85) {
    return 5; // yellow//
  } else if (colourArray[0] > 105 && colourArray[2] < 80) {
    return 1; // red//
  } else {
    if (colourArray[1] < 60) {
      return 2; // green//
    } else {
      if (colourArray[2] > 100) {
        return 3; //blue//
      }
      else {
        return 4;//purple//
      }
    }
  }

  return 1;
}

void playSound() {
  // notes in the melody:
  int melody[] = {
    NOTE_G4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_C5, NOTE_B4, 0,
    NOTE_G4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_D5, NOTE_C5, 0,
    NOTE_G4, NOTE_G4, NOTE_G5, NOTE_E5, NOTE_C5, NOTE_B4, NOTE_A4, 0,
    NOTE_F5, NOTE_F5, NOTE_E5, NOTE_C5, NOTE_D5, NOTE_C5, 0,
  };

  int noteDurations[] = {
    8, 8, 4, 4, 4, 4,
    4,
    8, 8, 4, 4, 4, 4,
    4,
    8, 8, 4, 4, 4, 4, 2,
    8,
    8, 8, 4, 4, 4, 2,
    4,
  };

  for (int thisNote = 0; thisNote < 29; thisNote++) {
    // to calculate the note duration, take one second
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    buzzer.tone(8, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    buzzer.noTone(8);
  }
}

void correctLeft() {
  motorL.stop();
  motorR.stop();
  motorL.run(-motorSpeedLeft - 10);
  motorR.run(-motorSpeedRight - 10);
  delay(10);
  motorL.stop();
  motorR.stop();
}

void correctRight() {
  motorL.stop();
  motorR.stop();
  motorL.run(motorSpeedLeft - 10);
  motorR.run(motorSpeedRight - 10);
  delay(10);
  motorL.stop();
  motorR.stop();
}

void setup() {
  Serial.begin(9600);
  led.setpin(13);
}

void loop() {
  int IRLeft = analogRead(IR_LEFT);
  int IRRight = analogRead(IR_RIGHT);
  
  // Scale input from IR
  IRLeft = IRLeft / 1023 * 5;
  IRRight = IRRight / 1023 * 5;

  if (finished) {
    motorL.stop();
    motorR.stop();
  } else {
    motorL.run(-motorSpeedLeft);
    motorR.run(motorSpeedRight);

    //    Serial.print("IR Left: ");
    //    Serial.println(IRLeft);
    //    Serial.print("IR Right: ");
    //    Serial.println(IRRight);

    if (IRLeft < 3) {
      correctLeft();
    }
    if (IRRight < 3) {
      correctRight();
    }
  }

  // Check for black line
  if (isBlackLine()) {
    motorL.stop();
    motorR.stop();
    delay(500);
    lightChallenge();
    delay(500);
  }

  if (!finished) {
    if (IRLeft < 3) {
      correctLeft();
    }
    if (IRRight < 3) {
      correctRight();
    }
  }
}
