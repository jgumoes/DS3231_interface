#ifndef __BLE_STRUCTS_HPP__
#define __BLE_STRUCTS_HPP__

/**
 * @file BLE_structs.hpp
 * @author your name (you@domain.com)
 * @brief at some point I'm going to implement bluetooth time updates (either Current Time Service or Device Time Service), and I want this library to be compatible. so, i'm writting out some of the specifications here to make that process easier when it happens
 * @version 0.1
 * @date 2025-06-25
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include <stdint.h>

namespace BluetoothStructs
{
  /**
   * @brief Day of Week, as defined in GATT Specification Supplement 3.74
   * 
   */
  enum class DayOfWeek : uint8_t{
    Unknown = 0,
    Monday = 1,
    Tuesday = 2,
    Wednesday = 3,
    Thursday = 4,
    Friday = 5,
    Saturday = 6,
    Sunday = 7,
  };
  
  /**
   * @brief Month field, as defined in GATT Specification Supplement 3.70.1, table 3.127. it matches the description in 3.71 Date Time
   * 
   */
  enum class Month : uint8_t {
    Unknown = 0,
    January = 1,
    February = 2,
    March = 3,
    April = 4,
    May = 5,
    June = 6,
    July = 7,
    August = 8,
    September = 9,
    October = 10,
    November = 11,
    December = 12,
  };
  
  /**
   * @brief Date Time, as defined in GATT Specification Supplement 3.71.1
   * 
   */
  struct DateTimeStruct{
    uint16_t year;  // year, between 1582 and 9999. 0 means year is unknown

    Month month;

    uint8_t day;    // i.e. Date, day in the month. 0 means month is unknown

    uint8_t hours;  // hours past midnight, between 0 and 23

    uint8_t minutes;

    uint8_t seconds;
  };

  /**
   * @brief Day Date Time, as defined in GATT Specification Supplement 3.73
   * 
   */
  struct DayDateTimeStruct{
    DateTimeStruct dateTime;

    DayOfWeek dayOfWeek;
  };
  
} // namespace BluetoothStructs



#endif