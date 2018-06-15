/*
https://www.pjrc.com/teensy/td_libs_DS1307RTC.html

**************************************************
  - Connections for DS1307RTC
    - SDA   --   A4
    - SCL   --   A5
    - VCC   --   5V
    - GND   --   GND
    
  - Connections for LEDs
    - RED LED       --    A1  (Digital 15)
    - YELLOW LED    --    A0  (Digital 14)
    - GREEN LED     --    A3  (Digital 17)
**************************************************
*/
#include <Wire.h>
#include <RTClib.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <EEPROM.h>

const PROGMEM char msg1[] = {"Couldn't find RTC"};
const PROGMEM char msg2[] = {"Waiting 10 seconds to enter command mode"};
const PROGMEM char msg3[] = {"Change system date/time (y/n)? "};
const PROGMEM char msg4[] = {"Enter date and time (MM/DD/YYYY-HH:MN)? "};
const PROGMEM char msg5[] = {"Change Light Schedule (y/n/d)? "};
const PROGMEM char msg6[] = {"Enter a day of the week to change (0=Sun, 1=Mon, etc)? "};
const PROGMEM char msg7[] = {"Enter on times for red, yellow, and green lights and off time for green light in minutes (HH:MN-HH:MN-HH:MN-HH:MN)? "};
const PROGMEM char msg8[] = {"timed out waiting for reply"};

volatile int f_wdt=1;

// For 3 LEDs
#define RED 0
#define YELLOW 1
#define GREEN 2
                  //RED   //YELLOW  //GREEN
byte ledPins[3] = {15,    14,       17};
const byte ledCount = 3;
int currentLED = -1;
RTC_DS3231 rtc;
struct sched {
  byte red_hour;
  byte red_min;  
  byte yellow_hour;
  byte yellow_min;  
  byte green_on_hour;
  byte green_on_min;  
  byte green_off_hour;
  byte green_off_min;  
};
sched agenda[7];
int chday;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char strbuf[100];

void printSchedule() {
  int day;
  Serial.println("Day\t\tRed On\tYellow On\tGreen On\tGreen Off");
  for(day=0; day<7; day++){
    Serial.print(daysOfTheWeek[day]);
    Serial.print("\t");
    if(strlen(daysOfTheWeek[day]) <8) Serial.print("\t");
    Serial.print(agenda[day].red_hour);
    Serial.print(":");
    if(agenda[day].red_min < 10) Serial.print("0");
    Serial.print(agenda[day].red_min);
    Serial.print("\t");
    Serial.print(agenda[day].yellow_hour);
    Serial.print(":");
    if(agenda[day].yellow_min < 10) Serial.print("0");
    Serial.print(agenda[day].yellow_min);
    Serial.print("\t\t");
    Serial.print(agenda[day].green_on_hour);
    Serial.print(":");
    if(agenda[day].green_on_min < 10) Serial.print("0");
    Serial.print(agenda[day].green_on_min);
    Serial.print("\t\t");
    Serial.print(agenda[day].green_off_hour);
    Serial.print(":");
    if(agenda[day].green_off_min < 10) Serial.print("0");
    Serial.print(agenda[day].green_off_min);
    Serial.println();
  }
}

