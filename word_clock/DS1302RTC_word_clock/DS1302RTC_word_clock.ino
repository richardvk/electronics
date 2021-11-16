#include <ThreeWire.h>  
#include <RtcDS1302.h>   // https://github.com/Makuna/Rtc/wiki
#define FASTLED_INTERNAL // (thanks to https://www.reddit.com/r/FastLED/comments/kn5vjn/pragma_message_fastled_version_3004000/)
#include <FastLED.h>
#include <LEDMatrix.h>

// Set up the DS1302 clock pins

//https://github.com/FastLED/FastLED/wiki/SPI-Hardware-or-Bit-banging
// note "Strong" (square) nodemcu can upload at max 230400 (usefull!!)`
//ThreeWire myWire(2,14,0); // (D4,D5,D3) IO/Data, SCLK/CLK, CE/Reset (Use these for 'Strong' (square) ESP8266 board) bitbanged
ThreeWire myWire(14,12,13); // (D5,D6,D7) IO/Data, SCLK/CLK, CE/Reset (Use these for 'Strong' (square) ESP8266 board) SPI!

//ThreeWire myWire(11,12,10); // IO, SCLK, CE (Use these for Raspberry Pi Pico) // FastLED DOESNT WORK WITH PICO!
//ThreeWire myWire(D4,D5,D3); // IO, SCLK, CE (USE THESE PINS FOR WEMOS DI R2 board!)
//ThreeWire myWire(4,5,2); // IO/Data, SCLK/CLK, CE/Reset (Use these for Arduino/Funduino Uno board)
RtcDS1302<ThreeWire> Rtc(myWire);

// Set up the LED panels (this is specific to the Glowbit currently)
//#define LED_PIN        D6  // WeMos D1 R2
//#define LED_PIN        5   // WeMos D1 R2
//#define LED_PIN        5   // (D1) (Use this for 'Strong' (square) ESP8266 board)
#define LED_PIN      15    // (D8) (Use this for 'Strong' (square) ESP8266 board)

//FastLED settings
#define COLOR_ORDER    GRB
#define CHIPSET        WS2812B

// Glowbit specific settings
#define MATRIX_TYPE   HORIZONTAL_MATRIX
#define BOARD_WIDTH   8 // boards are square so 'width' is both x width and y width
#define NUM_BOARDS    2
#define NUM_COLOURS   6
#define NORMAL_BRIGHTNESS 40
#define MAX_BRIGHTNESS    70

// Set up the matrix
cLEDMatrix<BOARD_WIDTH*NUM_BOARDS, BOARD_WIDTH, MATRIX_TYPE> leds;

// Set up some static colour related data
static const CRGB CRGBColours [NUM_COLOURS] =
{
  CRGB::Yellow,
  CRGB::Cyan,
  CRGB::Red,
  CRGB::Green,
  CRGB::Orange,
  CRGB::White
};

int fixed_colour = 6;

static int SelectedColourIndexes [NUM_COLOURS] = {99,99,99,99,99,99};

// Some static data for the word clock
String hour_str[]={"twelve","one","two","three","four","five","six","seven","eight","nine","ten","eleven","twelve","one","two","three","four","five","six","seven","eight","nine","ten","eleven","twelve"};
//            
//     01234567       
// 0   Its just   64  .gone...
// 8   almost t   72  entwenty
// 16  quarterh   80  alf past
// 24  to midda   88  y onetwo 
// 32  midnight   96  four six
// 40  three se   104 ven nine
// 48  fiveeigh   112 t eleven
// 56  tentwelv   120 e oclock 

