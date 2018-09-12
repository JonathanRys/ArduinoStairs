#include "PriorityQueue.h"
#include "QueueList_Modified.h"

// Toggle debug
const bool DEBUG = false;

// Maximum value for unsigned long
const unsigned long ULONG_MAX = 4294967295;
// Number of steps per flight
const unsigned int NUMBER_OF_STEPS = 8;

// Number of flights going up + number going down
const unsigned int NUM_FLIGHTS = 8;

// Ease-of-use constants
const bool OFF = 0;
const bool ON = 1;

// A data structure representing a flight of stairs
struct Flight {
  unsigned int sensorPin;
  unsigned int ledRange[NUMBER_OF_STEPS];
};

/**********************************************
 **************** Stair class *****************
 **********************************************
 *                                            *
 * Description: Reprsents a single stair      *
 * with an ouput pin to an LED and a priority *
 *                                            *
 *********************************************/
 
class Stair {
  private:
    unsigned int uiLedPin;
    unsigned int uiPriority = 0;
  public:
    Stair(){};
    Stair(const Stair &stair) {
       uiLedPin = stair.uiLedPin;
       uiPriority = stair.uiPriority;
    }
    Stair(unsigned int pin) {
      uiLedPin = pin;
      pinMode(uiLedPin, OUTPUT);
    }

    inline bool operator < (Stair *b) const {
      return uiPriority < b->getPriority();
    }
    unsigned int getPriority() {
      return uiPriority;
    }
    void setPriority(unsigned int priority) {
      uiPriority = priority;
    }
    void setLedState(bool state) {
      digitalWrite(uiLedPin, state);
    }
    ~Stair() {}
};

// Overload the less-than operator for Stair types
bool operator < (Stair a, Stair b) {
  return a.getPriority() < b.getPriority();
}

/**********************************************
 ************* StairFlight class **************
 **********************************************
 *                                            *
 * Description: Reprsents a single flight of  *
 * stairs with a sensor and a timer to        *
 * control  *
 *                                            *
 *********************************************/
class StairFlight {
  private:
    unsigned int uiSensor;
    unsigned long ulTimeOn;
    QueueList <Stair> queue;

    Stair **stairs;
    
    // Why does making these static screw everything up?
    const unsigned long INTERVAL = 200;
    const unsigned long TIME_TO_STAY_ON_FOR = 10000;
    
    bool bLightsOn;
  public:
    StairFlight(Flight stairFlight) {
      uiSensor = stairFlight.sensorPin;
      ulTimeOn = 0;

      stairs = new Stair*[NUMBER_OF_STEPS];
      pinMode(uiSensor, INPUT);

      for (unsigned int i = 0; i < NUMBER_OF_STEPS; i++) {
        stairs[i] = new Stair(stairFlight.ledRange[i]);
      }
    }

    void turnLightsOn() {
      unsigned long ulNow = millis();
      if (DEBUG) {
        Serial.print("Lights(");
        Serial.print(uiSensor);
        Serial.print(") on from ");
        Serial.print(ulNow);
        Serial.print(" until: ");
        Serial.print(ulNow + TIME_TO_STAY_ON_FOR);
        Serial.print("\n");
      }
      if (bLightsOn) return;
      
      ulTimeOn = ulNow;
      bLightsOn = true;

      //Clear the queue
      for (unsigned int i = 0; i < NUMBER_OF_STEPS; i++) {
        stairs[i]->setPriority(ulNow + (INTERVAL * i));
        if (DEBUG) {
          Serial.print("\nSetting priority: ");
          Serial.print(ulNow + (INTERVAL * i));
        }
        queue.push(*stairs[i]);
      }
    }

    void turnLightsOff() {
      for (unsigned int i = 0; i < NUMBER_OF_STEPS; i++) {
        stairs[i]->setLedState(OFF);
        stairs[i]->setPriority(0);
      }
      
      ulTimeOn = 0;
      bLightsOn = false;
    }

