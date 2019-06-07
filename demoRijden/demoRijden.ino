#define stepPinLinks A1         //De vier aansluitingen voor de stepper drivers (twee per motor)
#define dirPinLinks A0
#define stepPinRechts 13
#define dirPinRechts 12

boolean statusStepperLinks = false;
boolean statusStepperRechts = false;
boolean richtingStepperLinks = false;
boolean richtingStepperRechts = false;

long positieLinks = 0;
long positieRechts = 0;
#define CORNER_STEPS 340
int done = 0;

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

uint8_t state = 0;

void setup() {
  pinMode(stepPinLinks, OUTPUT);
  pinMode(dirPinLinks, OUTPUT);
  pinMode(stepPinRechts, OUTPUT);
  pinMode(dirPinRechts, OUTPUT);
  state = 1;
}

void loop() {
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
  stepperLinks();
  stepperRechts();
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