// define the led's that illuminate for each word
int its[]      = {0,1,2};
int just[]     = {4,5,6,7};
int gone[]     = {65,66,67,68};
int almost[]   = {8,9,10,11,12,13};
int ten1[]     = {15,72,73};
int twenty[]   = {74,75,76,77,78,79};
int quarter[]  = {16,17,18,19,20,21,22};
int half[]     = {23,80,81,82};
int past[]     = {84,85,86,87};
int to[]       = {24,25};
int midday[]   = {27,28,29,30,31,88};
int midnight[] = {32,33,34,35,36,37,38,39};
int one[]      = {90,91,92};
int two[]      = {93,94,95};
int three[]    = {40,41,42,43,44};
int four[]     = {96,97,98,99};
int five[]     = {48,49,50,51};
int six[]      = {101,102,103};
int seven[]    = {46,47,104,105,106};
int eight[]    = {52,53,54,55,112};
int nine[]     = {108,109,110,111};
int ten[]      = {56,57,58};
int eleven[]   = {114,115,116,117,118,119};
int twelve[]   = {59,60,61,62,63,120};
int oclock[]   = {122,123,124,125,126,127};
int error[]    = {21,22}; // used to show 'ER' if there is an error

int digit_one[]   = {9,10,18,26,34,41,42,43};                // 8 digits
int digit_two[]   = {9,10,11,19,25,26,27,33,41,42,43};       // 11 digits
int digit_three[] = {9,10,11,19,25,26,27,35,41,42,43};       // 11 digits
int digit_four[]  = {9,11,17,19,25,26,27,35,43};             // 9 digits
int digit_five[]  = {9,10,11,17,25,26,27,35,41,42,43};       // 11 digits
int digit_six[]   = {9,10,11,17,25,26,27,33,35,41,42,43};    // 12 digits
int digit_seven[] = {9,10,11,19,27,35,43};                   // 7 digits
int digit_eight[] = {9,10,11,17,19,25,26,27,33,35,41,42,43}; // 13 digits
int digit_nine[]  = {9,10,11,17,19,25,26,27,35,41,42,43};    // 12 digits
int digit_zero[]  = {9,10,11,17,19,25,27,33,35,41,42,43};    // 12 digits
int digit_colon[] = {80,96};                                 // 2 digits

int pixel_num, num_pixels;
int random_r, random_g, random_b, random_x, random_y;

// some variables to help track our 'millis' counter so we dont have to use 'delay()'
int display_refresh_period = 60000;
unsigned long millis_counter = 0;

// Buttons
const int minUpPin = 5; // D1
const int minDownPin = 4; // D2
int minUpPinState = 0;
int minDownPinState = 0;
unsigned long minUp_millis_counter = 0;
unsigned long minDown_millis_counter = 0;
int button_delay_period = 500;

int temp_cnt = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////

void setup () 
{
    Serial.begin(19200);

    // Set up the clock
    set_up_ds1302();

    // Set up the LEDs
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds[0], leds.Size());
    FastLED.setBrightness(NORMAL_BRIGHTNESS);

    // Sert up buttons
    pinMode(minUpPin,   INPUT);
    pinMode(minDownPin, INPUT);
}

void loop () 
{

  // do other stuff here, like checking button presses
  // read the state of the pushbutton value:
  minUpPinState   = digitalRead(minUpPin);
  minDownPinState = digitalRead(minDownPin);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (minUpPinState == LOW && millis() >= minUp_millis_counter) {
    RtcDateTime now = Rtc.GetDateTime();
    now += 60; // add a minute
    Rtc.SetDateTime(now);

    FastLED.setBrightness(MAX_BRIGHTNESS);
    display_digital_clock(now);
    FastLED.setBrightness(NORMAL_BRIGHTNESS);

    minUp_millis_counter = millis() + button_delay_period;
    millis_counter = millis()-display_refresh_period+2000;
  }

  if (minDownPinState == LOW && millis() >= minDown_millis_counter) {
    RtcDateTime now = Rtc.GetDateTime();
    now -= 60; // subtract a minute
    Rtc.SetDateTime(now);

    FastLED.setBrightness(MAX_BRIGHTNESS);
    display_digital_clock(now);
    FastLED.setBrightness(NORMAL_BRIGHTNESS);

    minDown_millis_counter = millis() + button_delay_period;
    millis_counter = millis()-display_refresh_period+2000;
  }

  if(millis_counter == 0 || millis() >= millis_counter + display_refresh_period){

    // Get the current time off the RTC
    RtcDateTime now = Rtc.GetDateTime();

    // This sneakiness allows our 'delay' to fall almost perfectly on the '00' second, as the minute rolls over.
    // Really its trying to account for any drift that happens if we do a refresh on 60sec + time_it_takes_to_do_all_the_display_work
    int sc = now.Second();
    millis_counter = millis_counter + display_refresh_period - ((sc-1) * 1000);

    if (!now.IsValid())
    {
        // the battery on the device is low or even missing or the power line is disconnected
        Serial.println("Cannot retrieve a valid time from RTC!");
        show_error();
        return;
    }

    display_word_clock(now);
  } // end millis check for clock display refresh
}

