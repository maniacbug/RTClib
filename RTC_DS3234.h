// Code by JeeLabs http://news.jeelabs.org/code/
// Released to the public domain! Enjoy!

#ifndef __RTC_DS3234_H__
#define __RTC_DS3234_H__

#include <RTClib.h>

// RTC based on the DS3234 chip connected via SPI and the SPI library
class RTC_DS3234
{
public:
    RTC_DS3234(int _cs_pin): cs_pin(_cs_pin) {}
    uint8_t begin(void);
    void adjust(const DateTime& dt);
    uint8_t isrunning(void);
    DateTime now();

    // temperature register
    float get_temperature_degC();
    
    // alarms
    // 1
    void set_alarm_1(const uint8_t s, const uint8_t mi, const uint8_t h, const uint8_t d, const uint8_t * flags);
    void enable_alarm_1();
    //void get_alarm_1(const uint8_t pin, char *buf, const uint8_t len);
    //void clear_a1f(const uint8_t pin);
    //uint8_t triggered_a1(const uint8_t pin);
    // 2
    void set_alarm_2(const uint8_t mi, const uint8_t h, const uint8_t d, const uint8_t * flags);
    void enable_alarm_2();
    //void get_alarm_2(const uint8_t pin, char *buf, const uint8_t len);
    //void clear_a2f(const uint8_t pin);
    //uint8_t triggered_a2(const uint8_t pin);
    // Both
    void enable_alarm(const uint8_t alarm_number); // Sets the bits to enable that alarm's interrupt; may inadvertently disable other alarm...?
    void clear_alarm_flag(const uint8_t alarm_number); // Clears alarm flags; release pulldown on INTERRUPT pin

		// Time-setting functions
		// Note that none of these check for sensibility: You can set the
		// date to July 42nd and strange things will probably result.
		
		void setSecond(uint8_t Second); 
			// In addition to setting the seconds, this clears the 
			// "Oscillator Stop Flag".
		void setMinute(uint8_t Minute); 
			// Sets the minute
		void setHour(uint8_t Hour); 
			// Sets the hour
		void setDoW(uint8_t DoW); 
			// Sets the Day of the Week (1-7);
		void setDate(uint8_t Date); 
			// Sets the Date of the Month
		void setMonth(uint8_t Month); 
			// Sets the Month of the year
		void setYear(uint8_t Year); 
			// Last two digits of the year
		void setClockMode(bool h12); 
			// Set 12/24h mode. True is 12-h, false is 24-hour.
    
    

protected:
    void cs(int _value);

    // control/status register -- might just break out into separate functions though
    // void DS3234_set_creg(const uint8_t pin, const uint8_t val);

private:
    int cs_pin;
    // Control register -- get and set bits/bytes
    uint8_t get_addr(const uint8_t addr);
    void set_addr(const uint8_t addr, const uint8_t val);
    // Helpers
    uint8_t decToBcd(const uint8_t);
    //uint8_t bcdtodec(const uint8_t); // currently unused
};

#endif // __RTC_DS3234_H__

// vim:ai:cin:sw=4 sts=4 ft=cpp
