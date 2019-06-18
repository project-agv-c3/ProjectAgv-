//Libraries toevoegen
#include <Wire.h>
#include <VL53L0X.h>
#include <NewPing.h>
#include <SoftwareSerial.h>

//De maximale afstanden
#define MAX_SONAR 254           //Voor de ultrasone sensoren is dit een afstand in centimeters
#define MAX_TOF 500             //Voor de ToFs is dit een timeout

//De pin aansluitingen
#define TOF_PIN1 6              //De aansluitingen voor de voedingen van de ToFs
#define TOF_PIN2 7
#define TOF_PIN3 8
#define TOF_PIN4 9
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
#define LED_PIN 11

#define MAX_PAD_WIDTH 160
#define DOORRIJ_LENGTE 620
#define BOCHT_LENGTE 330

#define LINKS true
#define RECHTS false

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

//Verschillende states
#define IDLING 0
#define PAD_VOLGEN 1
#define DOORRIJDEN 2
#define BOCHT_MAKEN 3

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
uint16_t distanceToF1 = 0;
uint16_t distanceToF2 = 0;
uint16_t distanceToF3 = 0;
uint16_t distanceToF4 = 0;
uint16_t oldDistanceToF1 = 0;
uint16_t oldDistanceToF2 = 0;
uint16_t oldDistanceToF3 = 0;
uint16_t oldDistanceToF4 = 0;
uint8_t distancesonar1 = 0;
uint8_t distancesonar2 = 0;
uint8_t distancesonar3 = 0;
uint8_t distancesonar4 = 0;
uint8_t gemiddeldeWaarde = 0;

//globale integers voor vaste afstanden
uint8_t afstandPad = 50;
uint8_t afstandSonarVoor = 7;

//Globale integers voor statussen
uint8_t sonarNummer = 0;
uint8_t tofNummer = 0;
boolean statusStepperLinks = false;
boolean statusStepperRechts = false;
boolean richtingStepperLinks = false;
boolean richtingStepperRechts = false;

long positieLinks = 0;
long positieRechts = 0;
boolean bochtRichting = LINKS;
uint8_t done = 0;

//variabele millis
unsigned long previousMillisToF = 0;
#define intervalToF 20

unsigned long previousMillisSonar = 6;
#define intervalsonar 20

unsigned long previousMillisStepperLinks = 0;
unsigned long previousMillisStepperRechts = 0;
uint8_t tellerStepperLinks = 0;
uint8_t tellerStepperRechts = 0;
int8_t intervalStepperLinks = 0;
int8_t intervalStepperRechts = 0;
#define intervalLaagzetten 1

uint8_t state = IDLING;
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
  state = PAD_VOLGEN;
  bluetooth.write(0x57);
  ToFs_init();          //Initializeer de time of flight sensoren
  TCCR2A = 0b00000000;  //
  TCCR2B = 0b00000100;  //Prescaler van 64 bits
  TIMSK2 = 0b00000001;  //Zet de overflow interrupt aan
  sei();
}

void loop() {
  if (btState == CONNECTED) {

  } else {
    if (bluetooth.available()) {
      if (bluetooth.read() == 0x58) {
        btState = CONNECTED;
      }
    }
  }

  if (!emergency) {
    //updaten sensoren
    sonar();
    ToF();

    switch (state) {
      case PAD_VOLGEN:
        if ((distanceToF3 + distanceToF4) > MAX_PAD_WIDTH) {
          positieLinks = 0;
          positieRechts = 0;
          interval(4, 4);
          bochtRichting = LINKS;
          state = DOORRIJDEN;
        } else {
          if (distancesonar1 > afstandSonarVoor) {
            uint16_t offset = abs(distanceToF3 - gemiddeldeWaarde);
            if (offset > 10) {
              if (offset > 20) {
                if (distanceToF3 < gemiddeldeWaarde) {
                  interval(5, 3);
                  bluetooth.write(-2);
                } else {
                  interval(3, 5);
                  bluetooth.write(2);
                }
              } else {
                if (distanceToF3 < gemiddeldeWaarde) {
                  interval(5, 4);
                  bluetooth.write(-1);
                } else {
                  interval(4, 5);
                  bluetooth.write(1);
                }
              }
            } else {
              interval(4, 4);
              bluetooth.write((byte)0);
            }
          } else {
            interval(0, 0);
            if ((millis() % 400) < 200) {
              digitalWrite(LED_PIN, HIGH);
            } else {
              digitalWrite(LED_PIN, LOW);
            }
          }
        }
        break;
      case DOORRIJDEN:
        if (positieLinks >= DOORRIJ_LENGTE && positieRechts >= DOORRIJ_LENGTE) {
          if (bochtRichting == LINKS) {
            interval(-6, 6);
          } else {
            interval(6, -6);
          }
          positieRechts = 0;
          positieLinks = 0;
          state = BOCHT_MAKEN;
          done = 0;
        }
        break;
      case BOCHT_MAKEN:
        if (positieRechts >= BOCHT_LENGTE && positieLinks >= BOCHT_LENGTE) {///////////////////////////////////////////////////////////////
          interval(0, 0);
          state = 99;
        }
        break;

      default:
        //Niks doen
        break;
    }
    if (analogRead(VOLTAGE_PIN) <= 10) {
      interval(0, 0);
      emergency = true;
      digitalWrite(LED_PIN, HIGH);
    }
  } else {
    if (analogRead(VOLTAGE_PIN) > 10) {
      emergency = false;
      digitalWrite(LED_PIN, LOW);
    }
  }
}

