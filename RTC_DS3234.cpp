// Code by JeeLabs http://news.jeelabs.org/code/
// Released to the public domain! Enjoy!

// SPI_MODE1 --> SPI_Mode3 by ADW; fails in my test if another SPI device is
// attached and it is in SPI_MODE1, even if that devices CSpin is OUTPUT, HIGH

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#include <avr/pgmspace.h>
#include <SPI.h>
#include "RTClib.h"
#include "RTC_DS3234.h"

////////////////////////////////////////////////////////////////////////////////
// RTC_DS3234 implementation

// Registers we use
const int CONTROL_R = 0x0e;
const int CONTROL_W = 0x8e;
const int CONTROL_STATUS_R = 0x0f;
const int CONTROL_STATUS_W = 0x8f;
const int SECONDS_R = 0x00;
const int SECONDS_W = 0x80;

// Bits we use
const int EOSC = 7;
const int OSF = 7;

// Alarm enable -- mostly commented out becaause 1 and 2 are alarm numbers, so can be written in functions in an intuitive way in decimal
//const int A1IE = 0x01; // bit 0: Alarm1 interrupt enable (1 to enable)
//const int A2IE = 0x02; // bit 1: Alarm2 interrupt enable (1 to enable)
const int INTCN = 0x04; // bit 2: Interrupt control (1 for use of the alarms and to disable square wave)

// Alarm status
//const int A1F = 0x01; // bit 0: Alarm 1 Flag - (1 if alarm 1 was triggered)
//const int A2F = 0x02;
//const int DS3234_OSF = 0x80; // Not using oscillator stop flag



uint8_t RTC_DS3234::begin(void)
{
    pinMode(cs_pin,OUTPUT);
    cs(HIGH);
    SPI.setBitOrder(MSBFIRST);

    //Ugh!  In order to get this to interop with other SPI devices,
    //This has to be done in cs()
    SPI.setDataMode(SPI_MODE3);

    //Enable oscillator, disable square wave, alarms
    cs(LOW);
    SPI.transfer(CONTROL_W);
    SPI.transfer(0x0);
    cs(HIGH);
    delay(1);

    //Clear oscilator stop flag, 32kHz pin
    cs(LOW);
    SPI.transfer(CONTROL_STATUS_W);
    SPI.transfer(0x0);
    cs(HIGH);
    delay(1);

    return 1;
}

void RTC_DS3234::cs(int _value)
{
    SPI.setDataMode(SPI_MODE3);
    digitalWrite(cs_pin,_value);
}

uint8_t RTC_DS3234::isrunning(void)
{
    cs(LOW);
    SPI.transfer(CONTROL_R);
    uint8_t ss = SPI.transfer(-1);
    cs(HIGH);
    return !(ss & _BV(OSF));
}

void RTC_DS3234::adjust(const DateTime& dt)
{
    cs(LOW);
    SPI.transfer(SECONDS_W);
    SPI.transfer(bin2bcd(dt.second()));
    SPI.transfer(bin2bcd(dt.minute()));
    SPI.transfer(bin2bcd(dt.hour()));
    SPI.transfer(bin2bcd(dt.dayOfWeek()));
    SPI.transfer(bin2bcd(dt.day()));
    SPI.transfer(bin2bcd(dt.month()));
    SPI.transfer(bin2bcd(dt.year() - 2000));
    cs(HIGH);

}

// Alarms from DS3234 library by Petre Rodan, with enabling functions by Andy
// Wickert so the user doesn't have to know which register to use and the 
// end code looks cleaner (even if it requires more functions up here)
// CS pin setting removed, since this is part of RTClib, in which these are 
// defined already

void RTC_DS3234::set_alarm_1(const uint8_t s, const uint8_t mi, const uint8_t h, const uint8_t d, const uint8_t * flags)
// flags are: A1M1 (seconds), A1M2 (minutes), A1M3 (hour), 
// A1M4 (day) 0 to enable, 1 to disable, DY/DT (dayofweek == 1/dayofmonth == 0)
//
// flags define what calendar component to be checked against the current time in order
// to trigger the alarm - see datasheet
// A1M1 (seconds) (0 to enable, 1 to disable)
// A1M2 (minutes) (0 to enable, 1 to disable)
// A1M3 (hour)    (0 to enable, 1 to disable) 
// A1M4 (day)     (0 to enable, 1 to disable)
// DY/DT          (dayofweek == 1/dayofmonth == 0)
// 
// So in short, if the mask bits are "1", the alarm will always go off
// regardless of this factor; if the mask bits are 0, they will check your
// value for this. So every minute on the "30 s" requires, e.g.,
// set_alarm_1(alarm_pin, 30, 0, 0, 0, {0, 1, 1, 1, 1})
{
    bool t[4] = { s, mi, h, d };
    uint8_t i;

    for (i = 0; i <= 3; i++) {
        cs(LOW);
        SPI.transfer(i + 0x87);
        if (i == 3) {
            SPI.transfer(dectobcd(t[3]) | (flags[3] << 7) | (flags[4] << 6));
        } else
            SPI.transfer(dectobcd(t[i]) | (flags[i] << 7));
        cs(HIGH);
    }
}

