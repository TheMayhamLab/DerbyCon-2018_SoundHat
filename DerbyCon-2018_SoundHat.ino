#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

// MSGEQ7 variables
int analogPin = 0; // read from multiplexer using analog input 0
int strobePin = 2; // strobe is attached to digital pin 2
int resetPin = 3; // reset is attached to digital pin 3
int spectrumValue[7]; // to hold a2d values
int leveler = 50;

long levelersum = 0;
int levelercount = 0;
int levelersamples = 23; // max is 300 or you'll overflow levelersum
int minleveler = 6;
int minlevelermed = 10;

int shiftreadings = -40;

//Hat Matrix Map
//Allows you to specify a pixel in the hat matrix using the Array format HatLED[X][Y] or HatLED[column][row]
//The value in each array cell represents the NeoPixel number/offset of the actual LED
//
//     column 0                                        column 6
//row 7 [0][7]  [1][7]  [2][7]  [3][7]  [4][7]  [5][7]  [6][7]
//      [0][6]  [1][6]  [2][6]  [3][6]  [4][6]  [5][6]  [6][6]
//      [0][5]  [1][5]  [2][5]  [3][5]  [4][5]  [5][5]  [6][5]
//      [0][4]  [1][4]  [2][4]  [3][4]  [4][4]  [5][4]  [6][4]
//      [0][3]  [1][3]  [2][3]  [3][3]  [4][3]  [5][3]  [6][3]
//      [0][2]  [1][2]  [2][2]  [3][2]  [4][2]  [5][2]  [6][2]
//      [0][1]  [1][1]  [2][1]  [3][1]  [4][1]  [5][1]  [6][1]
//row 0 [0][0]  [1][0]  [2][0]  [3][0]  [4][0]  [5][0]  [6][0]

int HatLED[7][8] = {
 {0, 13, 14, 27, 28, 41, 42, 55},
 {1, 12, 15, 26, 29, 40, 43, 54},
 {2, 11, 16, 25, 30, 39, 44, 53},
 {3, 10, 17, 24, 31, 38, 45, 52},
 {4,  9, 18, 23, 32, 37, 46, 51},
 {5,  8, 19, 22, 33, 36, 47, 50},
 {6,  7, 20, 21, 34, 35, 48, 49},
};

int waitcyclestocolorchange = 55;
int countcyclestocolorchange = 0;


int numcolors = 6;
int Colors[6][3] = {
  {255,10,10},
  {10,255,10},
  {10,10,255},
  {127,127,10},
  {10,127,127},
  {127,10,127}
};

int CurrentColor[7][3] = {
  {32,64,128},
  {255,10,10},
  {10,255,10},
  {10,10,255},
  {127,127,10},
  {10,127,127},
  {127,10,127}  
};

// LED Matrix variables
#define PIN 7
#define LED_COUNT 56
#define ctsPin 2
int brightness = 10; // 0 to 255
int tmpcolumnheight = 0;
int high = 0;
int colorR = 0;
int colorG = 0;
int colorB = 0;
int tmpcolumn = 0;
int tmprow = 0;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LED_COUNT, PIN, NEO_GRB + NEO_KHZ800);

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
  pinMode(12, INPUT_PULLUP); // Enable internal pull-up resistor on pin 12


  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code
  Serial.begin(9600);
  pinMode(analogPin, INPUT);
  pinMode(strobePin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  analogReference(DEFAULT);
  digitalWrite(resetPin, LOW);
  digitalWrite(strobePin, HIGH);
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}
void loop() {
  Spectrum1();
}

void Spectrum1()
{
  digitalWrite(resetPin, HIGH);
  digitalWrite(resetPin, LOW);

   for (int i = 0; i < 7; i++)
  {
    digitalWrite(strobePin, LOW);
    delayMicroseconds(30); // to allow the output to settle
    spectrumValue[i] = analogRead(analogPin) + shiftreadings;
    if (i > 3)
    {
      spectrumValue[i] = spectrumValue[i] - 10; // account for Treble too high typically
    }

    Level(i, spectrumValue[i]);
    Spectrum(i, spectrumValue[i]);
    ChangeColors();
    Serial.print(" ");
    Serial.print(spectrumValue[i]);
  
    digitalWrite(strobePin, HIGH);
  }
  btnBrightness(); // check if the button is pushed
  Serial.println();  
}

void ChangeColors()
{
  countcyclestocolorchange ++;
  if (countcyclestocolorchange > waitcyclestocolorchange)
  {
    int columntochange = random(7);
    int newcolor = random(numcolors);
    CurrentColor[columntochange][0] = Colors[newcolor][0];
    CurrentColor[columntochange][1] = Colors[newcolor][1];
    CurrentColor[columntochange][2] = Colors[newcolor][2]; 
    countcyclestocolorchange = 0;     
  } 
}

void Level (int i, int high)
{
  if ( (i == 5) || (i == 1) ) // Leveler Enabled
  {
    levelersum = levelersum + (high /10);
    levelercount++;
    if (levelercount > (2*levelersamples))
    {
      long leveleraverage = (levelersum / levelersamples * 10);
      leveler = leveleraverage / 5; // Higher number means more sensitive.
      int tempcurrentminleveler = 0;
      if (brightness < 50)
        {
          tempcurrentminleveler = minleveler;
        }
        else
        {
          tempcurrentminleveler = minlevelermed;
        }
      if (leveler < tempcurrentminleveler)
      {
          leveler = tempcurrentminleveler;
      }
      //Serial.println("");
      //Serial.print("Leveler Average: ");
      //Serial.println(leveleraverage);
      //Serial.print("Leveler: ");
      //Serial.println(leveler);
      levelersum = 0;
      levelercount = 0;
    }    
  }
}
void Spectrum(int i, int high)
{
  for (tmpcolumnheight = 0; tmpcolumnheight < 8; tmpcolumnheight++)
  {
    if ( (high > (tmpcolumnheight * leveler )) && ( high > 20) ) 
    {
      strip.setPixelColor(HatLED[i][tmpcolumnheight], CurrentColor[i][0], CurrentColor[i][1], CurrentColor[i][2]);
    }
    else
    {
      strip.setPixelColor(HatLED[i][tmpcolumnheight], 0, 0, 0);
    }
  }
  strip.setBrightness(brightness);
  strip.show();
}

void btnBrightness()
{
  if (digitalRead(12) == 0)
  {
    if (brightness > 28)
    {
      brightness = 10;
    }
    else
    {
      brightness = brightness + 10;     
    }
    Serial.println(brightness);
    delay(400);
  }
}