void display_digital_clock(RtcDateTime now){
  
  // work out what digits to show
  int hr = now.Hour();
  int mn = now.Minute();

  FastLED.clearData();
  delay(10);
  FastLED.clear(false);
  delay(10);

  if (hr >= 10) {
    display_digit(1,0);
    display_digit(hr-10,1);
  }
  else {
    display_digit(0,0);
    display_digit(hr,1);
  }

  //display_colon();

  if (mn >= 10) {
    display_digit(mn/10,2);
    display_digit(mn%10,3);
  }
  else {
    display_digit(0,2);
    display_digit(mn%10,3);
  }

  delay(10);
  FastLED.show();
  delay(10);
}

void display_digit(int digit, int offset){

  CRGB colour = CRGB::White;

  int led_offset = 0; // the number we must add to the led number to shift the lit led across to the right
  if (offset == 1){
    led_offset = 4;
  }
  else if (offset == 2){
    led_offset = 64;
  }
  else if (offset == 3){
    led_offset = 68;
  }

  if (digit == 0) {
    for (int a=0; a<12; a++)
        leds(digit_zero[a]+led_offset) = colour;
  }
  else if (digit == 1) {
    for (int a=0; a<8; a++)
        leds(digit_one[a]+led_offset) = colour;
  }
  else if (digit == 2) {
    for (int a=0; a<11; a++)
        leds(digit_two[a]+led_offset) = colour;
  }
  else if (digit == 3) {
    for (int a=0; a<11; a++)
        leds(digit_three[a]+led_offset) = colour;
  }
  else if (digit == 4) {
    for (int a=0; a<9; a++)
        leds(digit_four[a]+led_offset) = colour;
  }
  else if (digit == 5) {
    for (int a=0; a<11; a++)
        leds(digit_five[a]+led_offset) = colour;
  }
  else if (digit == 6) {
    for (int a=0; a<12; a++)
        leds(digit_six[a]+led_offset) = colour;
  }
  else if (digit == 7) {
    for (int a=0; a<7; a++)
        leds(digit_seven[a]+led_offset) = colour;
  }
  else if (digit == 8) {
    for (int a=0; a<13; a++)
        leds(digit_eight[a]+led_offset) = colour;
  }
  else if (digit == 9) {
    for (int a=0; a<12; a++)
        leds(digit_nine[a]+led_offset) = colour;
  }

} // end display_digit

void display_colon(){

  CRGB colour = CRGB::White;

  for (int a=0; a<2; a++)
        leds(digit_colon[a]) = colour;
} // end display_colon

