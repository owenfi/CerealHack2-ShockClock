

// pseudo interface for all UserInterfaces
/*
class UserInterface {
  public: 
    virtual int tick() = 0;
};
*/

#define CURRENT_TEMP 0
#define SET_TEMP_MIN     12
#define SET_TEMP_MAX     13
#define VELOCITY     1
#define DURATION     2

#define SETTINGS                    3

#define SETTINGS_SET_TIME           4
#define SETTINGS_SET_TIME_HOUR      5
#define SETTINGS_SET_TIME_MINUTE    6

#define SETTINGS_SET_SCHED          7
#define SETTINGS_SET_SCHED_MODE     14
#define SETTINGS_SET_SCHED_HOUR     8
#define SETTINGS_SET_SCHED_MINUTE   9
#define SETTINGS_SET_SCHED_TEMP_MIN 10
#define SETTINGS_SET_SCHED_TEMP_MAX 11

#define INFO_SCREEN 15

#define UP    1
#define DOWN  2
#define LEFT  3
#define RIGHT 0

static int adc_key_val[5] ={
  30, 150, 360, 535, 760};

template<
  class ThermostatStrategy, 
  class ThermostatSchedule,
  class HvacDriver>
class ThermostatUserInterface /* : UserInterface */ {
  private:
    struct pt keyThread;
    struct pt printThread;
    
    int key;
    int oldkey;
    
    int configMode;
    
    int displayState;

    LiquidCrystal* lcd;
    ThermostatStrategy* thermostatStrategy;
    ThermostatSchedule* thermostatSchedule;
    HvacDriver* hvacDriver;
    float* temperature;
    float* outsideTemperature;
    float* forecastTemperature;
    char slaveId;
    
    // Convert ADC value to key number
    int get_key(unsigned int input) {
      int NUM_KEYS = 5;
      int k;
    
      for (k = 0; k < NUM_KEYS; k++) {
        if (input < adc_key_val[k]) {
    
          return k;
        }
      }
    
      if (k >= NUM_KEYS)
        k = -1;     // No valid key pressed
    
      return k;
    }
    
    void printDigits(int digits){
      // utility function for digital clock display: prints preceding colon and leading 0
      lcd->print(":");
      
      if(digits < 10) {
        lcd->print('0');
      }
      
      lcd->print(digits);
    }

