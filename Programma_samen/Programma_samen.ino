//Libraries toevoegen
#include <Wire.h>
#include <VL53L0X.h>
#include <NewPing.h>
#include <SoftwareSerial.h>

//De maximale afstanden
#define MAX_SONAR 400           //Voor de ultrasone sensoren is dit een afstand in centimeters
#define MAX_TOF 500             //Voor de ToFs is dit een timeout

//De pin aansluitingen
#define TOF_PIN1 8              //De aansluitingen voor de voedingen van de ToFs
#define TOF_PIN2 9
#define TOF_PIN3 6
#define TOF_PIN4 7
#define stepPinLinks A1         //De vier aansluitingen voor de stepper drivers (twee per motor)
#define dirPinLinks A0
#define stepPinRechts 13
#define dirPinRechts 12
#define PING_PIN1 2             //De vier aansluitingen voor de ultrasone sensoren (één per sensor)
#define PING_PIN2 3
#define PING_PIN3 4
#define PING_PIN4 5
#define VOLTAGE_PIN A6
#define TX_PIN A3
#define RX_PIN A2

//De adressen voor de ToFs
#define TOF1 0x30
#define TOF2 0x31
#define TOF3 0x32
#define TOF4 0x33

//De defines voor de btState variabele
#define NOT_CONNECTED 0
#define MAKING_CONNECTION 1
#define CONNECTED 2

//Verschillende codes die via bluetooth verstuurd worden
#define INIT_DATA 0x56
#define INIT_RESPONSE 0x57
#define CON_RESPONSE 0x58

//Initialisatie van de ToFs
VL53L0X ToF1;
VL53L0X ToF2;
VL53L0X ToF3;
VL53L0X ToF4;

//Het aanmaken van het seriele object voor de bluetooth communicatie
SoftwareSerial bluetooth = SoftwareSerial(RX_PIN, TX_PIN);

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
uint8_t sonarNummer = 0;
uint8_t tofNummer = 0;
boolean statusStepperLinks = false;
boolean statusStepperRechts = false;
boolean richtingStepperLinks = false;
boolean richtingStepperRechts = false;
boolean Timer = false;

long positieLinks = 0;
long positieRechts = 0;
#define CORNER_STEPS 340
int done = 0;

//variabele millis
unsigned long previousMillisToF = 0;
#define intervalToF 12

unsigned long previousMillisSonar = 6;
#define intervalsonar 12

unsigned long previousMillisStepperLinks = 0;
unsigned long previousMillisStepperRechts = 0;
int16_t intervalStepperLinks = 15;
int16_t intervalStepperRechts = 15;
#define intervalLaagzetten 1

uint8_t state = 0;
boolean emergency = false;
uint8_t btState = NOT_CONNECTED;

void setup() {
  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, OUTPUT);
  bluetooth.begin(9600);  //Start de seriele connectie met de bluetooth module
  Serial.begin(9600);     //Start een seriele verbinding met de computer voor debugging
  Wire.begin();           //Start de I2C comunicatie

  //Zet alle output pinnen als outputs (dit is voor de ultrasone sensoren niet nodig, dit wordt gedaan door de library)
  pinMode(TOF_PIN1, OUTPUT);
  pinMode(TOF_PIN2, OUTPUT);
  pinMode(TOF_PIN3, OUTPUT);
  pinMode(TOF_PIN4, OUTPUT);
  pinMode(stepPinLinks, OUTPUT);
  pinMode(dirPinLinks, OUTPUT);
  pinMode(stepPinRechts, OUTPUT);
  pinMode(dirPinRechts, OUTPUT);

  state = 1;

  ToFs_init();          //Initializeer de time of flight sensoren

  //Interrupt service routine aanzetten
  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 5hz increments
  OCR1A = 3124;// = (16*10^6) / (5*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  sei();

}