void interval(int8_t _left, int8_t _right) {
  intervalStepperLinks = _left;
  intervalStepperRechts = _right;
}

void sonar() {
  //updaten Sonars
  if (millis() - previousMillisSonar >= intervalsonar) {
    previousMillisSonar = millis();
    uint8_t tempDist;
    switch (sonarNummer) {
      case 0:
        tempDist = sonar1.ping_cm();
        if (tempDist != 0) {
          distancesonar1 = tempDist;
        } else {
          distancesonar1 = 255;
        }
        break;
      case 1:
        tempDist = sonar2.ping_cm();
        if (tempDist != 0) {
          distancesonar2 = tempDist;
        } else {
          distancesonar2 = 255;
        }
        break;
      case 2:
        tempDist = sonar3.ping_cm();
        if (tempDist != 0) {
          distancesonar3 = tempDist;
        } else {
          distancesonar3 = 255;
        }
        break;
      case 3:
        tempDist = sonar4.ping_cm();
        if (tempDist != 0) {
          distancesonar4 = tempDist;
        } else {
          distancesonar4 = 255;
        }
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
    uint16_t tempDist;
    switch (tofNummer) {
      case 0:
        tempDist = ToF1.readRangeSingleMillimeters();
        distanceToF1 = (tempDist + oldDistanceToF1) / 2;
        oldDistanceToF1 = tempDist;
        break;
      case 1:
        tempDist = ToF2.readRangeSingleMillimeters();
        distanceToF2 = (tempDist + oldDistanceToF2) / 2;
        oldDistanceToF2 = tempDist;
        break;
      case 2:
        tempDist = ToF3.readRangeSingleMillimeters();
        distanceToF3 = (tempDist + oldDistanceToF3) / 2;
        oldDistanceToF3 = tempDist;
        break;
      case 3:
        tempDist = abs(ToF4.readRangeSingleMillimeters() - 26);
        distanceToF4 = (tempDist + oldDistanceToF4) / 2;
        oldDistanceToF4 = tempDist;
        break;
    }
    if (tofNummer >= 3) {
      tofNummer = 0;
    } else {
      tofNummer++;
    }
    gemiddeldeWaarde = (distanceToF3 + distanceToF4) / 2;
    Serial.print(distanceToF3);
    Serial.print("\t");
    Serial.println(distanceToF4);
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
  ToF3.init();
  ToF3.setAddress(TOF3);
  ToF3.setTimeout(MAX_TOF);

  //Geef ToF4 een nieuw adress
  digitalWrite(TOF_PIN4, HIGH);
  delay(10);
  ToF4.init();
  ToF4.setAddress(TOF4);
  ToF4.setTimeout(MAX_TOF);
}

ISR(TIMER2_OVF_vect) {
  if (intervalStepperLinks != 0) {
    tellerStepperLinks++;
    if (intervalStepperLinks < 0) {
      digitalWrite(dirPinLinks, LOW);
    } else {
      digitalWrite(dirPinLinks, HIGH);
    }
    if (tellerStepperLinks == 1) {
      digitalWrite(stepPinLinks, LOW);
    }
    if (tellerStepperLinks > abs(intervalStepperLinks)) {
      tellerStepperLinks = 0;
      digitalWrite(stepPinLinks, HIGH);
      positieLinks++;
    }
  }
  if (intervalStepperRechts != 0) {
    tellerStepperRechts++;
    if (intervalStepperRechts < 0) {
      digitalWrite(dirPinRechts, LOW);
    } else {
      digitalWrite(dirPinRechts, HIGH);
    }
    if (tellerStepperRechts == 1) {
      digitalWrite(stepPinRechts, LOW);
    }
    if (tellerStepperRechts > abs(intervalStepperRechts)) {
      tellerStepperRechts = 0;
      digitalWrite(stepPinRechts, HIGH);
      positieRechts++;
    }
  }
}
