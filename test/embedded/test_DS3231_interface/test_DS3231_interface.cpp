/*

to run the tests on an embedded device, use the command prompt:
  pio test -e esp32_wroom_32 -f "*test_DS3231*" -vv

the test panel doesn't tell you when to push the button
*/
#include <unity.h>
#include <Wire.h>

#include "DS3231_interface.hpp"
#include "../../testTimesArray.hpp"

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

void setUp(void) {
  // set stuff up here
  using namespace DS3231_Interface;

  TEST_ASSERT_TRUE_MESSAGE(Wire.begin(SDA_pin, SCL_pin, 400000), "wire has not begun");
  digitalWrite(RST, 0);
  delay(20);
  digitalWrite(RST, 1);
  delay(2000);  // allow 2 seconds for the DS3231 to sort itself out
}

void tearDown(void) {
  // clean stuff up here
}

void test_initialise(){
  using namespace DS3231_Interface;
  using namespace BluetoothStructs;

  InterfaceClass testClass(Wire);
  uint64_t actualTime;
  TEST_ASSERT_TRUE(testClass.initialise(&actualTime)); // there should be a time fault

  // control register should have been set
  byte controlRegister;
  TEST_ASSERT_TRUE(testClass.readControlRegister(&controlRegister));
  const byte expectedRegister = 0b00000100;
  TEST_ASSERT_EQUAL(expectedRegister, controlRegister);

  // set a timestamp
  TEST_ASSERT_TRUE(testClass.setTime(testArray.at(0).localTimestamp));
  TEST_ASSERT_FALSE(testClass.hasTimeFault());
  TEST_ASSERT_FALSE(testClass.checkStopFlag());

  TEST_ASSERT_FALSE(testClass.initialise(&actualTime)); // time fault should have been cleared
  TEST_ASSERT_UINT64_WITHIN(1, testArray.at(0).localTimestamp, actualTime);
}

void test_checkStopFlag(){
  using namespace DS3231_Interface;
  using namespace BluetoothStructs;

  InterfaceClass testClass(Wire);
  TEST_ASSERT_TRUE(testClass.checkStopFlag());  // should have a time fault on construction
  TEST_ASSERT_TRUE(testClass.setTime(testArray.at(0).localTimestamp));
  TEST_ASSERT_FALSE(testClass.hasTimeFault());
  TEST_ASSERT_FALSE(testClass.checkStopFlag());
}

void test_setTimestampGetTimeStruct(){
  // set a timestamp, and read it back as a time struct
  using namespace DS3231_Interface;
  using namespace BluetoothStructs;

  InterfaceClass testClass(Wire);
  uint64_t timestamp;
  testClass.initialise(&timestamp);
  for(TestTimeParamsStruct timeParams : testArray){

    // set the time
    TEST_ASSERT_TRUE_MESSAGE(testClass.setTime(timeParams.localTimestamp), timeParams.testName.c_str());
    TEST_ASSERT_FALSE_MESSAGE(testClass.hasTimeFault(), timeParams.testName.c_str());

    // read back the time
    DayDateTimeStruct actualStruct;
    TEST_ASSERT_TRUE_MESSAGE(testClass.getDateTime(&actualStruct), timeParams.testName.c_str());

    // if seconds roll over, testing the structs directly becomes very complicated. so, convert struct into timestamp
    uint64_t actualTime;
    convertToLocalTimestamp(actualStruct, &actualTime);
    TEST_ASSERT_UINT64_WITHIN_MESSAGE(1, timeParams.localTimestamp, actualTime, timeParams.testName.c_str());

    // there shouldn't be a fault
    TEST_ASSERT_FALSE_MESSAGE(testClass.checkStopFlag(), timeParams.testName.c_str());
  }
}

void test_setTimeStructGetTimestamp(){
  // set a time struct, and read it back as a timestamp
  using namespace DS3231_Interface;
  using namespace BluetoothStructs;

  InterfaceClass testClass(Wire);
  uint64_t timestamp;
  testClass.initialise(&timestamp);
  for(TestTimeParamsStruct timeParams : testArray){
    DayDateTimeStruct expectedStruct = makeDayDateTime(timeParams);

    // set the time
    TEST_ASSERT_TRUE_MESSAGE(testClass.setTime(expectedStruct), timeParams.testName.c_str());
    TEST_ASSERT_FALSE_MESSAGE(testClass.hasTimeFault(), timeParams.testName.c_str());

    // read back the time
    uint64_t actualTime;
    TEST_ASSERT_TRUE_MESSAGE(testClass.getTimestamp(&actualTime), timeParams.testName.c_str());

    TEST_ASSERT_UINT64_WITHIN_MESSAGE(1, timeParams.localTimestamp, actualTime, timeParams.testName.c_str());

    // there shouldn't be a fault
    TEST_ASSERT_FALSE_MESSAGE(testClass.checkStopFlag(), timeParams.testName.c_str());
  }
}

void RUN_UNITY_TESTS(){
  UNITY_BEGIN();
  RUN_TEST(test_initialise);
  RUN_TEST(test_checkStopFlag);
  RUN_TEST(test_setTimestampGetTimeStruct);
  RUN_TEST(test_setTimeStructGetTimestamp);

  UNITY_END();
};

void setup() {
  // Wait ~2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  delay(2000);
  // Wire.begin(SDA_pin, SCL_pin, 400000);

  RUN_UNITY_TESTS();
}
void loop() {}