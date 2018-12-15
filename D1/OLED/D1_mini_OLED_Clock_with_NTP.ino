//
//   There is very little original work here. Mostly this is a mashup of the following:
//
// - The D1 Mini OLED clock from the Wemos/Sparkfun examples (https://github.com/wemos/D1_mini_Examples)
// - Over the Air (OTA) updates                              (https://github.com/esp8266/Arduino/tree/master/libraries/ArduinoOTA)
// - NTP client to set up the clock automatically            (https://github.com/arduino-libraries/NTPClient)
// - WiFi Manager from Tzapu to simplify initial wifi setup  (https://github.com/tzapu/WiFiManager) 

#include <Wire.h>             // the D1 mini communicates with the OLED via I2C
#include <SFE_MicroOLED.h>    // the SFE_MicroOLED library to do all the drawing to the OLED
#include <ESP8266WiFi.h>      // we need WiFi!
#include <ArduinoOTA.h>       // so that we only need to install this software via physical connection once
#include <WiFiUdp.h>          // used by the NTP client to sent NTP UDP packets
#include <NTPClient.h>        // to get the initial time from an NTP server to set up the clock
#include <DNSServer.h>        // to give IP's to wifi clients
#include <ESP8266WebServer.h> // for WiFi manager below
#include <WiFiManager.h>      // WiFi manager to present web interface via Access Point to set up wifi credentials

#define PIN_RESET 255  //
#define DC_JUMPER 0  // I2C Addres: 0 - 0x3C, 1 - 0x3D


MicroOLED oled(PIN_RESET, DC_JUMPER);  // I2C Example
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "au.pool.ntp.org", 3600, 60000);

// global time variables
int hours   = 0;
int minutes = 0;
int seconds = 0;

// how fast you want the clock to spin, set this to 1000 to get it updating approx 1 second timing.
const int CLOCK_SPEED = 1000;    // 1 second (frequency with which to redraw the face)
const int NTP_REFRESH = 3600000; // 1 hour   (frequency with which to refresh the time, so it doesnt get too far out of sync with reality)

// Global variables to help draw the clock face:
const int MIDDLE_Y = oled.getLCDHeight() / 2;
const int MIDDLE_X = oled.getLCDWidth() / 2;

int CLOCK_RADIUS;
int POS_12_X, POS_12_Y;
int POS_3_X, POS_3_Y;
int POS_6_X, POS_6_Y;
int POS_9_X, POS_9_Y;
int S_LENGTH;
int M_LENGTH;
int H_LENGTH;

