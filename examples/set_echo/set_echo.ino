/*

Sets the time and prints back time stamps for 5 seconds

Based on DS3231_set.pde
by Eric Ayars
4/11

Added printing back of time stamps and increased baud rate
Andy Wickert
5/15/2011

Ported to the DS3234, using my modification of RTClib
Contribs in backwards order through time:
Andy Wickert, maniacbug, Petre Rodan, jeelabs, adafruit
5/16/2014 -- whoa, 3 years already!

*/

#include <SPI.h>
#include <RTC.h>
#include <stdio.h>
#include <RTC_DS3234.h>

RTC_DS3234 Clock(47);

byte Year;
byte Month;
byte Date;
byte DoW;
byte Hour;
byte Minute;
byte Second;

bool Century=false;
bool h12;
bool PM;

void GetDateStuff(byte& Year, byte& Month, byte& Day, byte& DoW, 
		byte& Hour, byte& Minute, byte& Second) {
	// Call this if you notice something coming in on 
	// the serial port. The stuff coming in should be in 
	// the order YYMMDDwHHMMSS, with an 'x' at the end.
	boolean GotString = false;
	char InChar;
	byte Temp1, Temp2;
	char InString[20];

	byte j=0;
	while (!GotString) {
		if (Serial.available()) {
			InChar = Serial.read();
			InString[j] = InChar;
			j += 1;
			if (InChar == 'x') {
				GotString = true;
			}
		}
	}
	Serial.println(InString);
	// Read Year first
	Temp1 = (byte)InString[0] -48;
	Temp2 = (byte)InString[1] -48;
	Year = Temp1*10 + Temp2;
	// now month
	Temp1 = (byte)InString[2] -48;
	Temp2 = (byte)InString[3] -48;
	Month = Temp1*10 + Temp2;
	// now date
	Temp1 = (byte)InString[4] -48;
	Temp2 = (byte)InString[5] -48;
	Day = Temp1*10 + Temp2;
	// now Day of Week
	DoW = (byte)InString[6] - 48;		
	// now Hour
	Temp1 = (byte)InString[7] -48;
	Temp2 = (byte)InString[8] -48;
	Hour = Temp1*10 + Temp2;
	// now Minute
	Temp1 = (byte)InString[9] -48;
	Temp2 = (byte)InString[10] -48;
	Minute = Temp1*10 + Temp2;
	// now Second
	Temp1 = (byte)InString[11] -48;
	Temp2 = (byte)InString[12] -48;
	Second = Temp1*10 + Temp2;
}

DateTime now;
const int len = 32;
static char buf[len];

void setup() {
	// Start the serial port
	Serial.begin(57600);

	// Start the I2C interface
	SPI.begin();
        Clock.begin();
}

void loop() {

	// If something is coming in on the serial line, it's
	// a time correction so set the clock accordingly.
	if (Serial.available()) {
		GetDateStuff(Year, Month, Date, DoW, Hour, Minute, Second);

    Serial.println();
    Serial.println("Transferred time from computer to board:");
    Serial.print(Year);
    Serial.print(F("-"));
    Serial.print(Month);
    Serial.print(F("-"));
    Serial.print(Date);
    Serial.print(F(" "));
    Serial.print(Hour); //24-hr
    Serial.print(F(":"));
    Serial.print(Minute);
    Serial.print(F(":"));
    Serial.println(Second);
    Serial.println();

		Clock.setClockMode(false);	// set to 24h
		//setClockMode(true);	// set to 12h
                
		Clock.setYear(Year);
		Clock.setMonth(Month);
		Clock.setDate(Date);
		Clock.setDoW(DoW);
		Clock.setHour(Hour);
		Clock.setMinute(Minute);
		Clock.setSecond(Second);
		
		// Give time at next five seconds
		for (int i=0; i<5; i++){
		    delay(1000);
            now = Clock.now();
            Serial.println(now.unixtime());
            Serial.println(now.toString(buf,len));
            Serial.println();
		}

	}
	delay(1000);
}
