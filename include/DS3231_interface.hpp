#ifndef __DS3231_INTERFACE_HPP__
#define __DS3231_INTERFACE_HPP__

#include <Arduino.h>
#include <stdint.h>
#include <Wire.h>

#include "BLE_structs.hpp"

#ifndef WIRE_DEPENDANCY
  #define WIRE_DEPENDANCY
  typedef typeof(Wire) WireDependancy;
#endif

namespace DS3231_Interface
{
  // constexpr uint16_t clockAddress = 0x68; // TODO: double check type
  // constexpr uint8_t DS3231_address = 0b1101000;  // 7 bit chip address
  constexpr uint8_t DS3231_address = 0x68;  // 7 bit chip address

  constexpr uint8_t BCDMask = 0b00001111;
  constexpr uint8_t bit4Mask = 0b00011111;  // mask to bit 4, i.e. the first 5 bits
  
  uint8_t bcdToDec (uint8_t val){
    return (val & BCDMask) + ((val >> 4) * 10);
  };
  
  uint8_t decToBcd(uint8_t val){
    return ( ((val/10) << 4) + (val%10) );
  };

  const uint8_t monthDays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

  /**
   * @brief converts a local DayDateTime struct into a 2000 epoch local timestamp
   * 
   * @param dateTime input DayDateTime struct
   * @param timestamp_S local timestamp in seconds since 2000
   */
  inline void convertToLocalTimestamp(
    const BluetoothStructs::DayDateTimeStruct &dateTime,
    uint64_t *timestamp_S
  ){
    using namespace BluetoothStructs;

    const uint8_t year = dateTime.dateTime.year - 2000;
    uint16_t dayOfYear = dateTime.dateTime.day;
    for(
      uint8_t month = 1;
      month < static_cast<uint8_t>(dateTime.dateTime.month);
      month++
    ){
      dayOfYear += monthDays[month - 1];
    }

    if(
      (year % 4 == 0)
      && (dateTime.dateTime.month <= Month::February)
    ){
      // remove the extra day if it hasn't happened yet
      dayOfYear -= 1;
    }
    
    *timestamp_S = (dateTime.dateTime.year - 2000) * 365.25;  // in days. adds an extra day on a leap year, which we might not want
    *timestamp_S += dayOfYear;
    *timestamp_S = (*timestamp_S * 24) + dateTime.dateTime.hours;
    *timestamp_S = (*timestamp_S * 60) + dateTime.dateTime.minutes;
    *timestamp_S = (*timestamp_S * 60) + dateTime.dateTime.seconds;
  };

  inline void convertToBLETimeStruct(
    uint64_t timestamp_S,
    BluetoothStructs::DayDateTimeStruct &dateTime
  ){
    // TODO: this could be better
    using namespace BluetoothStructs;
    dateTime.dateTime.seconds = timestamp_S % 60;   // i.e. divide by number of minutes and take remainder
    timestamp_S /= 60;             // timestamp_S is now in minutes
    dateTime.dateTime.minutes = timestamp_S % 60;   // i.e. divide by number of hours and take remainder
    timestamp_S /= 60;             // timestamp_S is now in hours
    dateTime.dateTime.hours = timestamp_S % 24;     // i.e. divide by number of days and take remainder
    timestamp_S /= 24;             // timestamp_S is now in days since 1/1/2000

    dateTime.dayOfWeek = static_cast<DayOfWeek>(((timestamp_S) + 5) % 7 + 1);  // epoch starts on a saturday
    
    dateTime.dateTime.year = ((4 * timestamp_S) / 1461);
    timestamp_S -= (dateTime.dateTime.year * 365.25) - 1;   // timestamp_S is now in day of the year
                                                            // +1 because day of the year isn't zero-indexed

    uint8_t leapDay = (!(dateTime.dateTime.year % 4) && (timestamp_S > 59)); // knock of a day if its a leap year and after Feb 28
    timestamp_S -= leapDay;
    uint8_t i = 0;
    while (timestamp_S > monthDays[i]){
        timestamp_S -= monthDays[i];
        i++;
    }
    dateTime.dateTime.month = static_cast<Month>(i + 1);
    dateTime.dateTime.day = timestamp_S + (leapDay && (dateTime.dateTime.month == Month::February)); // add the leap day back if it's actually Feb 29
    dateTime.dateTime.year += 2000;
  };

