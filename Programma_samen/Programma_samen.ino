//Libraries toevoegen
#include <Wire.h>
#include <VL53L0X.h>
#include <NewPing.h>
#include <SoftwareSerial.h>

//Initialisatie van de ToFs
VL53L0X ToF1;
VL53L0X ToF2;
VL53L0X ToF3;
VL53L0X ToF4;

//De maximale afstanden
#define MAX_SONAR 400           //Voor de ultrasone sensoren is dit een afstand in centimeters
#define MAX_TOF 500             //Voor de ToFs is dit een timeout

//De pin aansluitingen
#define TOF_PIN1 8              //De aansluitingen voor de voedingen van de ToFs
#define TOF_PIN2 9
#define TOF_PIN3 6
#define TOF_PIN4 7
#define stepPinLinks 13         //De vier aansluitingen voor de stepper drivers (twee per motor)
#define dirPinLinks 12
#define stepPinRechts A1
#define dirPinRechts A0
#define PING_PIN1 2             //De vier aansluitingen voor de ultrasone sensoren (één per sensor)
#define PING_PIN2 3
#define PING_PIN3 4
#define PING_PIN4 5

//De adressen voor de ToFs
#define TOF1 0x30
#define TOF2 0x31
#define TOF3 0x32
#define TOF4 0x33

//Het aanmaken van de NewPing objecten voor de ultrasone sensoren
NewPing sonar1(PING_PIN1, PING_PIN1, MAX_SONAR);
NewPing sonar2(PING_PIN2, PING_PIN2, MAX_SONAR);
NewPing sonar3(PING_PIN3, PING_PIN3, MAX_SONAR);
NewPing sonar4(PING_PIN4, PING_PIN4, MAX_SONAR);

//Globale integers sensoren afstanden
uint8_t distanceToF1 = 0;
uint8_t distanceToF2 = 0;
uint8_t distanceToF3 = 0;
uint8_t distanceToF4 = 0;
uint8_t distancesonar1 = 0;
uint8_t distancesonar2 = 0;
uint8_t distancesonar3 = 0;
uint8_t distancesonar4 = 0;

//Globale integers voor statussen
uint8_t sensornummer = 0;
boolean statusStepperLinks = false;
boolean statusStepperRechts = false;
boolean richtingStepperLinks = true;
boolean richtingStepperRechts = true;

//variabele millis
unsigned long previousMillisToF = 0;
#define intervalToF 50

unsigned long previousMillisSonar = 0;
#define intervalsonar 50

unsigned long previousMillisStepperLinks = 0;
unsigned long previousMillisStepperRechts = 0;
int16_t intervalStepperLinks = 15;
int16_t intervalStepperRechts = 15;
#define intervalLaagzetten 1

void setup() {
  Serial.begin(9600);   //Start een seriele verbinding met de computer voor debugging
  Wire.begin();         //Start de I2C comunicatie

  //Zet alle output pinnen als outputs (dit is voor de ultrasone sensoren niet nodig, dit wordt gedaan door de library)
  pinMode(TOF_PIN1, OUTPUT);
  pinMode(TOF_PIN2, OUTPUT);
  pinMode(TOF_PIN3, OUTPUT);
  pinMode(TOF_PIN4, OUTPUT);
  pinMode(stepPinLinks, OUTPUT);
  pinMode(dirPinLinks, OUTPUT);
  pinMode(stepPinRechts, OUTPUT);
  pinMode(dirPinRechts, OUTPUT);

  ToFs_init();          //Initializeer de time of flight sensoren
/*
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
  */
}

void loop() {
  //updaten sensoren
  sonar();
  ToF();
  stepperlinks();
  stepperrechts();
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

void stepperlinks() {
  if (intervalStepperLinks != 0) {
    if ((intervalStepperLinks < 0) == richtingStepperLinks) {
      richtingStepperLinks != richtingStepperLinks;
      digitalWrite(dirPinLinks, richtingStepperLinks);
    }
    if (millis() - previousMillisStepperLinks >= intervalStepperLinks && statusStepperLinks == false) {
      digitalWrite(stepPinLinks, HIGH);
      previousMillisStepperLinks = millis();
      statusStepperLinks = true;
    }
    if (millis() - intervalLaagzetten >= intervalStepperLinks && statusStepperLinks == true) {
      digitalWrite(stepPinLinks, LOW);
      previousMillisStepperLinks = millis();
      statusStepperLinks = false;
    }
  } else {
    if (statusStepperLinks == true) {
      digitalWrite(stepPinLinks, LOW);
      statusStepperLinks = false;
    }
  }
}

void stepperrechts() {
  if (intervalStepperRechts != 0) {
    if ((intervalStepperRechts < 0) == richtingStepperRechts) {
      richtingStepperRechts != richtingStepperRechts;
      digitalWrite(dirPinRechts, richtingStepperRechts);
    }
    if (millis() - previousMillisStepperRechts >= intervalStepperRechts && statusStepperRechts == false) {
      digitalWrite(stepPinLinks, HIGH);
      previousMillisStepperRechts = millis();
      statusStepperRechts = true;
    }
    if (millis() - intervalLaagzetten >= intervalStepperRechts && statusStepperRechts == true) {
      digitalWrite(stepPinLinks, LOW);
      previousMillisStepperRechts = millis();
      statusStepperRechts = false;
    }
  } else {
    if (statusStepperRechts == true) {
      digitalWrite(stepPinRechts, LOW);
      statusStepperRechts = false;
    }
  }
}

void ToFs_init() {
  //Reset de ToFs door de voedingen laag te maken
  digitalWrite(TOF_PIN1, LOW);
  digitalWrite(TOF_PIN2, LOW);
  digitalWrite(TOF_PIN3, LOW);
  digitalWrite(TOF_PIN4, LOW);
  delay(10);

  //Start de ToFs door de voedingen hoog te maken
  digitalWrite(TOF_PIN1, HIGH);
  digitalWrite(TOF_PIN2, HIGH);
  digitalWrite(TOF_PIN3, HIGH);
  digitalWrite(TOF_PIN4, HIGH);
  delay(10);

  //Geef ToF1 een nieuw adress
  digitalWrite(TOF_PIN2, LOW);
  digitalWrite(TOF_PIN3, LOW);
  digitalWrite(TOF_PIN4, LOW);
  delay(10);
  ToF1.init();
  ToF1.setAddress(TOF1);
  ToF1.setTimeout(MAX_TOF);

  //Geef ToF2 een nieuw adress
  digitalWrite(TOF_PIN2, HIGH);
  delay(10);
  ToF2.init();
  ToF2.setAddress(TOF2);
  ToF2.setTimeout(MAX_TOF);

  //Geef ToF3 een nieuw adress
  digitalWrite(TOF_PIN3, HIGH);
  delay(10);
  ToF2.init();
  ToF2.setAddress(TOF3);
  ToF2.setTimeout(MAX_TOF);

  //Geef ToF4 een nieuw adress
  digitalWrite(TOF_PIN4, HIGH);
  delay(10);
  ToF2.init();
  ToF2.setAddress(TOF4);
  ToF2.setTimeout(MAX_TOF);
}