    // This is the main program loop
    void checkSensor() {
      unsigned long ulNow = millis();
      
      if (queue.count() > 0) {
        Stair stair = queue.peek();
        
        if (ulNow >= stair.getPriority()) {
          if (DEBUG) {
            Serial.print("\n*LED activated by sensor ");
            Serial.print(uiSensor);
            Serial.print("\nNow: ");
            Serial.print(ulNow);
            Serial.print(", Priority: ");
            Serial.print(stair.getPriority());
            Serial.print("\n");
          }
          stair.setLedState(ON);
          queue.pop();
          // return here so the queue doesn't accept more events until it's cleared out.
          return;
        }
      }
      // Flip this logic if the default state of the sensor on different
      if (!bLightsOn && digitalRead(uiSensor)) {
        if (DEBUG) {
          Serial.print("\n");
          Serial.print("Sensor: ");
          Serial.print(uiSensor);
          Serial.print("\n");
    
          Serial.print("ulNow: ");
          Serial.print(ulNow);
          Serial.print("\n");
    
          Serial.print("ulTimeOn: ");
          Serial.print(ulTimeOn);
          Serial.print("\n");

          Serial.print("delta: ");
          Serial.print(ulNow - ulTimeOn);
          Serial.print("\n");
        }
        turnLightsOn();
        // return here so "ulNow" won't ever be before "ulTimeOn"
        return;
      }
           
      if (bLightsOn &&(unsigned long)(ulNow - ulTimeOn) >= TIME_TO_STAY_ON_FOR) {
        if (DEBUG) {
          Serial.print("\n*** LIGHTS OUT - SENSOR ");
          Serial.print(uiSensor);
          Serial.print(" ***\n");
          Serial.print(ulNow);
          Serial.print(" - ");
          Serial.print(ulTimeOn);
          Serial.print("\nulNow - time on: ");
        
          Serial.print((unsigned long)(ulNow - ulTimeOn));
          Serial.print(" >= ");
          Serial.print(TIME_TO_STAY_ON_FOR);
          Serial.print(" = ");
          Serial.print((unsigned long)(ulNow - ulTimeOn) >= TIME_TO_STAY_ON_FOR);
          Serial.print("\n\n");
        }
        turnLightsOff();
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
    Flight flights[NUM_FLIGHTS] = { 
      Flight({7, {22, 24, 26, 28, 30, 32, 34, 36}}), // Going down
      Flight({6, {36, 34, 32, 30, 28, 26, 24, 22}}), // Going up
      Flight({5, {23, 25, 27, 29, 31, 33, 35, 37}}), // Going down
      Flight({4, {37, 35, 33, 31, 29, 27, 25, 23}}), // Going up
      Flight({14, {38, 40, 42, 44, 46, 48, 50, 52}}), // Going down
      Flight({15, {52, 50, 48, 46, 44, 42, 40, 38}}), // Going up
      Flight({16, {39, 41, 43, 45, 47, 49, 51, 53}}), // Going down
      Flight({17, {53, 51, 49, 47, 45, 43, 41, 39}}) // Going up
    };

    StairFlight *stairFlights[NUM_FLIGHTS];
  public:
    Engine() {
      for (int i = 0 ; i < NUM_FLIGHTS ; i++) {
        stairFlights[i] = new StairFlight(flights[i]);
      }
    }
    
    void checkSensors() {
      for (int i = 0 ; i < NUM_FLIGHTS ; i++) {
        stairFlights[i]->checkSensor();
      }
    }

    ~Engine() {
      for (int i = 0 ; i < NUM_FLIGHTS ; i++) {
        delete stairFlights[i];
      }
    }
};

// ### Main program ###
void setup() {
  // Initialize the serial monitor:
  if (DEBUG) {
      Serial.begin(9600);
  }
}

// Create an instance of the engine to use in the event loop
Engine *engine = new Engine();

void loop() {
  engine->checkSensors();
}