  /**
   * @brief start addresses of the register bytes. Aging and Temp offsets are omitted because I'm not using them
   * 
   */
  namespace Address {
    constexpr uint8_t time = 0;
    constexpr uint8_t alarm1 = 0x07;
    constexpr uint8_t alarm2 = 0x0B;
    constexpr uint8_t control = 0x0E;
    constexpr uint8_t status = 0x0F;
  };

  namespace Size{
    constexpr uint8_t time = Address::alarm1 - Address::time;
    constexpr uint8_t alarm1 = Address::alarm2 - Address::alarm1;
    constexpr uint8_t alarm2 = Address::control - Address::alarm2;
    constexpr uint8_t control = Address::status - Address::control;
    constexpr uint8_t status = 1;
  }

  /*  =============================================
                  Register Bits
      ============================================= */

  enum class SquareWaveFrequency : int8_t{
    off = -1,
    hZ_1 = 0,    // 1 Hz
    hZ_1024 = 1, // 1.024 kHz
    hZ_4096 = 2, // 4.096 kHz
    hZ_8192 = 3, // 8.192 kHz
  };

  namespace ControlBitPositions{
    constexpr uint8_t alarm1 = 0;
    constexpr uint8_t alarm2 = 1;
    constexpr uint8_t interruptControl = 2;
    constexpr uint8_t squareWaveFreq = 3;
    constexpr uint8_t temp = 5;
    constexpr uint8_t batterySquareWave = 6;
    constexpr uint8_t enableOscillator = 7;
  };
      
  namespace statusRegisterBits{
    constexpr uint8_t enable32 = (1<<3);  // enable 32 kHz square wave
    constexpr uint8_t OSF = (1<<7); // Ocsillator Stop Flag: 1 if the oscillator has stopped i.e. if power fault, first time power is applied
  }
  
  class InterfaceClass{
  public:
    InterfaceClass(WireDependancy &wireClass) : _wire(&wireClass) {}
  
    /**
     * @brief IMPORTANT: make sure 2 seconds have passed before calling! first read from the RTC chip. resets alarms, checks for faults, and writes the local timestamp in seconds.
     * 
     * @return bool if there is a time fault
     */
    bool initialise(uint64_t *localTime_S);
  
    /**
     * @brief Set the RTC time from a local timestamp
     * 
     * @param localTime_S local timestamp in seconds
     * @retval true if operation was successful
     * @retval false if read failed
     */
    bool setTime(uint64_t localTime_S);
  
    /**
     * @brief Set the local time with a DayDateTime struct
     * 
     * @param dateTime 
     * @retval true if operation was successful
     * @retval false if read failed
     */
    bool setTime(BluetoothStructs::DayDateTimeStruct dateTime);
  
    /**
     * @brief Get the stored timestamp in seconds since 2000. use hasTimeFault() to see if the time can be trusted
     * 
     * @param localTime_S 
     * @retval true if operation was successful
     * @retval false if read failed
     */
    bool getTimestamp(uint64_t *localTime_S);
  
    /**
     * @brief reads the time from the chip, and checks values for validity. use hasTimeFault() to see if the time can be trusted
     * 
     * @param localDateTime 
     * @retval true if operation was successful
     * @retval false if read failed
     */
    bool getDateTime(BluetoothStructs::DayDateTimeStruct *localDateTime);
  
    /**
     * @brief reads the status register to see if the Oscillator Stop Flag is high. if it is, the oscillator has stopped and time can't be trusted.
     * 
     * @return bool time fault flag
     */
    bool checkStopFlag();

    /**
     * @brief returns the time fault flag. timeFault can be cleared by setting a new time
     * 
     * @return bool
     * @retval true if there's a time fault
     * @retval false if there isn't a time fault
     */
    bool hasTimeFault(){return _timeFault;}
    