void display_word_clock(RtcDateTime now) {

    // Clear the LEDs
    FastLED.clear(true);
    
    int hr = now.Hour();
    int mn = now.Minute();
    int sc = now.Second();
    String w1, w2, w3, w4, w5;
    String min_str = mn < 10 ? "0"+mn : ""+mn;
    String hr_str  = hr < 10 ? "0"+hr : ""+hr;

    // fill the colour array with our predetermined colours but each time in a different colour order
    SetCRGBColours();

    int colourIndex=0;

    // Word zero (always "It's")

    CRGB random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
    for (int a=0; a<3; a++){
        leds(its[a]) = random_colour;
    }
    colourIndex++;
    
    // Word one
    if ((mn>=1 && mn<=5) || (mn>=11 && mn<=12) || (mn>=16 && mn<=17) || (mn>=21 && mn<=25) || (mn>=31 && mn<=35) || (mn>=41 && mn<=42) || (mn>=46 && mn<=47) || (mn>=51 && mn<=54)){
      w1 = "just gone ";

      random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
      colourIndex++;
      for (int i=0; i<4; i++)
        leds(just[i]) = random_colour;

      random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
      for (int j=0; j<4; j++)
        leds(gone[j]) = random_colour;  
      colourIndex++;
    }
    else if ((mn>=6 && mn<=9) || (mn>=11 && mn<=14) || (mn>=18 && mn<=19) || (mn>=26 && mn<=29)  || (mn>=36 && mn<=39) || (mn>=43 && mn<=44) || (mn>=48 && mn<=49) || (mn>=55 && mn<=59)){
      w1 = "almost ";

      random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
      for (int i=0; i<6; i++)
        leds(almost[i]) = random_colour;
      colourIndex++;
    }

    // Word two
    // this 'word' is not always shown, so only increase the colour index if it is shown
    if ((mn>=13 && mn<=17) || (mn>=43 && mn<=47)){
      w2 = "quarter ";
      random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
      for (int i=0; i<7; i++)
        leds(quarter[i]) = random_colour;
      colourIndex++;
    }
    else if (mn>=26 && mn<=35){
      w2 = "half ";
      random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
      for (int i=0; i<4; i++)
        leds(half[i]) = random_colour;
      colourIndex++;
    }
    else if ((mn>=6 && mn<=12) || (mn>=48 && mn<=54)){
      w2 = "ten ";
      random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
      for (int i=0; i<3; i++)
        leds(ten1[i]) = random_colour;
      colourIndex++;
    }
    else if ((mn>=18 && mn<=25) || (mn>=36 && mn<=42)){
      w2 = "twenty ";
      random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
      for (int i=0; i<6; i++)
        leds(twenty[i]) = random_colour;
      colourIndex++;
    }

    // Word three
    if ((mn>=6 && mn<=35)) {
      w3 = "past ";
      random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
      for (int i=0; i<4; i++)
        leds(past[i]) = random_colour;
      colourIndex++;
    }
    else if ((mn>=36 && mn<=54)) {
      w3 = "to ";
      random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
      for (int i=0; i<2; i++)
        leds(to[i]) = random_colour;
      colourIndex++;
    }

    // Word for the hour
    int word_hr = hr;

    if (mn>=36 && mn<=59) {
      word_hr = hr+1;
    }

    word_hr = word_hr<=12 ? word_hr : (word_hr==24 ? 0 : word_hr-12);

    // Word four
    if (word_hr == 0 && (mn>=55 || mn<=5)) {
      w4 = "midnight ";

      random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
      for (int i=0; i<8; i++)
        leds(midnight[i]) = random_colour;
    }
    else if (word_hr == 12 && (mn>=55 || mn<=5)) {
      w4 = "midday ";
      random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
      for (int i=0; i<6; i++)
        leds(midday[i]) = random_colour;
    }
    else {
      w4 = hour_str[word_hr] + " ";

      if (word_hr == 1 || word_hr == 13) {
        random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
        for (int i=0; i<3; i++)
          leds(one[i]) = random_colour;
      }
      else if (word_hr == 2 || word_hr == 14) {
        random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
        for (int i=0; i<3; i++)
          leds(two[i]) = random_colour;
      }
      else if (word_hr == 3 || word_hr == 15) {
        random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
        for (int i=0; i<5; i++)
          leds(three[i]) = random_colour;
      }
      else if (word_hr == 4 || word_hr == 16) {
        random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
        for (int i=0; i<4; i++)
          leds(four[i]) = random_colour;
      }
      else if (word_hr == 5 || word_hr == 17) {
        random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
        for (int i=0; i<4; i++)
          leds(five[i]) = random_colour;
      }
      else if (word_hr == 6 || word_hr == 18) {
        random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
        for (int i=0; i<3; i++)
          leds(six[i]) = random_colour;
      }
      else if (word_hr == 7 || word_hr == 19) {
        random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
        for (int i=0; i<5; i++)
          leds(seven[i]) = random_colour;
      }
      else if (word_hr == 8 || word_hr == 20) {
        random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
        for (int i=0; i<5; i++)
          leds(eight[i]) = random_colour;
      }
      else if (word_hr == 9 || word_hr == 21) {
        random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
        for (int i=0; i<4; i++)
          leds(nine[i]) = random_colour;
      }
      else if (word_hr == 10 || word_hr == 22) {
        random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
        for (int i=0; i<3; i++)
          leds(ten[i]) = random_colour;
      }
      else if (word_hr == 11 || word_hr == 23) {
        random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
        for (int i=0; i<6; i++)
          leds(eleven[i]) = random_colour;
      }
      else if (word_hr == 0 || word_hr == 12 || word_hr == 24) {
        random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
        for (int i=0; i<6; i++)
          leds(twelve[i]) = random_colour;
      }
    }
    colourIndex++;

    // Word five
    if (((mn>=0 && mn<=5) || (mn>=55 && mn<=59)) && word_hr != 0 && word_hr != 12) {
      w5 = "o'clock";
      random_colour = CRGBColours[SelectedColourIndexes[colourIndex]];
      for (int i=0; i<6; i++)
        leds(oclock[i]) = random_colour;
    }

    String sentence = "It's " + w1 + w2 + w3 + w4 + w5;
    
    printDateTime(now);
    Serial.print(": ");
    Serial.println(sentence);

    FastLED.show();
    
} // end display_word_clock