    int getKeyThreadFunction(struct pt *pt) {
      PT_BEGIN(pt);
    
      while (1) {
        key = get_key(analogRead(0));
    
        if (key != oldkey) {
          PT_DELAY(pt, 10);		// wait for debounce time
          key = get_key(analogRead(0));    // read the value from the sensor  
          		        // convert into key press
          if (key != oldkey) {			
            oldkey = key;
    
    
            if (displayState == CURRENT_TEMP) {
              if (key == UP){
                displayState = SETTINGS_SET_SCHED;
              } else if (key == DOWN) {
                displayState = INFO_SCREEN;
              } else if (key == RIGHT) {
                displayState = SET_TEMP_MIN;
              }
              
            } else if (displayState == INFO_SCREEN) {
              if (key == UP){
                displayState = CURRENT_TEMP;
              } else if (key == DOWN) {
                displayState = SETTINGS_SET_TIME;
              }
              
            } else if (displayState == SET_TEMP_MIN) {
              if (key == UP){
                thermostatStrategy->setMinTemp(thermostatStrategy->getMinTemp() + 1);
              } else if (key == DOWN) {
                thermostatStrategy->setMinTemp(thermostatStrategy->getMinTemp() - 1);
              } else if (key == LEFT) {
                displayState = CURRENT_TEMP;
              } else if (key == RIGHT) {
                displayState = SET_TEMP_MAX;
              }
              
            } else if (displayState == SET_TEMP_MAX) {
              if (key == UP){
                thermostatStrategy->setMaxTemp(thermostatStrategy->getMaxTemp() + 1);
              } else if (key == DOWN) {
                thermostatStrategy->setMaxTemp(thermostatStrategy->getMaxTemp() - 1);
              } else if (key == LEFT) {
                displayState = SET_TEMP_MIN;
              } else if (key == RIGHT) {
                displayState = CURRENT_TEMP;
              }
    
    
            } else if (displayState == SETTINGS_SET_TIME) {
              if (key == UP){
                displayState = INFO_SCREEN;
              } else if (key == DOWN) {
                displayState = SETTINGS_SET_SCHED;
              } else if (key == RIGHT) {
                displayState = SETTINGS_SET_TIME_HOUR;
              }
              
            } else if (displayState == SETTINGS_SET_TIME_HOUR) {
              if (key == UP){
                setTime(now() + (60 * 60));
              } else if (key == DOWN) {
                setTime(now() - (60 * 60));
              } else if (key == LEFT) {
                displayState = SETTINGS_SET_TIME;
              } else if (key == RIGHT) {
                displayState = SETTINGS_SET_TIME_MINUTE;
              }
              
            } else if (displayState == SETTINGS_SET_TIME_MINUTE) {
              if (key == UP){
                setTime(now() + (60));
              } else if (key == DOWN) {
                setTime(now() - (60));
              } else if (key == LEFT) {
                displayState = SETTINGS_SET_TIME_HOUR;
              } else if (key == RIGHT) {
                displayState = SETTINGS_SET_TIME;
              }
    
            } else if (displayState == SETTINGS_SET_SCHED) {
              configMode = 0;
              
              if (key == UP){
                displayState = SETTINGS_SET_TIME;
              } else if (key == DOWN) {
                displayState = CURRENT_TEMP;
              } else if (key == RIGHT) {
                displayState = SETTINGS_SET_SCHED_MODE;
              }
              
            } else if (displayState == SETTINGS_SET_SCHED_MODE) {
              if (key == UP){
                configMode = (configMode + 3) % 4;
              } else if (key == DOWN) {
                configMode = (configMode + 1) % 4;
              } else if (key == LEFT) {
                displayState = SETTINGS_SET_SCHED;
              } else if (key == RIGHT) {
                displayState = SETTINGS_SET_SCHED_HOUR;
              }
              
            } else if (displayState == SETTINGS_SET_SCHED_HOUR) {
              if (key == UP){
                thermostatSchedule->setStartTime(
                  configMode,
                  (thermostatSchedule->getStartTime(configMode) + ((unsigned long)1 * 60 * 60)) % ((unsigned long)24 * 60 * 60)); 
                
              } else if (key == DOWN) {
                thermostatSchedule->setStartTime(
                  configMode,
                  (thermostatSchedule->getStartTime(configMode) + ((unsigned long)23 * 60 * 60)) % ((unsigned long)24 * 60 * 60)); 
    
              } else if (key == LEFT) {
                displayState = SETTINGS_SET_SCHED_MODE;
              } else if (key == RIGHT) {
                displayState = SETTINGS_SET_SCHED_MINUTE;
              }
              
            } else if (displayState == SETTINGS_SET_SCHED_MINUTE) {
              if (key == UP){
                thermostatSchedule->setStartTime(
                  configMode,
                  (thermostatSchedule->getStartTime(configMode) + ((unsigned long)1 * 60)) % ((unsigned long)60 * 60)); 
              } else if (key == DOWN) {
                thermostatSchedule->setStartTime(
                  configMode,
                  (thermostatSchedule->getStartTime(configMode) + ((unsigned long)59 * 60)) % ((unsigned long)60 * 60)); 
              } else if (key == LEFT) {
                displayState = SETTINGS_SET_SCHED_HOUR;
              } else if (key == RIGHT) {
                displayState = SETTINGS_SET_SCHED_TEMP_MIN;
              }
              
            } else if (displayState == SETTINGS_SET_SCHED_TEMP_MIN) {
              if (key == UP){
                thermostatSchedule->setMinTemp(
                  configMode, 
                  thermostatSchedule->getMinTemp(configMode) + 1);
                
              } else if (key == DOWN) {
                thermostatSchedule->setMinTemp(
                  configMode, 
                  thermostatSchedule->getMinTemp(configMode) - 1);
                  
              } else if (key == LEFT) {
                displayState = SETTINGS_SET_SCHED_MINUTE;
              } else if (key == RIGHT) {
                displayState = SETTINGS_SET_SCHED_TEMP_MAX;
              }
              
            } else if (displayState == SETTINGS_SET_SCHED_TEMP_MAX) {
              if (key == UP){
                thermostatSchedule->setMaxTemp(
                  configMode, 
                  thermostatSchedule->getMaxTemp(configMode) + 1);
                  
              } else if (key == DOWN) {
                thermostatSchedule->setMaxTemp(
                  configMode, 
                  thermostatSchedule->getMaxTemp(configMode) - 1);
                  
              } else if (key == LEFT) {
                displayState = SETTINGS_SET_SCHED_TEMP_MIN;
              } else if (key == RIGHT) {
                configMode = (configMode + 1) % 4;
                
                if (configMode == 0) {
                  displayState = SETTINGS_SET_SCHED;
                } else {
                  displayState = SETTINGS_SET_SCHED_MODE;
                }
              }
            }
            
            lcd->clear();
            updateDisplay();
          }
        }
        
        PT_DELAY(pt, 100);
      }
      PT_END(pt);
    }
    