    /**
     * @brief Set the Control Register. params are ordered by how likely i think they are to be used. setting a square wave will disable alarms, but setting alarms won't disable the square wave
     * 
     * @param freq square wave frequency. if set, this disables alarms
     * @param alarm1 enable alarm 1
     * @param alarm2 enable alarm 2
     * @param workWithBattery enable the oscillator that keeps time when on battery power
     * @param convertTemperature force a temperature conversion. status should be read to ensure conversion is not already taking place
     * @param batterySquareWave allow square wave when powered by backup battery
     * @return bool if the operation was successful
     */
    bool setControlRegister(
      SquareWaveFrequency freq = SquareWaveFrequency::off,
      bool alarm1 = false,
      bool alarm2 = false,
      bool workWithBattery = true,
      bool convertTemperature = false,
      bool batterySquareWave = false
    );

    /**
     * @brief read the control register
     * 
     * @param buffer single byte buffer to write to
     * @return bool if operation was successful
     */
    bool readControlRegister(byte *buffer);
    
    static const uint8_t bufferSize = static_cast<uint8_t>(Address::status);
    
  private:

    WireDependancy *_wire;
  
    bool _timeFault = true;
    uint8_t _enable32Output = false;
  };
} // namespace DS3231_Interface

bool DS3231_Interface::InterfaceClass::initialise(uint64_t *localTime_S)
{
  setControlRegister();
  
  checkStopFlag();
  
  getTimestamp(localTime_S);

  return _timeFault;
}

bool DS3231_Interface::InterfaceClass::setTime(uint64_t localTime_S)
{
  BluetoothStructs::DayDateTimeStruct timeStruct;
  convertToBLETimeStruct(localTime_S, timeStruct);
  return setTime(timeStruct);
}

bool DS3231_Interface::InterfaceClass::setTime(BluetoothStructs::DayDateTimeStruct dateTime)
{
  _wire->flush();
  byte buffer[Size::time+1] = {
    Address::time,
    decToBcd(dateTime.dateTime.seconds),
    decToBcd(dateTime.dateTime.minutes),
    decToBcd(dateTime.dateTime.hours),
    static_cast<uint8_t>(dateTime.dayOfWeek),
    decToBcd(dateTime.dateTime.day),
    decToBcd(static_cast<uint8_t>(dateTime.dateTime.month)),
    decToBcd(dateTime.dateTime.year - 2000),
  };

  _wire->beginTransmission(DS3231_address);
  // _wire->write(buffer, Size::time+1);
  _wire->write(buffer[0]);
  _wire->write(buffer[1]);
  _wire->write(buffer[2]);
  _wire->write(buffer[3]);
  _wire->write(buffer[4]);
  _wire->write(buffer[5]);
  _wire->write(buffer[6]);
  _wire->write(buffer[7]);
  uint8_t error = _wire->endTransmission();
  if(0 != error){
    _timeFault = true;
    return false;
  }

  byte statusBuffer[2] = {
    Address::status,
    (_enable32Output == 0 ? 0 : statusRegisterBits::enable32)
  };
  _wire->flush();
  _wire->beginTransmission(DS3231_address);
  _wire->write(statusBuffer, 2);
  error = _wire->endTransmission();
  if(0 != error){
    _timeFault = true;
    return false;
  }

  _timeFault = false;
  return true;
}

bool DS3231_Interface::InterfaceClass::getTimestamp(uint64_t *localTime_S)
{
  using namespace BluetoothStructs;
  *localTime_S = 0;
  DayDateTimeStruct localStruct;
  if(!getDateTime(&localStruct)){
    return false;
  }

  convertToLocalTimestamp(localStruct, localTime_S);
  
  return true;
}