#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            //PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            PSTR("%02u:%02u:%02u"),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    Serial.print(datestring);
}

CRGB get_random_colour() 
{
  CRGB random_colour;
  random_colour = CRGBColours[random(NUM_COLOURS)];
  return random_colour;
}

void SetCRGBColours() 
{

  if (fixed_colour > 0){
    for (int i=0; i<NUM_COLOURS; i++) {
      SelectedColourIndexes[i] = fixed_colour-1;
    }

    fixed_colour++;
    if (fixed_colour>6)
      fixed_colour=1;

    return;

  }

  int fillIndex = 0;

  while (fillIndex<NUM_COLOURS) { // we still have unfilled colour slots

    bool Filled = false;

    // The first one we can always just fill with whatever, there is nothing to clash with yet

    if (fillIndex == 0) {
      //select a random index
      int RandomIndex = random(NUM_COLOURS);
        
      SelectedColourIndexes[fillIndex] = RandomIndex;
      fillIndex++;
      continue;
    }
  
    while (!Filled) {

      bool Found = false;
      
      //select a random index
      int RandomIndex = random(NUM_COLOURS);
      
      // Run through the array of colours we have filled so far and make sure we havent already picked this one
      // If we have, loop again...

      for (int i=0; i<fillIndex; i++) {

        if (RandomIndex == SelectedColourIndexes[i]) {
          Found = true; // its in the array already, we cant use it, so we need to go through the 'while' one more time...
          break;
        } // end if
           
      } // end for

      // We have gone through all the currently filled indexes.
      // If Found is still false it means the RandomIndex we chose _is_ available - we should add this value to this index of SelectedColourIndexes
      if (!Found) {
        SelectedColourIndexes[fillIndex] = RandomIndex;
        fillIndex++;
        Filled = true;
      } // end if
        
    } // end while (!Filled)

    Filled = false; // reset for the next round
      
  } // end while (fillIndex<NUM_COLOURS) 

} // end SetCRGBColours

void set_up_ds1302() {

    // Setup for the clock, try get the time off the chip
    Serial.print("compiled: "); 
    Serial.print(__DATE__);
    Serial.println(__TIME__);

    Rtc.Begin();

    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);
    Serial.println();

    if (!Rtc.IsDateTimeValid())
    {
        // Common Causes:
        //    1) first time you ran and the device wasn't running yet
        //    2) the battery on the device is low or even missing

        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(compiled);
    }

    if (Rtc.GetIsWriteProtected())
    {
        Serial.println("RTC was write protected, enabling writing now");
        Rtc.SetIsWriteProtected(false);
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled)
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled)
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled)
    {
        Serial.println("RTC is the same as compile time! (fine assuming the board was just flashed!)");
    }

} // end set_up_ds1302

void show_error() {

  CRGB colour = CRGB::Red;

  for (int i=0; i<2; i++)
    leds(error[i]) = colour;
  delay(500);
  FastLED.show();
  delay(1000);

}
