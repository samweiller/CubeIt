/*
CUBEIT
Version 0.1
Sam Weiller

INCLUDES CODE SNIPPETS TAKEN FROM:
'tester' and 'pitchrollheading' example scripts from Adafruit 9DOF built-in demos
'simple' example sript from Adafruit Neopixel built-in demos

*/

// Include accelerometer, neopixel libraries
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_9DOF.h>
#include <Adafruit_NeoPixel.h>

/* Assign a unique ID to the sensors */
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);
Adafruit_L3GD20_Unified       gyro  = Adafruit_L3GD20_Unified(20);

// Set up Neopixel Details
//                                                #, PIN, defaults
Adafruit_NeoPixel pixelsFaceA = Adafruit_NeoPixel(4, 6, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsFaceB = Adafruit_NeoPixel(4, 1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsFaceC = Adafruit_NeoPixel(4, 10, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsFaceD = Adafruit_NeoPixel(4, 9, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsFaceE = Adafruit_NeoPixel(4, 0, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsFaceF = Adafruit_NeoPixel(4, 12, NEO_GRB + NEO_KHZ800);

// A bunch of variable initializations...
int currentAccelX;
int currentAccelY;
int currentAccelZ;
int xStatus;
int yStatus;
int zStatus;
byte faceUpNow;
byte tempUpFace;
byte returnedFace;
byte faceUpLast;
byte theChosenLED;
byte theChosenLED2;
byte theNextFace;
byte gameIsActive = 0;
byte failureIsPossible = 0;
int colorOfLED = 175;
int posAccelThreshold = 9;
int negAccelThreshold = -9;

// ***********************************************************************************************

void setup() {
  Serial.begin(9600);

  // Begin each face of Neopixels
  pixelsFaceA.begin();
  pixelsFaceB.begin();
  pixelsFaceC.begin();
  pixelsFaceD.begin();
  pixelsFaceE.begin();
  pixelsFaceF.begin();

  Serial.println("Initializing CubeIt. Please stand by.");

  /* Initialise the sensors */
  if (!accel.begin())
  {
    Serial.println(F("Ooops, no LSM303 (accelerometer) detected ... Check your wiring!"));
    while (1);
  }
  if (!mag.begin())
  {
    Serial.println("Ooops, no LSM303 (magnetometer) detected ... Check your wiring!");
    while (1);
  }
  if (!gyro.begin())
  {
    Serial.print("Ooops, no L3GD20 (gyroscope) detected ... Check your wiring or I2C ADDR!");
    while (1);
  }

  // Twinkle cube on
  for (int pix = 0; pix < 4; pix++) {
    pixelsFaceA.setPixelColor(pix, random(220) + 20, random(220) + 20, random(220) + 20);
    pixelsFaceA.show();
    delay(random(500));
    pixelsFaceB.setPixelColor(pix, random(220) + 20, random(220) + 20, random(220) + 20);
    pixelsFaceB.show();
    delay(random(220));
    pixelsFaceC.setPixelColor(pix, random(220) + 20, random(220) + 20, random(220) + 20);
    pixelsFaceC.show();
    delay(random(500));
    pixelsFaceD.setPixelColor(pix, random(220) + 20, random(220) + 20, random(220) + 20);
    pixelsFaceD.show();
    delay(random(220));
    pixelsFaceE.setPixelColor(pix, random(220) + 20, random(220) + 20, random(220) + 20);
    pixelsFaceE.show();
    delay(random(500));
    pixelsFaceF.setPixelColor(pix, random(220) + 20, random(220) + 20, random(220) + 20);
    pixelsFaceF.show();
    delay(random(500));
  }

  faceUpNow = determineUpFace();
  Serial.println(faceUpNow); // debug code
  delay(1000);
  lightsOut();
}

// ***********************************************************************************************

void loop() {
  // Check for face currently up
  returnedFace = determineUpFace();

  /* Debugging code
  Serial.print("Face Up: ");
  Serial.println(faceUpNow);
  Serial.print("Returned: ");
  Serial.println(returnedFace);
  Serial.print("Next: ");
  Serial.println(theNextFace);
  */

  // Set constant faces (the "currently up" and "previously up" faces)
  // Checks if constantly checked UP face is the same as "known" UP face
  if (returnedFace != faceUpNow) {

    if (gameIsActive == 0) { // Start the game!
      Serial.println("game activated!");
      gameIsActive = 1;
      failureIsPossible = 0; // does NOT allow player to 'fail' on first turn
    }

    // If UP face has changed, change values of constFaceUp & faceUpLast.
    faceUpLast = faceUpNow;
    faceUpNow = returnedFace;

    // CHECK IF ANSWER IS CORRECT
    // if Correct, next turn; if wrong, goto FAIL line.


    if (faceUpNow != theNextFace && failureIsPossible == 1) {
      failureIsPossible = 0;
      // FAILURE!
      gameIsActive = 0;
      runFailSequence();
    }
    failureIsPossible = 1;
    lightsOut();

    if (gameIsActive == 1) {
      // THIS CODE TAKES CARE OF TURN TO TURN GAME PROGRESSION.
      // STEPS:
      // 1. Choose a random LED
      theChosenLED = random(3);
      // 2. Choose its partner (n+1)
      theChosenLED2 = theChosenLED + 1;

      // If max LED is chosen, circle back to zero
      if (theChosenLED2 == 4) {
        theChosenLED2 = 0;
      }

      // 3. Determine (and log) corresponding face
      theNextFace = chooseNextFace(theChosenLED);

      // 4. Light up correct lights
      switch (faceUpNow) {
        case 0:
          pixelsFaceA.setPixelColor(theChosenLED, 0, colorOfLED, 0);
          pixelsFaceA.setPixelColor(theChosenLED2, 0, colorOfLED, 0);
          pixelsFaceA.show();
          break;
        case 1:
          pixelsFaceB.setPixelColor(theChosenLED, 0, colorOfLED, 0);
          pixelsFaceB.setPixelColor(theChosenLED2, 0, colorOfLED, 0);
          pixelsFaceB.show();
          break;
        case 2:
          pixelsFaceC.setPixelColor(theChosenLED, 0, colorOfLED, 0);
          pixelsFaceC.setPixelColor(theChosenLED2, 0, colorOfLED, 0);
          pixelsFaceC.show();
          break;
        case 3:
          pixelsFaceD.setPixelColor(theChosenLED, 0, colorOfLED, 0);
          pixelsFaceD.setPixelColor(theChosenLED2, 0, colorOfLED, 0);
          pixelsFaceD.show();
          break;
        case 4:
          pixelsFaceE.setPixelColor(theChosenLED, 0, colorOfLED, 0);
          pixelsFaceE.setPixelColor(theChosenLED2, 0, colorOfLED, 0);
          pixelsFaceE.show();
          break;
        case 5:
          pixelsFaceF.setPixelColor(theChosenLED, 0, colorOfLED, 0);
          pixelsFaceF.setPixelColor(theChosenLED2, 0, colorOfLED, 0);
          pixelsFaceF.show();
          break;
        default:
          break;
      }
    }
  }
  delay(10);
}

// ***********************************************************************************************

int determineUpFace() {
  //  Serial.println("Determining Up Face");
  // Determine UP side
  /* Get a new sensor event */
  sensors_event_t event;

  /* Display the results (acceleration is measured in m/s^2) */
  accel.getEvent(&event);

  // 1. Pull out accelerometer data
  // A NOTE: these variables are technically unnecessary, but make code slightly more readable.
  //         Remove these variables if memory becomes an issue.
  currentAccelX = event.acceleration.x;
  currentAccelY = event.acceleration.y;
  currentAccelZ = event.acceleration.z;

  // If the accelerometer value is close to +/-10, set face status to 1
  if (currentAccelX > posAccelThreshold) {
    xStatus = 1;
  } else if (currentAccelX < negAccelThreshold) {
    xStatus = -1;
  } else {
    xStatus = 0;
  }

  if (currentAccelY > posAccelThreshold) {
    yStatus = 1;
  } else if (currentAccelY < negAccelThreshold) {
    yStatus = -1;
  } else {
    yStatus = 0;
  }

  if (currentAccelZ > posAccelThreshold) {
    zStatus = 1;
  } else if (currentAccelZ < negAccelThreshold) {
    zStatus = -1;
  } else {
    zStatus = 0;
  }

  if (xStatus == 1 && yStatus == 0 && zStatus == 0) {
    tempUpFace = 3;
  } else if (xStatus == -1 && yStatus == 0 && zStatus == 0) {
    tempUpFace = 4;
  } else if (xStatus == 0 && yStatus == 1 && zStatus == 0) {
    tempUpFace = 1;
  } else if (xStatus == 0 && yStatus == -1 && zStatus == 0) {
    tempUpFace = 0;
  } else if (xStatus == 0 && yStatus == 0 && zStatus == 1) {
    tempUpFace = 5;
  } else if (xStatus == 0 && yStatus == 0 && zStatus == -1) {
    tempUpFace = 2;
    Serial.println("C is up!");
  } else {

  }

  return tempUpFace;
}

// ***********************************************************************************************

byte chooseNextFace(byte) {
  // Hard coded chooser for next face based up face currently up
  switch (faceUpNow) {
    case 0: // Face A
      switch (theChosenLED) {
        case 0:
          theNextFace = 4; // D
          break;
        case 1:
          theNextFace = 2; // F
          break;
        case 2:
          theNextFace = 3; // E
          break;
        case 3:
          theNextFace = 5; // C
          break;
        default:
          break;
      }
      break;
    case 1: // Face B
      switch (theChosenLED) {
        case 0:
          theNextFace = 4; // D
          break;
        case 1:
          theNextFace = 5; // C
          break;
        case 2:
          theNextFace = 3; // E
          break;
        case 3:
          theNextFace = 2; // F
          break;
        default:
          break;
      }
      break;
    case 2: // Face C
      switch (theChosenLED) {
        case 0:
          theNextFace = 4; // D
          break;
        case 1:
          theNextFace = 1; // A
          break;
        case 2:
          theNextFace = 3; // E
          break;
        case 3:
          theNextFace = 0; // B
          break;
        default:
          break;
      }
      break;
    case 3: // Face D
      switch (theChosenLED) {
        case 0:
          theNextFace = 1; // A
          break;
        case 1:
          theNextFace = 5; // C
          break;
        case 2:
          theNextFace = 0; // B
          break;
        case 3:
          theNextFace = 2; // F
          break;
        default:
          break;
      }
      break;
    case 4: // Face E
      switch (theChosenLED) {
        case 0:
          theNextFace = 1; // A
          break;
        case 1:
          theNextFace = 2; // F
          break;
        case 2:
          theNextFace = 0; // B
          break;
        case 3:
          theNextFace = 5; // C
          break;
        default:
          break;
      }
      break;
    case 5: // Face F
      switch (theChosenLED) {
        case 0:
          theNextFace = 4; // D
          break;
        case 1:
          theNextFace = 0; // B
          break;
        case 2:
          theNextFace = 3; // E
          break;
        case 3:
          theNextFace = 1; // A
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }

  return theNextFace;
}

// ***********************************************************************************************

void runFailSequence() {
  // failure lights

  gameIsActive = 0;

  // "Randomly" turns all lights on to blue
  for (int pix = 0; pix < 4; pix++) {
    pixelsFaceA.setPixelColor(pix, 0, 0, 225);
    pixelsFaceA.show();
    delay(random(100));
    pixelsFaceB.setPixelColor(pix, 0, 0, 225);
    pixelsFaceB.show();
    delay(random(200));
    pixelsFaceC.setPixelColor(pix, 0, 0, 225);
    pixelsFaceC.show();
    delay(random(200));
    pixelsFaceD.setPixelColor(pix, 0, 0, 225);
    pixelsFaceD.show();
    delay(random(400));
    pixelsFaceE.setPixelColor(pix, 0, 0, 225);
    pixelsFaceE.show();
    delay(random(300));
    pixelsFaceF.setPixelColor(pix, 0, 0, 225);
    pixelsFaceF.show();
    delay(random(350));
  }
  delay(1000);

  // fade all lights to zero
  for (int val = 225; val >= 0; val--) {
    for (int pix = 0; pix < 4; pix++) {
      pixelsFaceA.setPixelColor(pix, 0, 0, val);
      pixelsFaceA.show();
      pixelsFaceB.setPixelColor(pix, 0, 0, val);
      pixelsFaceB.show();

      pixelsFaceC.setPixelColor(pix, 0, 0, val);
      pixelsFaceC.show();

      pixelsFaceD.setPixelColor(pix, 0, 0, val);
      pixelsFaceD.show();

      pixelsFaceE.setPixelColor(pix, 0, 0, val);
      pixelsFaceE.show();

      pixelsFaceF.setPixelColor(pix, 0, 0, val);
      pixelsFaceF.show();

    }
  }
}


void lightsOut() {
  // turn off all lights
  for (int k = 0; k < 4; k++) {
    pixelsFaceA.setPixelColor(k, 0, 0, 0);
    pixelsFaceA.show();
    pixelsFaceB.setPixelColor(k, 0, 0, 0);
    pixelsFaceB.show();
    pixelsFaceC.setPixelColor(k, 0, 0, 0);
    pixelsFaceC.show();
    pixelsFaceD.setPixelColor(k, 0, 0, 0);
    pixelsFaceD.show();
    pixelsFaceE.setPixelColor(k, 0, 0, 0);
    pixelsFaceE.show();
    pixelsFaceF.setPixelColor(k, 0, 0, 0);
    pixelsFaceF.show();
  }
}
