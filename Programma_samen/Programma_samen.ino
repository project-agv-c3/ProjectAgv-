//libraries toevoegen
#include <Wire.h>
#include <VL53L0X.h>
#include <NewPing.h>
//dit is een test
//initialisatie ToF
VL53L0X ToF1;
VL53L0X ToF2;
VL53L0X ToF3;
VL53L0X ToF4;

//maximale distance sonar
#define MAX_DISTANCE 400

//pin aansluitingen
#define ToF_voeding1 8                                            //aangesloten voeding tof1
#define ToF_voeding2 9                                            //aangesloten voeding tof2
#define ToF_voeding3 6
#define ToF_voeding4 7
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
uint8_t distanceToF3 = 0;
uint8_t distanceToF4 = 0;
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
  pinMode(ToF_voeding3, OUTPUT);
  pinMode(ToF_voeding4, OUTPUT);

  //reset ToF door voeding te stoppen
  digitalWrite(ToF_voeding1, LOW);
  digitalWrite(ToF_voeding2, LOW);
  digitalWrite(ToF_voeding3, LOW);
  digitalWrite(ToF_voeding4, LOW);
  delay(10);

  //start ToF door voeding te geven
  digitalWrite(ToF_voeding1, HIGH);
  digitalWrite(ToF_voeding2, HIGH);
  digitalWrite(ToF_voeding3, HIGH);
  digitalWrite(ToF_voeding4, HIGH);
  delay(10);

  //nieuw adres aan tof1
  digitalWrite(ToF_voeding2, LOW);
  digitalWrite(ToF_voeding3, LOW);
  digitalWrite(ToF_voeding4, LOW);
  delay(10);
  ToF1.init();
  ToF1.setAddress(0x30);
  ToF1.setTimeout(500);

  //nieuw adres aan tof2
  digitalWrite(ToF_voeding2, HIGH);
  delay(10);
  ToF2.init();
  ToF2.setAddress(0x31);
  ToF2.setTimeout(500);

  //nieuw adres aand Tof3
  digitalWrite(ToF_voeding3, HIGH);
  delay(10);
  ToF2.init();
  ToF2.setAddress(0x32);
  ToF2.setTimeout(500);

  //nieuw adres aan ToF4
  digitalWrite(ToF_voeding4, HIGH);
  delay(10);
  ToF2.init();
  ToF2.setAddress(0x33);
  ToF2.setTimeout(500);

  //stepper pins als output zetten
  pinMode(stepPinLinks, OUTPUT);
  pinMode(dirPinLinks, OUTPUT);
  pinMode(stepPinRechts, OUTPUT);
  pinMode(dirPinRechts, OUTPUT);

  //Interrupt service routine aanzetten
//set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei();
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
    distanceToF3 = ToF3.readRangeSingleMillimeters();
    distanceToF4 = ToF4.readRangeSingleMillimeters();
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

ISR(TIMER1_COMPA_vect){
  
}
