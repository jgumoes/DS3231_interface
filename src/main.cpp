#include <Arduino.h>

#include "DS3231_interface.hpp"
#include "testTimesArray.hpp"

#ifdef DEVKIT
constexpr int SCL_pin = GPIO_NUM_26;
constexpr int SDA_pin = GPIO_NUM_25;
constexpr int RST = GPIO_NUM_33;      // ideally this would be the reset pin, but VCC should also work
#endif

#ifdef XIAO_ESP32S3
constexpr int SCL_pin = D9;
constexpr int SDA_pin = D8;
constexpr int RST = D10;      // ideally this would be the reset pin, but VCC should also work
#endif

void setup() {
  using namespace DS3231_Interface;
  Serial.begin(115200);
  delay(2000);
  Serial.println("Setting up...");
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.println("");

  if(!Wire.begin(SDA_pin, SCL_pin)){
    Serial.println("wire has not begun");
  }

  // reset chip to make tests consistent
  {
    Serial.print("timeout: ");
    Serial.println(Wire.getTimeOut());
    // ideally the RST would be the reset pin, but if there's no backup battery then toggling the power works
    digitalWrite(RST, 0);
    delay(20);
    digitalWrite(RST, 1);
    delay(2000);  // allow 2 seconds for the DS3231 to sort itself out
  }
  
  // setup
  InterfaceClass testClass(Wire);

  uint64_t actualTime;
  Serial.print("initialise: ");
  Serial.println(testClass.initialise(&actualTime)); // there should be a time fault


  // some manual testing
  {
    // testClass.setControlRegister();
  
    // control register should have been set
    byte controlRegister;
    Serial.print("readControlRegister: ");
    Serial.println(testClass.readControlRegister(&controlRegister));
    const byte expectedRegister = 0b00000100;
    Serial.print("expectedRegister: ");
    Serial.println(expectedRegister);
    Serial.print("controlRegister: ");
    Serial.println(controlRegister);
  
    // set a timestamp
    bool setTimeSuccess = testClass.setTime(testArray.at(0).localTimestamp);
    Serial.print("setTimeSuccess: ");
    Serial.println(setTimeSuccess);
  
    Serial.print("hasTimeFault: ");
    Serial.println(testClass.hasTimeFault());
    bool stopFlagFault = testClass.checkStopFlag();
    Serial.print("checkStopFlag: ");
    Serial.println(stopFlagFault);
  
    Serial.print("re-initialise");
    Serial.println(testClass.initialise(&actualTime)); // time fault should have been cleared
    Serial.print("expected timestamp: ");
    Serial.println(testArray.at(0).localTimestamp);
  
    testClass.getTimestamp(&actualTime);
    Serial.print("actual timestamp: ");
    Serial.println(actualTime);
  
    Serial.print("has time fault = "); Serial.println(testClass.hasTimeFault());
  }
}

void loop() {
  // put your main code here, to run repeatedly:
}