void loop() {
  if (!emergency) {
    //updaten sensoren
    sonar();
    ToF();

    switch (state) {
      case 1:
        positieLinks = 0;
        positieRechts = 0;
        intervalStepperLinks = 2;
        intervalStepperRechts = 2;
        done = 0;
        state = 2;
        break;
      case 2:
        if (positieLinks == 1000) {
          intervalStepperLinks = 0;
          done++;
        }
        if (positieRechts == 1000) {
          intervalStepperRechts = 0;
          done++;
        }
        if (done >= 2) {
          state = 3;
        }
        break;
      default:
        //Nothing
        break;
    }

    if (Timer == true) {
      stepperLinks();
      stepperRechts();
      Timer = false;
    }


    if (analogRead(VOLTAGE_PIN) <= 10) {
      emergency = true;
    }
  } else {
    if (analogRead(VOLTAGE_PIN) > 10) {
      emergency = false;
    }
  }

  if (btState == CONNECTED) {
  }
  else {
    if (bluetooth.available()) {
      if (btState == NOT_CONNECTED) {
        if (bluetooth.read() == INIT_DATA) {
          bluetooth.write(INIT_RESPONSE);
          btState = MAKING_CONNECTION;
        }
      } else if (btState == MAKING_CONNECTION) {
        if (bluetooth.read() == CON_RESPONSE) {
          btState = CONNECTED;
        }
      }
    }
  }

}

void sonar() {
  //updaten Sonars
  if (millis() - previousMillisSonar >= intervalsonar) {
    previousMillisSonar = millis();
    switch (sonarNummer) {
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
    if (sonarNummer >= 3) {
      sonarNummer = 0;
    } else {
      sonarNummer++;
    }
  }
}

void ToF() {
  if (millis() - previousMillisToF >= intervalToF) {
    previousMillisToF = millis();
    switch (tofNummer) {
      case 0:
        distanceToF1 = ToF1.readRangeSingleMillimeters();
        break;
      case 1:
        distanceToF2 = ToF2.readRangeSingleMillimeters();
        break;
      case 2:
        distanceToF3 = ToF3.readRangeSingleMillimeters();
        break;
      case 3:
        distanceToF4 = ToF4.readRangeSingleMillimeters();
        break;
    }
    if (tofNummer >= 3) {
      tofNummer = 0;
    } else {
      tofNummer++;
    }
  }
}

void stepperLinks() {
  if (intervalStepperLinks != 0) {
    if ((intervalStepperLinks > 0) != richtingStepperLinks) {
      richtingStepperLinks = !richtingStepperLinks;
      digitalWrite(dirPinLinks, richtingStepperLinks);
    }
    if (millis() - previousMillisStepperLinks >= abs(intervalStepperLinks) && statusStepperLinks == false) {
      if (intervalStepperLinks < 0) {
        positieLinks--;
      } else {
        positieLinks++;
      }
      digitalWrite(stepPinLinks, HIGH);
      previousMillisStepperLinks = millis();
      statusStepperLinks = true;
    }
    if (millis() - intervalLaagzetten >= abs(intervalStepperLinks) && statusStepperLinks == true) {
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

void stepperRechts() {
  if (intervalStepperRechts != 0) {
    if ((intervalStepperRechts > 0) != richtingStepperRechts) {
      richtingStepperRechts = !richtingStepperRechts;
      digitalWrite(dirPinRechts, richtingStepperRechts);
    }
    if (millis() - previousMillisStepperRechts >= abs(intervalStepperRechts) && statusStepperRechts == false) {
      if (intervalStepperRechts < 0) {
        positieRechts--;
      } else {
        positieRechts++;
      }
      digitalWrite(stepPinRechts, HIGH);
      previousMillisStepperRechts = millis();
      statusStepperRechts = true;
    }
    if (millis() - intervalLaagzetten >= abs(intervalStepperRechts) && statusStepperRechts == true) {
      digitalWrite(stepPinRechts, LOW);
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

ISR(TIMER1_COMPA_vect) { //timer1 interrupt service routine
  Timer = true;
}