    int printThreadFunction(struct pt *pt) {
      PT_BEGIN(pt);
    
      while (1) {
        updateDisplay();
        PT_DELAY(pt, 1000);
      }
    
      PT_END(pt);
    }
    
    void updateDisplay() {
      time_t currentTime = now();
      
      if (displayState == CURRENT_TEMP) {
        lcd->setCursor(0, 0);
        lcd->print(hourFormat12(currentTime));
        printDigits(minute(currentTime));
        lcd->print(" ");
        
        if (isAM(currentTime)) {
          lcd->print("AM ");
        } else {
          lcd->print("PM ");
        }
         

         
        lcd->setCursor(10, 0);
        if (hvacDriver->isHeating()) { 
          lcd->print("H");
        } else {
          lcd->print(" ");
        }
         
        if (hvacDriver->isCooling()) { 
          lcd->print("C");
        } else {
          lcd->print(" ");
        }
         
        if (hvacDriver->isFan()) { 
          lcd->print("F");
        } else {
          lcd->print(" ");
        }
        
        if (hvacDriver->isUpstairsZone()) { 
          lcd->print(" ");
        } else {
          lcd->print("U");
        }
         
        if (hvacDriver->isDownstairsZone()) { 
          lcd->print(" ");
        } else {
          lcd->print("D");
        }
         
        if (hvacDriver->isVent()) { 
          lcd->print("V");
        } else {
          lcd->print(" ");
        }
        
        
    
        lcd->setCursor(0, 1);
        lcd->print((int) *temperature);
        lcd->print((char) 223);
        lcd->print(" ");
        lcd->print(slaveId);
        lcd->print("  ");
        
        lcd->setCursor(10, 1);
        lcd->print(thermostatSchedule->getModeString());
        
      } else if (displayState == INFO_SCREEN) {
        lcd->setCursor(0, 0);
        lcd->print("Outside: "); 
        lcd->print((int) *outsideTemperature);
        lcd->print((char) 223);
        
        lcd->setCursor(0, 1);
        lcd->print("Forecast: ");
        lcd->print((int) *forecastTemperature);
        lcd->print((char) 223);
        
      } else if (displayState == SET_TEMP_MIN) {
        lcd->setCursor(0, 0);
        lcd->print("Min Temp");
    
        lcd->setCursor(0, 1);
        lcd->print(thermostatStrategy->getMinTemp());
        
      } else if (displayState == SET_TEMP_MAX) {
        lcd->setCursor(0, 0);
        lcd->print("Max Temp");
    
        lcd->setCursor(0, 1);
        lcd->print(thermostatStrategy->getMaxTemp());
          
    
      } else if (displayState == SETTINGS_SET_TIME) {
        lcd->setCursor(0, 0);
        lcd->print("Set Time");
    
      } else if (displayState == SETTINGS_SET_TIME_HOUR) {
        lcd->setCursor(0, 0);
        lcd->print("Set Hour");
    
        lcd->setCursor(0, 1);
        lcd->print(hour(currentTime));
        printDigits(minute(currentTime));
        
      } else if (displayState == SETTINGS_SET_TIME_MINUTE) {
        lcd->setCursor(0, 0);
        lcd->print("Set Minute");
    
        lcd->setCursor(0, 1);
        lcd->print(hour(currentTime));
        printDigits(minute(currentTime));
      
      
      } else if (displayState == SETTINGS_SET_SCHED) {
        lcd->setCursor(0, 0);
        lcd->print("Set Schedule");
        
      } else if (displayState == SETTINGS_SET_SCHED_MODE) {
        lcd->setCursor(0, 0);
        lcd->print(thermostatSchedule->getModeString(configMode));
        
      } else if (displayState == SETTINGS_SET_SCHED_HOUR) {
        lcd->setCursor(0, 0);
        lcd->print(thermostatSchedule->getModeString(configMode));
        lcd->print(" Hour");
        
        lcd->setCursor(0, 1);
        lcd->print((thermostatSchedule->getStartTime(configMode) % ((unsigned long) 24 * 60 * 60))/(60 * 60));
        
      } else if (displayState == SETTINGS_SET_SCHED_MINUTE) {
        lcd->setCursor(0, 0);
        lcd->print(thermostatSchedule->getModeString(configMode));
        lcd->print(" Minute");
        
        lcd->setCursor(0, 1);
        lcd->print((thermostatSchedule->getStartTime(configMode) % ((unsigned long) 60 * 60)) / 60);
        
      } else if (displayState == SETTINGS_SET_SCHED_TEMP_MIN) {
        lcd->setCursor(0, 0);
        lcd->print(thermostatSchedule->getModeString(configMode));
        lcd->print(" Min Temp");
        
        lcd->setCursor(0, 1);
        lcd->print(thermostatSchedule->getMinTemp(configMode));
        
      } else if (displayState == SETTINGS_SET_SCHED_TEMP_MAX) {
        lcd->setCursor(0, 0);
        lcd->print(thermostatSchedule->getModeString(configMode));
        lcd->print(" Max Temp");
        
        lcd->setCursor(0, 1);
        lcd->print(thermostatSchedule->getMaxTemp(configMode));
      }
    }
   
  public:
    ThermostatUserInterface(
      LiquidCrystal* _lcd,
      ThermostatStrategy* _thermostatStrategy,
      ThermostatSchedule* _thermostatSchedule,
      HvacDriver* _hvacDriver,
      float* _temperature,
      float* _outsideTemperature,
      float* _forecastTemperature,
      char _slaveId
    ) {
      lcd = _lcd;
      thermostatStrategy = _thermostatStrategy;
      thermostatSchedule = _thermostatSchedule;
      hvacDriver = _hvacDriver;
      temperature = _temperature;
      outsideTemperature = _outsideTemperature;
      forecastTemperature = _forecastTemperature;
      slaveId = _slaveId;
      
      configMode = 0;
      displayState = CURRENT_TEMP;
      
      PT_INIT(&keyThread);
      PT_INIT(&printThread); 
    }

    int tick() {
      getKeyThreadFunction(&keyThread);
      printThreadFunction(&printThread);
    }
};

