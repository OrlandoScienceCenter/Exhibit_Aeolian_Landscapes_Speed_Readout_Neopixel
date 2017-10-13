#include <Adafruit_NeoPixel.h>
#include <CapacitiveSensor.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define neoPin 9
#define relPin 8
#define butPin 10
#define capSend 6
#define capRec 2
#define potPin A0
#define timeout 10000 // Change this to change how long it takes to go to attract mode
#define thresh = 1000  //Change this to adjust the capacitive button sensitivity

Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, neoPin, NEO_GRB + NEO_KHZ800);
CapacitiveSensor   capSense = CapacitiveSensor(capSend,capRec);
// Smoothing vars
const int numReadings = 100;
int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
unsigned long total = 0;                  // the running total
unsigned long potAverage = 0;                // the average

// Regular vars
unsigned long potVal = 0;
int redVal = 0;
int greenVal = 0;
int position1 = 0;
int rainCol = 0;

// Timeout vars
unsigned long timeUnchanged = 0;
byte lastVal = 0;

void setup() {
  pinMode(potPin, INPUT);
  pinMode(butPin, INPUT_PULLUP);
  pinMode(relPin, OUTPUT);
  digitalWrite(relPin,LOW);
  cs_4_2.set_CS_AutocaL_Millis(0xFFFFFFFF);
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

bool attractMode = 0;

void loop() {
  readPot();
 
  //Serial.println(timeUnchanged);

  if (potVal == lastVal) {timeUnchanged++;} else {timeUnchanged = 0;}
  lastVal = potVal;

  if (timeUnchanged > timeout){attractMode = 1;} else {attractMode = 0;}
  
if (!attractMode){
    for(uint16_t i = 0; i < potVal; i++) {
      // This inverts the green and red values so as the pot value gets higher, red gets higher. It makes the color fade work. Just trust me. I promise it does.
      greenVal = map(i * 18, 255, 0, 0, 255);
      redVal = i * 19; // Raise the number to make the red fade in faster. The below statement makes sure it will never overflow and wrap.
      if (redVal > 255){redVal = 255;}
      if (greenVal < 4){greenVal = 0;}
      
      position1 = map(i, 15, 0, 0, 15); // Reverse stuff because neopixel orders their rings counterclockwise. Wtf?
      
      strip.setPixelColor(position1, strip.Color(redVal, greenVal, 0));
    }
  
    // This part clears the pixels above the written ones to black.
    for(uint16_t i = position1 - 1; i < position1; i--) {
      strip.setPixelColor(i, 0);
    }
    
    strip.show();
  }

  if (attractMode){
    rainbow(15);
  }
  long capVal = capSense.capacitiveSensor(30);
  digitalWrite(relPin,(capVal > thresh) || !digitalRead(butPin));
  delay(1);
}

void rainbow(uint8_t wait) {
  uint16_t rainPos, j;

    for(rainPos=0; rainPos<strip.numPixels(); rainPos++) {
      strip.setPixelColor(rainPos, Wheel((rainCol) & 255));
    }
    strip.show();

    rainCol++;
    if (rainCol > 255) {rainCol = 0;}

    readPot();
    
    delay(wait);
  }

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void readPot(){
  // All of this block of code is from the arduino pot smoothing example.
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = analogRead(potPin);
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;
  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }
  // calculate the average:
  potAverage = total / numReadings;
  // send it to the computer as ASCII digits
  
  potVal = map(potAverage, 890, 0, 1, 15);
}
