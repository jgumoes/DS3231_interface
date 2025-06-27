#include <unity.h>

#include "DS3231_interface.hpp"
#include "../../testTimesArray.hpp"

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

void test_convertToLocalTimestamp(){
  using namespace DS3231_Interface;
  using namespace BluetoothStructs;

  for(TestTimeParamsStruct testIT : testArray){
    DayDateTimeStruct testStruct = makeDayDateTime(testIT);
    
    uint64_t actual = 0;
    convertToLocalTimestamp(testStruct, &actual);
    uint64_t expected = testIT.localTimestamp;
    TEST_ASSERT_EQUAL_MESSAGE(expected, actual, testIT.testName.c_str());
  }
}

void test_convertToBLETimeStruct(){
  using namespace DS3231_Interface;
  using namespace BluetoothStructs;

  for(TestTimeParamsStruct testIT : testArray){
    DayDateTimeStruct actual;
    convertToBLETimeStruct(testIT.localTimestamp, actual);
    DayDateTimeStruct expected = makeDayDateTime(testIT);

    TEST_ASSERT_EQUAL_MESSAGE(expected.dateTime.year, actual.dateTime.year, testIT.testName.c_str());
    TEST_ASSERT_EQUAL_MESSAGE(expected.dateTime.month, actual.dateTime.month, testIT.testName.c_str());
    TEST_ASSERT_EQUAL_MESSAGE(expected.dateTime.day, actual.dateTime.day, testIT.testName.c_str());
    TEST_ASSERT_EQUAL_MESSAGE(expected.dateTime.hours, actual.dateTime.hours, testIT.testName.c_str());
    TEST_ASSERT_EQUAL_MESSAGE(expected.dateTime.minutes, actual.dateTime.minutes, testIT.testName.c_str());
    TEST_ASSERT_EQUAL_MESSAGE(expected.dateTime.seconds, actual.dateTime.seconds, testIT.testName.c_str());
    TEST_ASSERT_EQUAL_MESSAGE(expected.dayOfWeek, actual.dayOfWeek, testIT.testName.c_str());
  }
}

void RUN_UNITY_TESTS(){
  UNITY_BEGIN();
  RUN_TEST(test_convertToLocalTimestamp);
  RUN_TEST(test_convertToBLETimeStruct);
  UNITY_END();
};

#ifdef native_env
void WinMain(){
  RUN_UNITY_TESTS();
}
#else
void setup() {
  // Wait ~2 seconds before the Unity test runner
  // establishes connection with a board Serial interface
  delay(2000);

  RUN_UNITY_TESTS();
}
void loop() {}
#endif