void printTime()
{
  DateTime now = rtc.now();
    
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print('/');
  Serial.print(now.year(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
}

void defaultsched() {
  for(chday=0; chday <7; chday++) { // set up the default schedule
    EEPROM.write(1+chday*8,6); // red hour
    EEPROM.write(2+chday*8,0); // red minute
    EEPROM.write(3+chday*8,6); // yellow hour
    EEPROM.write(4+chday*8,50); // yellow minute
    EEPROM.write(5+chday*8,7); // green on hour
    EEPROM.write(6+chday*8,0); // green on minute
    EEPROM.write(7+chday*8,7); // green off hour
    EEPROM.write(8+chday*8,15); // green off minute
  }
}

void setup () {
  char inbuf[32];
  int bufcnt = 0;
  uint8_t MM,DD,HH,MN;
  uint16_t YYYY;
  byte val;

  Serial.begin(9600);
  if (! rtc.begin()) {
    Serial.println(strcpy_P(strbuf,msg1)); // "Couldn't find RTC"
    while (1);
  }

  // Establish LED outputs 
  for (int thisPin = 0; thisPin < ledCount; thisPin++) {
    pinMode(ledPins[thisPin], OUTPUT);
    digitalWrite(ledPins[thisPin],LOW);
  }

  val = EEPROM.read(0);
  if(val != 0x4A) { // check for the magic word
    defaultsched(); // init the schedule
    EEPROM.write(0,0x4A); // set the magic word
  }
  for(chday=0; chday<7; chday++) {
    agenda[chday].red_hour = EEPROM.read(1+chday*8);
    agenda[chday].red_min = EEPROM.read(2+chday*8);
    agenda[chday].yellow_hour = EEPROM.read(3+chday*8);
    agenda[chday].yellow_min = EEPROM.read(4+chday*8);
    agenda[chday].green_on_hour = EEPROM.read(5+chday*8);
    agenda[chday].green_on_min = EEPROM.read(6+chday*8);
    agenda[chday].green_off_hour = EEPROM.read(7+chday*8);
    agenda[chday].green_off_min = EEPROM.read(8+chday*8);
  }
  // See if settings need to be changed
  Serial.println(strcpy_P(strbuf,msg2)); // "Waiting 10 seconds for you to type C to enter command mode"
  Serial.setTimeout(10000);
  bufcnt = Serial.readBytesUntil(10,inbuf,3);
  if(bufcnt > 0) {
    Serial.setTimeout(60000);
    printTime();
    Serial.print(strcpy_P(strbuf,msg3)); // "Change system date/time (y/n)? "
    bufcnt = 0;
    bufcnt = Serial.readBytesUntil(10,inbuf,3);
    if(bufcnt> 0) {
      inbuf[bufcnt]='\0';
      //Serial.println(inbuf);
      if(inbuf[0] == 'y') { // Change system date and time
        Serial.print(strcpy_P(strbuf,msg4)); //"Enter date and time (MM/DD/YYYY-HH:MN)? "
        bufcnt = 0;
        bufcnt = Serial.readBytesUntil(10,inbuf,20);
        if(bufcnt> 0) {
          inbuf[bufcnt]='\0';
          //Serial.println(inbuf);
          MM = (inbuf[0]-'0')*10 + (inbuf[1]-'0');
          DD = (inbuf[3]-'0')*10 + (inbuf[4]-'0');
          YYYY = (inbuf[6]-'0')*1000 + (inbuf[7]-'0')*100 + (inbuf[8]-'0')*10 + (inbuf[9]-'0');
          HH = (inbuf[11]-'0')*10 + (inbuf[12]-'0');
          MN = (inbuf[14]-'0')*10 + (inbuf[15]-'0');
          rtc.adjust(DateTime(YYYY,MM,DD,HH,MN,0));
          printTime();
        }
      } // Changed date/time
      printSchedule();
      Serial.print(strcpy_P(strbuf,msg5)); // "Change Light Schedule (y/n/d)? "
      bufcnt = 0;
      bufcnt = Serial.readBytesUntil(10,inbuf,3);
      if(bufcnt > 0) {
        inbuf[bufcnt]='\0';
        //Serial.println(inbuf);
        if(inbuf[0] == 'd') defaultsched(); // init the schedule
        while(inbuf[0] == 'y') {
          Serial.println(strcpy_P(strbuf,msg6)); // "Enter a day of the week to change (0=Sun, 1=Mon, etc)? "
          bufcnt = 0;
          bufcnt = Serial.readBytesUntil(10,inbuf,3);
          if(bufcnt > 0) {
            inbuf[bufcnt]='\0';
            //Serial.println(inbuf);
            chday = inbuf[0] - '0';
            Serial.println(strcpy_P(strbuf,msg7)); // "Enter on times for red, yellow, and green lights and off time for green light in minutes (HH:MN-HH:MN-HH:MN-HH:MN)? "
            bufcnt = 0;
            bufcnt = Serial.readBytesUntil(10,inbuf,31);
            if(bufcnt > 0) {
              inbuf[bufcnt]='\0';
              //Serial.println(inbuf);
              agenda[chday].red_hour = (inbuf[0]-'0')*10 + (inbuf[1]-'0');
              agenda[chday].red_min = (inbuf[3]-'0')*10 + (inbuf[4]-'0');
              agenda[chday].yellow_hour = (inbuf[6]-'0')*10 + (inbuf[7]-'0');
              agenda[chday].yellow_min = (inbuf[9]-'0')*10 + (inbuf[10]-'0');
              agenda[chday].green_on_hour = (inbuf[12]-'0')*10 + (inbuf[13]-'0');
              agenda[chday].green_on_min = (inbuf[15]-'0')*10 + (inbuf[16]-'0');
              agenda[chday].green_off_hour = (inbuf[18]-'0')*10 + (inbuf[19]-'0');
              agenda[chday].green_off_min = (inbuf[21]-'0')*10 + (inbuf[22]-'0');
              EEPROM.write(1+chday*8,agenda[chday].red_hour);
              EEPROM.write(2+chday*8,agenda[chday].red_min);
              EEPROM.write(3+chday*8,agenda[chday].yellow_hour);
              EEPROM.write(4+chday*8,agenda[chday].yellow_min);
              EEPROM.write(5+chday*8,agenda[chday].green_on_hour);
              EEPROM.write(6+chday*8,agenda[chday].green_on_min);
              EEPROM.write(7+chday*8,agenda[chday].green_off_hour);
              EEPROM.write(8+chday*8,agenda[chday].green_off_min);
              printSchedule();
            }
            else Serial.println(strcpy_P(strbuf,msg8)); // "timed out waiting for reply"
          }
          else Serial.println(strcpy_P(strbuf,msg8));
          Serial.print(strcpy_P(strbuf,msg5)); // "Change Light Schedule (y/n/d)? "
          bufcnt = 0;
          bufcnt = Serial.readBytesUntil(10,inbuf,3);
          if(bufcnt == 0) inbuf[0] = 'x';
          else {
            inbuf[bufcnt]='\0';
            //Serial.println(inbuf);
          }
        }
      } // Changed Schedule
      else Serial.println(strcpy_P(strbuf,msg8));
    }
    else Serial.println(strcpy_P(strbuf,msg8));
  }
  Serial.println("Starting clock");
  
  //*** Setup the Watch Dog Timer ***/
  
  /* Clear the reset flag. */
  MCUSR &= ~(1<<WDRF);
  
  /* In order to change WDE or the prescaler, we need to
   * set WDCE (This will allow updates for 4 clock cycles).
   */
  WDTCSR |= (1<<WDCE) | (1<<WDE);

  /* set new watchdog timeout prescaler value */
  WDTCSR = 1<<WDP0 | 1<<WDP3; /* 8.0 seconds */
  
  /* Enable the WD interrupt (note no reset). */
  WDTCSR |= _BV(WDIE);
}

void loop () {
  // Coming back from sleep, need to wait a few ms before reading the clock
  delay(30);
  DateTime now;

  // Read the clock to get the time.
  now = rtc.now();  
  // And also wait a couple ms before setting LEDs and going back to sleep
  delay(20);
  
  // Set LEDs based on hour and minute on the clock
  setLEDs(now.dayOfTheWeek(), now.hour(), now.minute());
  
  // For Testing comment out setLEDs above and uncomment to cycle through each LED for 8 seconds
  // testLEDs();
  
  // Clear the watchdog flag
  f_wdt = 0;
  
  // Go back to Sleep
  enterSleep();
}

void setLEDs(int dow, int hour, int minute)
{
  // Set Green at 7:00am for 15 minutes
  if(hour == agenda[dow].green_on_hour &&  minute == agenda[dow].green_on_min) {
    if(currentLED != GREEN) {
      currentLED = GREEN;
      clearLEDs();
      digitalWrite(ledPins[currentLED],HIGH);
    }
  // Set Yellow at 6:50am for 10 minutes
  } else if(hour == agenda[dow].yellow_hour &&  minute == agenda[dow].yellow_min) {
    if(currentLED != YELLOW) {
      currentLED = YELLOW;
      clearLEDs();
      digitalWrite(ledPins[currentLED],HIGH);
    }
  // Set Red at 6:00am for 50 minutes
  } else if(hour == agenda[dow].red_hour &&  minute == agenda[dow].red_min) {
    if(currentLED != RED) {
      currentLED = RED;
      clearLEDs();
      digitalWrite(ledPins[currentLED],HIGH);
    }
  // Turn off green LED
  } else if(hour == agenda[dow].green_off_hour &&  minute == agenda[dow].green_off_min){
    currentLED = -1;
    clearLEDs();
  }
}

void testLEDs()
{
  if(currentLED == 2) {
    currentLED = -1;
  } else {
    currentLED = currentLED + 1;
  }
  clearLEDs();
  digitalWrite(ledPins[currentLED],HIGH);
}

void clearLEDs()
{
  for (int thisPin = 0; thisPin < ledCount; thisPin++) {
    digitalWrite(ledPins[thisPin],LOW);
  }
}

// Interrupt service routine for watch dog timer
ISR(WDT_vect)
{
  if(f_wdt == 0)
  {
    f_wdt=1;
  }
}

void enterSleep(void)
{
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);   /* using SLEEP_MODE_PWR_DOWN for lowest power consumption. */
  sleep_enable();
  
  power_adc_disable();
  power_spi_disable();
  power_timer0_disable();
  power_timer2_disable();
  power_twi_disable();
  
  /* Now enter sleep mode. */
  sleep_mode();
  
  /* The program will continue from here after the WDT timeout*/
  sleep_disable(); /* First thing to do is disable sleep. */
  
  /* Re-enable the peripherals. */
  power_all_enable();
}

