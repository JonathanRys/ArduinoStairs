#include "PriorityQueue.h"
#include "QueueList_Modified.h"

const unsigned long ULONG_MAX = 4294967295;
const unsigned int NUMBER_OF_STEPS = 8;
const bool OFF = 0;
const bool ON = 1;

struct Flight {
  unsigned int sensorPin;  
  unsigned int ledRange[NUMBER_OF_STEPS];
};

class Stair {
private:
  unsigned int iLedPin;
  unsigned int uiPriority;
public:
  Stair(){}
  Stair(unsigned int pin) {
    iLedPin = pin;
    pinMode(iLedPin, OUTPUT);
  }
  
  inline bool operator < (const Stair *b) {
    return uiPriority < b->getPriority();
  }
  unsigned int getPriority() {
    return uiPriority;
  }
  void setPriority(unsigned int priority) {
    uiPriority = priority;
  }
  void setLedState(bool state) {
    digitalWrite(iLedPin, state);
  }
  ~Stair() {}
};

bool operator < (const Stair a, const Stair b) {
  return a.getPriority() < b.getPriority();
}


class StairFlight {
private:
  unsigned int uiSensor;
  QueueList <Stair> queue;
  
  Stair **stairs;
  
  const unsigned long INTERVAL = 200;
  const unsigned long TIME_TO_STAY_ON_FOR = 10000;
  
  unsigned long ulTimeOff;
  bool bLightsOn;
public:
  StairFlight(Flight stairFlight) {
    uiSensor = stairFlight.sensorPin;
    
    stairs = new Stair*[NUMBER_OF_STEPS];
    pinMode(uiSensor, INPUT);

    for (unsigned int i = 0; i < NUMBER_OF_STEPS; i++) {
      stairs[i] = new Stair(stairFlight.ledRange[i]);
    }
  }
  
  void turnLightsOn() {
    unsigned long now = millis();
    ulTimeOff = now + TIME_TO_STAY_ON_FOR;
    if (bLightsOn) return;

    Serial.print("Turning lights on at ");
    Serial.print(now);
    Serial.print(" until ");
    Serial.print(ulTimeOff);
    Serial.print("\n");

    bLightsOn = true;

    for (unsigned int i = 0; i < NUMBER_OF_STEPS; i++) {
      stairs[i]->setPriority(now + (INTERVAL * i));
      queue.push(*stairs[i]);
    }
  }

  void turnLightsOff() {
    for (unsigned int i = 0; i < NUMBER_OF_STEPS; i++) {
      stairs[i]->setLedState(OFF);
    }

    bLightsOn = false;
  }
  
  void checkSensor() {
    unsigned long now = millis();

    if (queue.count() > 0) {
      Stair pStair = queue.peek();
      if (now >= pStair.getPriority()) {
        pStair.setLedState(ON);
        queue.pop();
      }
    } else {
      if (digitalRead(uiSensor)) {
        Serial.print("Sensor ");
        Serial.print(uiSensor);
        Serial.print(" read.\n");

        Serial.print("Time off: ");
        Serial.print(ulTimeOff);
        Serial.print("\nnow: ");
        Serial.print(now);
        Serial.print("\n*");
        Serial.print((ulTimeOff - now) <= 0);
        Serial.print("\n");
        
        turnLightsOn();
        return;
      }
      else if ((ulTimeOff - now) <= 0) {
        turnLightsOff();
        ulTimeOff = 0;
      }
    }
  }

  ~StairFlight() {
    for (unsigned int i = 0; i < NUMBER_OF_STEPS; i++) {
      delete stairs[i];
    }
    delete stairs;
  }
};

class Engine {
private:  
  // Flight: {sensorPin, [ledRange]}

  // Define variables to represent the top flight of stairs
  Flight flightKitchenTop = {14, {22, 24, 26, 28, 30, 32, 34, 36}};
  StairFlight *stairFlightKitchenTop = new StairFlight(flightKitchenTop);
  
  Flight flightKitchenBottom = {15, {36, 34, 32, 30, 28, 26, 24, 22}};
  StairFlight *stairFlightKitchenBottom = new StairFlight(flightKitchenBottom);

  // Define variables to represent the second flight of stairs
  Flight flightKitchenBedroomTop = {16, {23, 25, 27, 29, 31, 33, 35, 37}};
  StairFlight *stairFlightKitchenBedroomTop = new StairFlight(flightKitchenBedroomTop);
  
  Flight flightKitchenBedroomBottom = {17, {37, 35, 33, 31, 29, 27, 25, 23}};
  StairFlight *stairFlightKitchenBedroomBottom = new StairFlight(flightKitchenBedroomBottom);

  // Define variables to represent the third set of stairs
  Flight flightBedroomBasementTop = {18, {38, 40, 42, 44, 46, 48, 50, 52}};
  StairFlight *stairFlightBedroomBasementTop = new StairFlight(flightBedroomBasementTop);
  
  Flight flightBedroomBasementBottom = {19, {52, 50, 48, 46, 44, 42, 40, 38}};
  StairFlight *stairFlightBedroomBasementBottom = new StairFlight(flightBedroomBasementBottom);

  // Define variables to represent the bottom set of stairs
  Flight flightBasementTop = {20, {39, 41, 43, 45, 47, 49, 51, 53}};
  StairFlight *stairFlightBasementTop = new StairFlight(flightBasementTop);
  
  Flight flightBasementBottom = {21, {53, 51, 49, 47, 45, 43, 41, 39}};
  StairFlight *stairFlightBasementBottom = new StairFlight(flightBasementBottom); 
public:
  Engine() {}
  void checkSensors() {
      stairFlightKitchenTop->checkSensor();
      stairFlightKitchenBottom->checkSensor();
      
      stairFlightKitchenBedroomTop->checkSensor();
      stairFlightKitchenBedroomBottom->checkSensor();
      
      stairFlightBedroomBasementTop->checkSensor();
      stairFlightBedroomBasementBottom->checkSensor();
      
      stairFlightBasementTop->checkSensor();
      stairFlightBasementBottom->checkSensor();
  }
  ~Engine() {
    delete stairFlightKitchenTop;
    delete stairFlightKitchenBottom;
    delete stairFlightKitchenBedroomTop;
    delete stairFlightKitchenBedroomBottom;
    delete stairFlightBedroomBasementTop;
    delete stairFlightBedroomBasementBottom;
    delete stairFlightBasementTop;
    delete stairFlightBasementBottom;
  }
};

void setup() {
  // Initialize the serial montor:
  Serial.begin(9600);
}

// Create an instance of the
Engine *engine = new Engine();

void loop() {
  engine->checkSensors();  
}