bool DS3231_Interface::InterfaceClass::getDateTime(BluetoothStructs::DayDateTimeStruct *localTime)
{
  using namespace BluetoothStructs;
  _wire->flush();
  _wire->beginTransmission(DS3231_address);
  _wire->write(Address::time);
  _wire->endTransmission();

  byte buffer[Size::time];
  _wire->requestFrom(DS3231_address, Size::time);
  if(_wire->readBytes(buffer, Size::time) != Size::time){
    return false;
  }

  localTime->dateTime.seconds = bcdToDec(buffer[0]);
  localTime->dateTime.minutes = bcdToDec(buffer[1]);
  
  if((buffer[3] & (1<<6)) == 0){
    // if 24 hours
    localTime->dateTime.hours = bcdToDec(buffer[2]);
  }
  else{
    // if 12 hours
    localTime->dateTime.hours
      = bcdToDec(buffer[2] & bit4Mask)
      + (buffer[2] & (1<<5)) * 12;
  }

  localTime->dayOfWeek = static_cast<DayOfWeek>(buffer[3]);
  if(localTime->dayOfWeek > DayOfWeek::Sunday){
    localTime->dayOfWeek = DayOfWeek::Unknown;
    _timeFault = true;
  }

  localTime->dateTime.day = bcdToDec(buffer[4]);
  if(localTime->dateTime.day == 0){
    _timeFault = true;
  }

  localTime->dateTime.month = static_cast<Month>(bcdToDec(buffer[5] & bit4Mask));
  if(localTime->dateTime.month > Month::December){
    localTime->dateTime.month = Month::Unknown;
    _timeFault = true;
  }

  if((buffer[5] & 0b11100000)){
    // century bit will be unused, bits 5-7 are high there's a read error
    _timeFault = true;
  }

  localTime->dateTime.year = (uint16_t)2000 + bcdToDec(buffer[6]);
  if(
    (localTime->dateTime.year < 2000)
    || (localTime->dateTime.year > 2100)
  ){
    // tighter than allowed by BLE, but a lower limit that is post-Shakespear seams a bit more sensible
    localTime->dateTime.year = 0;
    _timeFault = true;
  }
  checkStopFlag();
  return true;
}

bool DS3231_Interface::InterfaceClass::checkStopFlag()
{
  _wire->flush();
  _wire->beginTransmission(DS3231_address);
  _wire->write(Address::status);
  uint8_t error = _wire->endTransmission(); // TODO: this might need to be false
  _wire->requestFrom(DS3231_address, Size::status);
  if(0 < error){
    return true;
  }

  byte buffer;
  _wire->readBytes(&buffer, Size::status);
  if(0 != (buffer & statusRegisterBits::OSF)){
    _timeFault = true;
  };

  return _timeFault;
}

bool DS3231_Interface::InterfaceClass::setControlRegister(SquareWaveFrequency freq, bool alarm1, bool alarm2, bool workWithBattery, bool convertTemperature, bool batterySquareWave){
  _wire->flush();
  byte registerFlags
    = (alarm1 << ControlBitPositions::alarm1)
    + (alarm2 << ControlBitPositions::alarm2)
    + (convertTemperature << ControlBitPositions::temp)
    + (batterySquareWave << ControlBitPositions::batterySquareWave)
    + ((!workWithBattery) << ControlBitPositions::enableOscillator);
  
  if(SquareWaveFrequency::off == freq){
    registerFlags |= (1 << ControlBitPositions::interruptControl);
  }
  else{
    registerFlags |= (static_cast<uint8_t>(freq) << ControlBitPositions::squareWaveFreq);
  }

  byte writeBuffer[2] = {
    Address::control,
    registerFlags
  };

  _wire->beginTransmission(DS3231_address);
  _wire->write(writeBuffer, 2);
  uint8_t error = _wire->endTransmission();

  if(0 != error){
    _timeFault = true;
    return false;
  }
  return true;
}

bool DS3231_Interface::InterfaceClass::readControlRegister(byte *buffer)
{
  _wire->flush();
  _wire->beginTransmission(DS3231_address);
  _wire->write(Address::control);
  uint8_t error = _wire->endTransmission(true);
  if(0 != error){
    _timeFault = true;
    return false;
  }
  _wire->requestFrom(DS3231_address, Size::control);
  _wire->readBytes(buffer, Size::control);
  return true;
}


#endif