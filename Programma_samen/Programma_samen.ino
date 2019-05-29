//libraries toevoegen
#include <Wire.h>
#include <VL53L0X.h>
#include <NewPing.h>
//dit is een test
//initialisatie ToF
VL53L0X ToF1;
VL53L0X ToF2;

#define MAX_DISTANCE 400

//pin aansluitingen
#define ToF_voeding1 8                                            //aangesloten voeding tof1
#define ToF_voeding2 9                                            //aangesloten voeding tof2
#define stepPinLinks 13                                           //aangesloten step pin
#define dirPinLinks 12                                            //aangesloten dir pin
#define stepPinRechts A1                                          //aangesloten step pin
#define dirPinRechts A0                                           //aangesloten dir pin
#define PING_PIN_1  2                                             // Arduino pin tied to both trigger and echo pins on the ultrasonic sensor.
#define PING_PIN_2  3                                             // Arduino pin tied to both trigger and echo pins on the ultrasonic sensor.
#define PING_PIN_3  4                                             // Arduino pin tied to both trigger and echo pins on the ultrasonic sensor.
#define PING_PIN_4  5                                             // Arduino pin tied to both trigger and echo pins on the ultrasonic sensor.
NewPing sonar1(PING_PIN_1, PING_PIN_1, MAX_DISTANCE);             // NewPing setup of pin and maximum distance.
NewPing sonar2(PING_PIN_2, PING_PIN_2, MAX_DISTANCE);             // NewPing setup of pin and maximum distance.
NewPing sonar3(PING_PIN_3, PING_PIN_3, MAX_DISTANCE);             // NewPing setup of pin and maximum distance.
NewPing sonar4(PING_PIN_4, PING_PIN_4, MAX_DISTANCE);             // NewPing setup of pin and maximum distance.

//global integers sensoren afstanden
uint8_t distanceToF1 = 0;
uint8_t distanceToF2 = 0;
uint8_t distancesonar1 = 0;
uint8_t distancesonar2 = 0;
uint8_t distancesonar3 = 0;
uint8_t distancesonar4 = 0;

//globale integers voor statussen
uint8_t sensornummer = 0;
uint8_t rijtype = 0;
boolean statusStepperLinks = false;
boolean statusStepperRechts = false;

//variabele millis
unsigned long previousMillisToF = 0;
uint8_t intervalToF = 50;

unsigned long previousMillisSonar = 0;
uint8_t intervalsonar = 50;

unsigned long previousMillisStepperLinks = 0;
unsigned long previousMillisStepperRechts = 0;
uint16_t intervalStepper = 15;
uint8_t intervalLaagzetten = 1;


void setup() {
  Serial.begin(115200);
  Wire.begin();

  pinMode(ToF_voeding1, OUTPUT);
  pinMode(ToF_voeding2, OUTPUT);

  //reset ToF door voeding te stoppen
  digitalWrite(ToF_voeding1, LOW);
  digitalWrite(ToF_voeding2, LOW);
  delay(100);

  //start ToF door voeding te geven
  digitalWrite(ToF_voeding1, HIGH);
  digitalWrite(ToF_voeding2, HIGH);
  delay(100);

  //nieuw adres aan tof1
  digitalWrite(ToF_voeding2, LOW);
  delay(100);
  ToF1.init();
  ToF1.setAddress(0x30);
  ToF1.setTimeout(500);

  //nieuw adres aan tof2
  digitalWrite(ToF_voeding2, HIGH);
  ToF2.init();
  ToF2.setAddress(0x31);
  ToF2.setTimeout(500);

  //stepper pins als output zetten
  pinMode(stepPinLinks, OUTPUT);
  pinMode(dirPinLinks, OUTPUT);
  pinMode(stepPinRechts, OUTPUT);
  pinMode(dirPinRechts, OUTPUT);
}

void loop() {
  //updaten sensoren
  sonar();
  ToF();
  rijtype = 0;
  rijden();
}

void sonar() {
  //updaten Sonars
  if (millis() - previousMillisSonar >= intervalsonar) {
    previousMillisSonar = millis();
    switch (sensornummer) {
      case 0:
        distancesonar1 = sonar1.ping_cm();
        break;
      case 1:
        distancesonar2 = sonar2.ping_cm();
        break;
      case 2:
        distancesonar3 = sonar3.ping_cm();
        break;
      case 3:
        distancesonar4 = sonar4.ping_cm();
        break;
    }
    if (sensornummer >= 3) {
      sensornummer = 0;
    }
    else {
      sensornummer++;
    }
  }
}

void ToF() {
  if (millis() - previousMillisToF >= intervalToF) {
    previousMillisToF = millis();
    distanceToF1 = ToF1.readRangeSingleMillimeters();
    distanceToF2 = ToF2.readRangeSingleMillimeters();
  }
}
void rijden() {
  switch (rijtype) {
    case 0: //vooruit
      digitalWrite(dirPinRechts, HIGH);
      digitalWrite(dirPinLinks, HIGH);
      stepperlinks();
      stepperechts();
      break;
    case 1: //achteruit
      digitalWrite(dirPinRechts, LOW);
      digitalWrite(dirPinLinks, LOW);
      stepperlinks();
      stepperechts();
      break;
    case 2: //links
      digitalWrite(dirPinRechts, HIGH);
      digitalWrite(dirPinRechts, HIGH);
      break;
    case 3: //rechts
      digitalWrite(dirPinLinks, HIGH);
      digitalWrite(dirPinLinks, HIGH);

      break;
  }
}



void stepperlinks() {
  if (millis() - previousMillisStepperLinks >= intervalStepper && statusStepperLinks == false) {
    digitalWrite(stepPinLinks, HIGH);
    previousMillisStepperLinks = millis();
    statusStepperLinks = true;
  }
  if (millis() - intervalLaagzetten >= intervalStepper && statusStepperLinks == true) {
    digitalWrite(stepPinLinks, LOW);
    previousMillisStepperLinks = millis();
    statusStepperLinks = false;
  }
}


void stepperechts() {
  if (millis() - previousMillisStepperRechts >= intervalStepper && statusStepperRechts == false) {
    digitalWrite(stepPinLinks, HIGH);
    previousMillisStepperRechts = millis();
    statusStepperRechts = true;
  }
  if (millis() - intervalLaagzetten >= intervalStepper && statusStepperRechts == true) {
    digitalWrite(stepPinLinks, LOW);
    previousMillisStepperRechts = millis();
    statusStepperRechts = false;
  }
}
