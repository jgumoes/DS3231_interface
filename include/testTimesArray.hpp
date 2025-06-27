/*
warning: javascript only uses UTC timestamps
javascript helper function to convert time to 2000 epoch:

let convert1970to2000 = (dateObj) => {
  let timestamp = dateObj.getTime();
  let epoch = new Date(0);
  epoch.setYear(2000);
  epoch.setUTCHours(0);
  return (timestamp - epoch.getTime())/1000;
}

javascript function for finding timestamps:

let convert2000To1970 = (localTimestamp, offset) => {
  let utcTimestamp = localTimestamp - offset;
  let epoch = new Date(0);
  epoch.setYear(2000);
  epoch.setUTCHours(0);
  let newTime = new Date(epoch.getTime() + utcTimestamp * 1000);
  console.log("1970 timestamp = ", newTime.getTime() + offset);
  newTime.setSeconds(newTime.getSeconds() + offset);
  console.log(newTime.toUTCString());
}
*/
#pragma once

#include <Arduino.h>
#include <etl/vector.h>

#include "BLE_structs.hpp"

struct TestTimeParamsStruct
{
  std::string testName = "";
  uint64_t localTimestamp = 0;
  uint8_t seconds = 0;
  uint8_t minutes = 0;
  uint8_t hours = 0;
  uint8_t dayOfWeek = 0;
  uint8_t date = 0;
  uint8_t month = 0;
  uint8_t years = 0;
  int32_t timezone = 0;
  uint16_t DST = 0;

  TestTimeParamsStruct(
    std::string set_testName,
    uint64_t set_localTimestamp,
    uint8_t set_seconds,
    uint8_t set_minutes,
    uint8_t set_hours,
    uint8_t set_dayOfWeek,
    uint8_t set_date,
    uint8_t set_month,
    uint8_t set_years,
    int32_t set_timezone,
    uint16_t set_DST
  ){
    testName = set_testName;
    localTimestamp = set_localTimestamp;
    seconds = set_seconds;
    minutes = set_minutes;
    hours = set_hours;
    dayOfWeek = set_dayOfWeek;
    date = set_date;
    month = set_month;
    years = set_years;
    timezone = set_timezone;
    DST = set_DST;
  }
};

constexpr uint16_t year2000 = 2000;

BluetoothStructs::DayDateTimeStruct makeDayDateTime(const TestTimeParamsStruct testingStruct){
  using namespace BluetoothStructs;
  DayDateTimeStruct dayDateTime{
    .dateTime = DateTimeStruct{
      .year = static_cast<uint16_t>(year2000 + testingStruct.years),
      .month = static_cast<Month>(testingStruct.month),
      .day = testingStruct.date,
      .hours = testingStruct.hours,
      .minutes = testingStruct.minutes,
      .seconds = testingStruct.seconds
    },
    .dayOfWeek = static_cast<DayOfWeek>(testingStruct.dayOfWeek)
  };
  return dayDateTime;
}

etl::vector<TestTimeParamsStruct, UINT8_MAX> const testArray = {
  TestTimeParamsStruct(std::string("during_british_winter_time_2023"), 729082742, 2, 59, 10, 2, 7, 2, 23, 0, 0),
  TestTimeParamsStruct(std::string("during_british_summer_time_2023"), 743102001, 21, 13, 17, 3, 19, 7, 23, 0, 60*60),
  TestTimeParamsStruct(std::string("feb_28_2023"), 730925880, 0, 58, 18, 2, 28, 2, 23, 0, 0),
  TestTimeParamsStruct(std::string("mar_1_2023"), 731007913, 13, 45, 17, 3, 1, 3, 23, 0, 0),
  TestTimeParamsStruct(std::string("feb_28_2024"), 762461880, 0, 58, 18, 3, 28, 2, 24, 0, 0),
  TestTimeParamsStruct(std::string("feb_29_2024"), 762548280, 0, 58, 18, 4, 29, 2, 24, 0, 0),
  TestTimeParamsStruct(std::string("mar_1_2024"), 762630313, 13, 45, 17, 5, 1, 3, 24, 0, 0),
  TestTimeParamsStruct(std::string("mar_2_2024"), 762716713, 13, 45, 17, 6, 2, 3, 24, 0, 0),
  TestTimeParamsStruct(std::string("mar_3_2024"), 762803113, 13, 45, 17, 7, 3, 3, 24, 0, 0),
  TestTimeParamsStruct(std::string("mar_4_2024"), 762889513, 13, 45, 17, 1, 4, 3, 24, 0, 0),
  TestTimeParamsStruct(std::string("mar_5_2024"), 762975913, 13, 45, 17, 2, 5, 3, 24, 0, 0),
  TestTimeParamsStruct(std::string("mar_6_2024"), 763062313, 13, 45, 17, 3, 6, 3, 24, 0, 0),
  TestTimeParamsStruct(std::string("mar_7_2024"), 763148713, 13, 45, 17, 4, 7, 3, 24, 0, 0),
  TestTimeParamsStruct(std::string("april_6_24"), 765740713, 13, 45, 17, 6, 6, 4, 24, 0, 60*60),
  TestTimeParamsStruct(std::string("may_10_2024"), 768678313, 13, 45, 17, 5, 10, 5, 24, 0, 60*60),
  TestTimeParamsStruct(std::string("june_10_2024"), 771356713, 13, 45, 17, 1, 10, 6, 24, 0, 60*60),
  TestTimeParamsStruct(std::string("july_10_2024"), 773948713, 13, 45, 17, 3, 10, 7, 24, 0, 60*60),
  TestTimeParamsStruct(std::string("aug_10_2024"), 776627113, 13, 45, 17, 6, 10, 8, 24, 0, 60*60),
  TestTimeParamsStruct(std::string("sep_10_2024"), 779305513, 13, 45, 17, 2, 10, 9, 24, 0, 60*60),
  TestTimeParamsStruct(std::string("oct_10_2024"), 781897513, 13, 45, 17, 4, 10, 10, 24, 0, 0),
  TestTimeParamsStruct(std::string("nov_10_2024"), 784575913, 13, 45, 17, 7, 10, 11, 24, 0, 0),
  TestTimeParamsStruct(std::string("dec_10_2024"), 787167913, 13, 45, 17, 2, 10, 12, 24, 0, 0),
  TestTimeParamsStruct(std::string("june_30_2023_1hr_dst"), 804600317, 17, 5, 12, 1, 30, 6, 25, 0, 60*60),
  TestTimeParamsStruct(std::string("july_31_25_1hr_dst_minus_8hr_timezone"), 807244446, 6, 34, 2, 4, 31, 7, 25, -8*60*60, 60*60),
  TestTimeParamsStruct(std::string("nov_30_26_plus_3hrs_timezone"), 849321246, 6, 34, 2, 1, 30, 11, 26, 3*60*60, 0),
  TestTimeParamsStruct(std::string("max negative timezone"), 771356713, 13, 45, 17, 1, 10, 6, 24, -48*15*60, 60*60),
  TestTimeParamsStruct(std::string("max positive timezone"), 771356713, 13, 45, 17, 1, 10, 6, 24, 56*15*60, 60*60),
  TestTimeParamsStruct(std::string("half hour DST"), 784575913, 13, 45, 17, 7, 10, 11, 24, (10*60 + 30)*60, 60*30), // literally just one island off Australia
  TestTimeParamsStruct(std::string("2 hour DST"), 771356713, 13, 45, 17, 1, 10, 6, 24, 0, 60*60*2) // used by England in WW2 and that's it l0l
};