// keep track of when to refresh the face display and the NTP time
unsigned long lastDraw = 0;
unsigned long lastNTPRefresh = 0;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);

  WiFiManager wifiManager;
  wifiManager.autoConnect("MyD1MiniClock");

  Serial.println("connected to wifi!");
  
  ArduinoOTA.begin();

  oled.begin();     // Initialize the OLED
  oled.clear(PAGE); // Clear the display's internal memory
  oled.clear(ALL);  // Clear the library's display buffer
  oled.display();   // Display what's in the buffer (splashscreen)
  
  initClockVariables();

  timeClient.begin();
  timeClient.setTimeOffset(36000);
  timeClient.update();
  hours = timeClient.getHours();
  minutes = timeClient.getMinutes();
  seconds = timeClient.getSeconds();
  
  oled.clear(ALL);
  drawFace();
  drawArms(hours, minutes, seconds);
  oled.display(); // display the memory buffer drawn
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void loop() {

  // the only things we do are:
  // - check OTA
  // - check if NTP needs to be sync'd (eg hourly)
  // - check if we need to redraw the clock face (every second)
   
  ArduinoOTA.handle();

  // check if its time to refresh the time via NTP
  
  if (lastNTPRefresh + NTP_REFRESH < millis()){
    Serial.println("UPDATING NTP");
    lastNTPRefresh = millis();
    timeClient.update();
    hours = timeClient.getHours();
    minutes = timeClient.getMinutes();
    seconds = timeClient.getSeconds();
    Serial.print("H:");
    Serial.print(hours);
    Serial.print(" M:");
    Serial.print(minutes);
    Serial.print(" S:");
    Serial.println(seconds);
  }
  
  // check if we need to redraw the clock face
  
  if (lastDraw + CLOCK_SPEED < millis()) {
    lastDraw = millis();
    // add a second, update minutes/hours if necessary
    updateTime();
    
    // draw the clock
    oled.clear(PAGE);  // Clear the buffer
    drawFace();  // Draw the face to the buffer
    drawArms(hours, minutes, seconds);  // Draw arms to the buffer
    oled.display(); // Draw the memory buffer
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void initClockVariables() {
  
  // calculate constants for clock face component positions
  
  oled.setFontType(0);
  CLOCK_RADIUS = min(MIDDLE_X, MIDDLE_Y) - 1;
  POS_12_X = MIDDLE_X - oled.getFontWidth();
  POS_12_Y = MIDDLE_Y - CLOCK_RADIUS + 2;
  POS_3_X  = MIDDLE_X + CLOCK_RADIUS - oled.getFontWidth() - 1;
  POS_3_Y  = MIDDLE_Y - oled.getFontHeight()/2;
  POS_6_X  = MIDDLE_X - oled.getFontWidth()/2;
  POS_6_Y  = MIDDLE_Y + CLOCK_RADIUS - oled.getFontHeight() - 1;
  POS_9_X  = MIDDLE_X - CLOCK_RADIUS + oled.getFontWidth() - 2;
  POS_9_Y  = MIDDLE_Y - oled.getFontHeight()/2;
  
  // calculate clock arm lengths
  S_LENGTH = CLOCK_RADIUS - 2;
  M_LENGTH = S_LENGTH * 0.7;
  H_LENGTH = S_LENGTH * 0.5;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void updateTime() {
  
  // function to increment seconds and then increment minutes and hours if necessary.
  
  seconds++;  // Increment seconds
  if (seconds >= 60) {  // If seconds overflows (>=60)
    seconds = 0;         // Set seconds back to 0
    minutes++;           // Increment minutes
    if (minutes >= 60){   // If minutes overflows (>=60)
      minutes = 0;       // Set minutes back to 0
      hours++;           // Increment hours
      if (hours >= 12){  // If hours overflows (>=12)
        hours = 0;  // Set hours back to 0
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void drawArms(int h, int m, int s){

  // draw the clock's three arms: seconds, minutes, hours
  
  double midHours;  // this will be used to slightly adjust the hour hand
  static int hx, hy, mx, my, sx, sy;
  
  // adjust time to shift display 90 degrees ccw
  // this will turn the clock the same direction as text:
  h -= 3;
  m -= 15;
  s -= 15;
  if (h <= 0)
    h += 12;
  if (m < 0)
    m += 60;
  if (s < 0)
    s += 60;
  
  // calculate and draw new lines
  
  s = map(s, 0, 60, 0, 360);                                   // map the 0-60, to "360 degrees"
  sx = S_LENGTH * cos(PI * ((float)s) / 180);                  // woo trig!
  sy = S_LENGTH * sin(PI * ((float)s) / 180);                  // woo trig!
  
  oled.line(MIDDLE_X, MIDDLE_Y, MIDDLE_X + sx, MIDDLE_Y + sy); // draw the second hand
  
  m = map(m, 0, 60, 0, 360);                                   // map the 0-60, to "360 degrees"
  mx = M_LENGTH * cos(PI * ((float)m) / 180);                  // woo trig!
  my = M_LENGTH * sin(PI * ((float)m) / 180);                  // woo trig!
  
  oled.line(MIDDLE_X, MIDDLE_Y, MIDDLE_X + mx, MIDDLE_Y + my); // draw the minute hand
  
  midHours = minutes/12;                                       // midHours is used to set the hours hand to middling levels between whole hours
  h *= 5;                                                      // get hours and midhours to the same scale
  h += midHours;                                               // add hours and midhours
  h = map(h, 0, 60, 0, 360);                                   // map the 0-60, to "360 degrees"
  hx = H_LENGTH * cos(PI * ((float)h) / 180);                  // woo trig!
  hy = H_LENGTH * sin(PI * ((float)h) / 180);                  // woo trig!
  
  oled.line(MIDDLE_X, MIDDLE_Y, MIDDLE_X + hx, MIDDLE_Y + hy); // draw the hour hand
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void drawFace() {

  // draw an analog clock face
  
  oled.circle(MIDDLE_X, MIDDLE_Y, CLOCK_RADIUS); // draw the clock border
  
  // Draw the clock numbers
  oled.setFontType(0); // set font type 0, please see declaration in SFE_MicroOLED.cpp
  oled.setCursor(POS_12_X, POS_12_Y); // points cursor to x=27 y=0
  oled.print(12);
  oled.setCursor(POS_6_X, POS_6_Y);
  oled.print(6);
  oled.setCursor(POS_9_X, POS_9_Y);
  oled.print(9);
  oled.setCursor(POS_3_X, POS_3_Y);
  oled.print(3);
}
