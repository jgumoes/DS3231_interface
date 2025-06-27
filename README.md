# DS3231 Interface

A minimal library for interfacing with the DS3231 RTC chip. The local time can be set or read via a local timestamp in seconds, or a DayDateTime struct consistent with the BLE Special Interest Group's Current Time Service.

## Usage

Basic setup is as follows:

```c++
// setup
Wire.begin(SDA_pin, SCL_pin);
delay(2000);  // allow 2 seconds for the DS3231 to sort itself out

InterfaceClass testClass(Wire); // inject the wire dependancy

uint64_t actualTime;
if(testClass.initialise(&actualTime)){
  // initialise returns true if the chip has a time fault
}
```

### Alternate Wire classes

This library uses the Arduino Wire class by default:

```c++
#ifndef WIRE_DEPENDANCY
  #define WIRE_DEPENDANCY
  typedef typeof(Wire) WireDependancy;
#endif
```

This can be overriden with a compatible dependancy:

```c++
#define WIRE_DEPENDANCY
typedef MyWireClass WireDependancy;  // the wire instance has to be a global object. yeah i don't like it either

#include "DS3231_interface.hpp"
```

## TODOs

* does the typedef override actually work?
* native tests for reading bad times
* abstract away WireDependancy without using pre-compile directives (CRTP?)