void RTC_DS3234::set_alarm_2(const uint8_t mi, const uint8_t h, const uint8_t d,
                   const uint8_t * flags)
// flags are: A2M2 (minutes), A2M3 (hour), A2M4 (day) 0 to enable, 1 to disable, DY/DT (dayofweek == 1/dayofmonth == 0) - 
//
// flags define what calendar component to be checked against the current time in order
// to trigger the alarm
// A2M2 (minutes) (0 to enable, 1 to disable)
// A2M3 (hour)    (0 to enable, 1 to disable) 
// A2M4 (day)     (0 to enable, 1 to disable)
// DY/DT          (dayofweek == 1/dayofmonth == 0)
//
// See notes from Alarm 1
{
    uint8_t t[3] = { mi, h, d };
    uint8_t i;

    for (i = 0; i <= 2; i++) {
        digitalWrite(cs_pin, LOW);
        cs(LOW);
        if (i == 2) {
            SPI.transfer(dectobcd(t[2]) | (flags[2] << 7) | (flags[3] << 6));
        } else
            SPI.transfer(dectobcd(t[i]) | (flags[i] << 7));
        cs(HIGH);
    }
}

void RTC_DS3234::enable_alarm(const uint8_t alarm_number)
// Written like this thanks to the fact that these A1IE and A2IE are binary 1
// and 2, which are also decimal 1 and 2: so just pass the number of that alarm
// INTCN is the interrupt enable bit
{
    cs(LOW);
    set_addr(0x8E, INTCN | alarm_number);
    cs(HIGH);
}

// when the alarm flag is cleared the pulldown on INT is also released
void RTC_DS3234::clear_alarm_flag(const uint8_t alarm_number)
{
    uint8_t reg_val;
    cs(LOW);
    uint8_t addr_in = get_addr(0x0F);
    reg_val = addr_in & ~alarm_number;
    set_addr(0x8F, 0);
    cs(HIGH);
}

/*
// Old one in case I did something wrong here
void Logger::DS3234_clear_a1f()
{
    uint8_t reg_val;

    DS3234_A1F = 0x1;
    reg_val = DS3234_get_sreg(CSpinRTC) & ~DS3234_A1F;
    DS3234_set_sreg(pin, reg_val);
}

uint8_t Logger::DS3234_get_sreg(const uint8_t pin)
{
    uint8_t rv;
    rv = DS3234_get_addr(pin, 0x0f);
    return rv;
}

void DS3234_set_sreg(const uint8_t pin, const uint8_t sreg)
{
    DS3234_set_addr(pin, 0x8F, sreg);
}
*/



// Temperature
float RTC_DS3234::get_temperature_degC()
{
    float temp_degC;
    uint8_t temp_msb, temp_lsb;
    int8_t nint;

    cs(LOW);
    temp_msb = get_addr(0x11);
    temp_lsb = get_addr(0x12) >> 6;
    cs(HIGH);
    if ((temp_msb & 0x80) != 0)
        nint = temp_msb | ~((1 << 8) - 1);      // if negative get two's complement
    else
        nint = temp_msb;

    temp_degC = 0.25 * temp_lsb + nint;

    return temp_degC;
}

// Addressing

uint8_t RTC_DS3234::get_addr(const uint8_t addr)
{
    uint8_t rv;
    cs(LOW); // Somehow this is still needed in spite of the fact that this works within other functions that already do this... strange
    SPI.transfer(addr);
    rv = SPI.transfer(0x00);
    cs(HIGH);
    return rv;
}

void RTC_DS3234::set_addr(const uint8_t addr, const uint8_t val)
{
    cs(LOW); // Somehow this is still needed in spite of the fact that this works within other functions that already do this... strange
    SPI.transfer(addr);
    SPI.transfer(val);
    cs(HIGH);
}

// helpers from DS3234 library by Petre Rodan -- just 1 now for setting the
// alarms

uint8_t RTC_DS3234::dectobcd(const uint8_t val)
{
    return ((val / 10 * 16) + (val % 10));
}

/*
Uncomment if I decide to include the "get" functions.
uint8_t RTC_DS3234::bcdtodec(const uint8_t val)
{
    return ((val / 16 * 10) + (val % 16));
}
*/

// end helpers

DateTime RTC_DS3234::now()
{
    cs(LOW);
    SPI.transfer(SECONDS_R);
    uint8_t ss = bcd2bin(SPI.transfer(-1) & 0x7F);
    uint8_t mm = bcd2bin(SPI.transfer(-1));
    uint8_t hh = bcd2bin(SPI.transfer(-1));
    SPI.transfer(-1);
    uint8_t d = bcd2bin(SPI.transfer(-1));
    uint8_t m = bcd2bin(SPI.transfer(-1));
    uint16_t y = bcd2bin(SPI.transfer(-1)) + 2000;
    cs(HIGH);

    return DateTime (y, m, d, hh, mm, ss);
}

// vim:ai:cin:sw=4 sts=4 ft=cpp
