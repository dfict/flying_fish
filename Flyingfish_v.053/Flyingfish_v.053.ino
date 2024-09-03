// Karplus-Strong implemented for solar sounder, with temperature sensor.
// DFISHKIN, 2021—2024. Thanks electro-music.com forums for inspiration!
// thanks Lee Tusman and Fame Tothong and Keng for joining the initial crew of beta testers.

#include <EEPROM.h>

#define SAMPLE_RATE 22050 // adjust sample rate to lower pitch, 
                         //11025 for octave lower for example, 22050 normal
#define SIZE        256
#define OFFSET      32
#define LED_PIN     13
#define LED_PORT    PORTB
#define LED_BIT     5
#define SPEAKER_PIN 11
#define BUTTON_PIN 2

int out;
int last = 0;
int curr = 0;
uint8_t delaymem[SIZE];
uint8_t locat = 0;
uint8_t bound = SIZE; //bound is the period of the delayline. 
                      //it's important for pitch and timing routines later.

int accum = 0;
int lowpass = 0.1; // 0 ... 1.0
bool trig = false;

volatile int tempSensor = 0;
volatile int solarPanel = 0;
volatile int analogPot = 0;

int mappedTemp = 0;
int mappedSolar = 0;
int mappedPot = 0;  
float floatPot = 0.0; // always need decimal places in floats.
     //this seems to work now.

int mode = 1; // Variable to track the current program mode
int val[6];

// note root -- needs to be under the size of the delayline window, can't be less than 256.  
float beep = 255.0;

/////////// tuning in just intonation. based on LMY's WTP 3/7 prism—
// i added 11 limit notes to the original prism 3 and 7
float note1 = beep / (1.0/1.0);
float note2 = beep / (33.0/32.0);
float note3 = beep / (9.0/8.0);
float note4 = beep / (147.0/128.0);
float note5 = beep / (77.0/64.0);
float note6 = beep / (21.0/16.0);
float note7 = beep / (11.0/8.0);
float note8 = beep / (3.0/2.0);
float note9 = beep / (49.0/32.0);
float note10 = beep / (441.0/256.0);
float note11 = beep / (7.0/4.0);
float note12 = beep / (63.0/32.0);

float note13 = beep / (2.0/1.0);
float note14 = beep / (33.0/16.0);
float note15 = beep / (9.0/4.0);
float note16 = beep / (147.0/64.0);
float note17 = beep / (77.0/32.0);
float note18 = beep / (21.0/8.0);
float note19 = beep / (11.0/4.0);
float note20 = beep / (3.0/1.0);
float note21 = beep / (49.0/16.0);
float note22 = beep / (441.0/128.0);
float note23 = beep / (7.0/2.0);
float note24 = beep / (63.0/16.0);


float noteTable[24] = {
  note1, note2, note3, note4, note5, note6, note7, note8, 
  note9, note10, note11, note12,  note13, note14, note15, note16, 
  note17, note18, note19, note20, note21, note22, note23, note24 
};

// Added Southeast Asian tuning freq from out research
// written by elekhlekha (kengchakaj & Nitcha fame)
///////////////////////////////////////////////////
// Ranat Ek # 1-8 
float bar1 = beep / (1.0/1.0);
float bar2 = beep / (561.5/508.5);
float bar3 = beep / (619.9/508.5);
float bar4 = beep / (684.4/508.5);
float bar5 = beep / (755.7/508.5);
float bar6 = beep / (834.4/508.5);
float bar7 = beep / (921.2/508.5);
float bar8 = beep / (1017.1/508.5);
float bar9 = beep / (2.0/1.0);
float bar10 = beep / (561.5/254.25);
float bar11 = beep / (619.9/254.25);
float bar12 = beep / (684.4/254.25);
float bar13 = beep / (755.7/254.25);
float bar14 = beep / (834.4/254.25);
float bar15 = beep / (921.2/254.25);
float bar16 = beep / (1017.1/254.25);
float bar17 = beep / (4.0/1.0);
float bar18 = beep / (561.5/127.125);
float bar19 = beep / (619.9/127.125);
float bar20 = beep / (684.4/127.125);
float bar21 = beep / (755.7/127.125);
float bar22 = beep / (834.4/127.125);
float bar23 = beep / (921.2/127.125);
float bar24 = beep / (1017.1/127.125);

float ranatTable[24] = {
  bar1, bar2, bar3, bar4, bar5, bar6, bar7, bar8, 
  bar9, bar10, bar11, bar12,  bar13, bar14, bar15, bar16, 
  bar17, bar18, bar19, bar20, bar21, bar22, bar23, bar24 
};
///////////////////////////////////////////////////
// Punong sound #1-8 SoundCultures from Cambodia
float punong1 = beep / (1.0/1.0);
float punong2 = beep / (411.5/369.0);
float punong3 = beep / (458/369.0);
float punong4 = beep / (493.6/369.0);
float punong5 = beep / (554.4/369.0);
float punong6 = beep / (682.0/369.0);
float punong7 = beep / (740.0/369.0);
float punong8 = beep / (823.0/369.0);
float punong9 = beep / (2.0/1.0);
float punong10 = beep / (411.5/184.5);
float punong11 = beep / (458/184.5);
float punong12 = beep / (493.6/184.5);
float punong13 = beep / (554.4/184.5);
float punong14 = beep / (682.0/184.5);
float punong15 = beep / (740.0/184.5);
float punong16 = beep / (823.0/184.5);
float punong17 = beep / (4.0/1.0);
float punong18 = beep / (411.5/92.25);
float punong19 = beep / (458/92.25);
float punong20 = beep / (493.6/92.25);
float punong21 = beep / (554.4/92.25);
float punong22 = beep / (682.0/92.25);
float punong23 = beep / (740.0/92.25);
float punong24 = beep / (823.0/92.25);

//184.5 // 92.25 

float punongTable[24] = {
  punong1, punong2, punong3, punong4, punong5, punong6, punong7, punong8, 
  punong9, punong10, punong11, punong12,  punong13, punong14, punong15, punong16, 
  punong17, punong18, punong19, punong20, punong21, punong22, punong23, punong24 
};
///////////////////////////////////////////////////

//random weights function is kind of boring sounding to me, one year later.
//
int weights1[24] = {20, 1, 1, 1, 1, 1, 1, 20, 1, 1, 20, 1, 20, 1, 1, 1, 1, 1, 1, 20, 1, 1, 20, 1};
/*by increasing numbers for each index in the weighted array, you make an item more likely to be chosen. 
zero for no choices, all 1s for regular random algo*/

int weights2[24] = {1, 40, 1, 1, 1, 1, 40, 1, 1, 1, 1, 1, 1, 40, 1, 1, 1, 1, 40, 1, 1, 1, 1, 1};
//try to emphasize 7

int weightsRanat[8] = {30, 30, 30, 1, 30, 30, 1, 30};

int weightsPunong[8] = {20, 1, 40, 40, 40, 30, 40, 1 };

int randomweights[24] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};


//karplus strong implementation
ISR(TIMER1_COMPA_vect) {
   
  OCR2A = 0xff & out; //send output to timer A, how sound leaves the pin!

  if (trig) { //if the karplus gets triggered
    
    for (int i = 0; i < SIZE; i++) delaymem[i] = random(); //fill the delay line with random numbers
    
    trig = false; //turn off the trigger
    
  } else { 
    
    delaymem[locat++] = out;        //the input to the delay line the output of the previous call of this function
    if (locat >= bound) locat = 0;  //this makes the delay line wrap around. bound is the effective length of the line.
                                   //if you surpass the bound, move pointer back to zero.
                            
    curr = delaymem[locat]; //get the new output of the delay line.
    
    out = accum >> lowpass; //divide the previous output by two
                            //controls the dampening factor of the string, reduces the energy by lopass
    
    //accum = accum - out + ((last>>1) + (curr>>1)); //1 pole IR filter, a smoothing function, 
                                                   //averages the current output and the previous output

    accum = accum - out + ((last>>1) + (curr>>1)); 

    last = curr;
    
  }
}

void startPlayback()
{
    pinMode(SPEAKER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT); 
   // Set up Timer 2 to do beep width modulation on the speaker pin.
    
    ASSR &= ~(_BV(EXCLK) | _BV(AS2));   // Use internal clock (datasheet p.160)
    TCCR2A |= _BV(WGM21) | _BV(WGM20);
    TCCR2B &= ~_BV(WGM22);  // Set fast PWM mode  (p.157)

    TCCR2A = (TCCR2A | _BV(COM2A1)) & ~_BV(COM2A0);
    TCCR2A &= ~(_BV(COM2B1) | _BV(COM2B0));    
    // Do non-inverting PWM on pin OC2A (p.155)-- pin 11.
    
    TCCR2B = (TCCR2B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);   // No prescaler (p.158)
    OCR2A = 0;   // Set initial beep width to zero/
    cli();   // Set up Timer 1 to send a sample every interrupt. or, clear interrupts
    
    TCCR1B = (TCCR1B & ~_BV(WGM13)) | _BV(WGM12);
    TCCR1A = TCCR1A & ~(_BV(WGM11) | _BV(WGM10));
    // Set CTC mode (Clear Timer on Compare Match) (p.133)
    // Have to set OCR1A *after*, otherwise it gets reset to 0!
    
    TCCR1B = (TCCR1B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);
    // No prescaler (p.134)
    
    OCR1A = F_CPU / SAMPLE_RATE;
 // Set the compare register (OCR1A).
    // OCR1A is a 16-bit register, so we have to do this with
    // interrupts disabled to be safe. 
    // 16e6 / x = y

    TIMSK1 |= _BV(OCIE1A);
   // Enable interrupt when TCNT1 == OCR1A (p.136)
   
    sei();
}

void stopPlayback()
{
    TIMSK1 &= ~_BV(OCIE1A);  // Disable playback per-sample interrupt.
    TCCR1B &= ~_BV(CS10);    // Disable the per-sample timer completely.
    TCCR2B &= ~_BV(CS10);    // Disable the PWM timer.
    digitalWrite(SPEAKER_PIN, LOW);
}



void setup() {
  startPlayback();
   Serial.begin(9600);
  randomSeed(analogRead(4));       //was 0. trying another analog pin. move to the regular void loop?
    pinMode(BUTTON_PIN, INPUT_PULLUP); // Configure the button pin as input with pull-up resistor
    
  if(EEPROM.read(0)==255){ // when first uploading program, eeprom is empty (aka, 255)
    mode=1;           //so we need to check that and write a default state
  } else {
  mode = EEPROM.read(0);
  }
}

void updateSensorReadings() 
{
  tempSensor = analogRead(0);
  /* define where the temperature sensor is. 
  Use 3 pin TMP36GT9Z from analog devices! easy.
  Note that for 105F° the ADC is approx 185.
  and for 15° the ADC is approx 83
  roughly one degree F per bit but not exactly.  */
  solarPanel = analogRead(1);
    /* define where the wobbly voltage is. 
    this is just a voltage divider hooked up to the solar panel, 
   should vary from 5v to 0v depending on sun conditions. 
  can be used later. 
   value reliably ferret between 600 and 900 with 100k and 81k resistor*/
  analogPot = analogRead(2); // analog pot for testing.
  tempSensor = constrain(tempSensor, 83, 185);
  mappedTemp = map(tempSensor, 83, 185, 200, 400);
  solarPanel = constrain(solarPanel, 630, 950);
  mappedSolar = map(solarPanel, 630, 950, 200, 400);    
  mappedPot = map(analogPot, 0, 1023, 0, 500);  
  floatPot = mappedPot / 100.0; // always need decimal places in floats.
     //this seems to work now.


}
                                  
float fade(float t) {
  return t * t * t * (t * (t * 6 - 15) + 10);
}

float lerp(float a, float b, float t) {
  return a + t * (b - a);
}

float grad(int hash, float x) {
  int h = hash & 15;
  float grad = 1.0 + (h & 7);
  if (h & 8) grad = -grad;
  return (grad * x);
}

float perlinNoise(float x) {
  int X = (int)x & 255;
  x -= (int)x;
  float u = fade(x);
  int a = (X) & 255;
  int b = (X + 1) & 255;
  return lerp(grad(a, x), grad(b, x - 1), u);
}


/*WEIGHTED RANDOM FUNCTION
 REPLACES REGULAR ARDUINO RANDOM WITH MORE OCCURRANCES
 Just adjust the weight and width the table correctly*/
int weightedRandom(int* weights, int width) {
  int totalWeight = 0;
  for (int i = 0; i < width; i++) {
    totalWeight += weights[i];
  }
  
  int randomValue = random(totalWeight); // is this truly random?
  int cumulativeWeight = 0;
  for (int i = 0; i < width; i++) {
    cumulativeWeight += weights[i];
    if (randomValue < cumulativeWeight) {
      return i;
    }
  }
  return width - 1;
}

//weights array referencing the likelihood of a note in the 24 note scale 
// noteTable is the scale
// phrase end is the width of the forloop (rhythm phrasing)
//divisor is the lowest note of the scale. divide by 2 or 4 to change the octave, etc.

void FlyingFlounder(int* weights, float* noteTable, int phraseBegin, int phraseEnd, float divisor) //need to add value for the start value
{
  int j = 1;
  int phraseStop = phraseBegin - 1;
  for (int i = phraseBegin; i > phraseStop; i = i + j)  // Wraps entire rhythm structure in this loop.
  {  
    if (i == phraseEnd) 
          { // Catch the edge of the timer! this number 30 could be resized.
          j = -1;     // Switch direction at the peak.
          }


    LED_PORT ^= 1 << LED_BIT; // Toggles an LED for debug purposes.
    trig = true;              // True at the trigger fires a Karplus grain.

    // Note Choice
    int tab = weightedRandom(weights, 24); // Critical note choice line of code. 
                                            // '24' could become 'Weightsize'
    
    // Bound = noteTable[tab] + thermal; // Put a little thermal variation on the pitch.
    bound = noteTable[tab]; // No thermal variation on the pitch.
    bound = bound / divisor; //choose where to put the tonic.

    // LOPASS FILTERING
    int value = random(0, 1023); // Put a random articulator variable onto the lopass input.
    float falue = map(value, 0, 1023, 1, 3000) / 1000.0; // Shift the value to between 0.0 and 3.0.
    lowpass = falue; // Route that value to the lopass variable.

    // RHYTHMIC ARTICULATION
    int bond = bound * i; // Takes the bound, multiplies it by the expanding variable, will go to "phraseEnd"!
    int vari = i * 3;
    bond = bond + vari;
    bond = bond / 5;
    delay(bond);

    for(int q = 0; q < 3; q++) { //this loop prints the three analog voltages in 10 bit binary to the Serial 
      val[q] = analogRead(q); //comment this out if you need to!
      Serial.print(val[q]);
      Serial.print(" ");
    }
    Serial.println(); //need to carriage return of course
    
    // Check for button press and exit loop if pressed
    if (digitalRead(BUTTON_PIN) == LOW) {
      Serial.println(F("Button pressed. Exiting FlyingFlounder();"));
      break;
    }
  }
  Serial.println(F("Executing FlyingFlounder();")); 
}

void FlyingMarlin(float* noteTable, int phraseBegin, int phraseEnd, float divisor, float mult, float add) 
{
  int j = 1;
  int phraseStop = phraseBegin - 1;
  for (int i = phraseBegin; i > phraseStop; i = i + j) 
  {
    if (i == phraseEnd) 
    {
      j = -1;
    }

    updateSensorReadings();
    LED_PORT ^= 1 << LED_BIT;
    trig = true;
  
    // Use i * mult + add for the Perlin noise calculation
    float noiseValue = (perlinNoise((i * (mult + 0.0) ) + floatPot) + 1.0) / 2.0;
// i * (mult + floatPot) + add?
 //or  i * (mult ) + floatPot?
          // float noiseValue = (perlinNoise(i * 0.4) + 1.0) / 2.0; 
          // i / 20.0 still works but changes are so gradual...2.5 is good
          // i * 3.4 is ok... i * 2.0 also ok
          // add is usually just 1. maybe get rid of + add + 1?
    noiseValue = constrain(noiseValue, 0.0, 1.0);
    int tab = (int)(noiseValue * 23.0);  // Scale noise to select note from noteTable
    tab = constrain(tab, 0, 23);  // Ensure tab is within bounds...might not be needed
    bound = noteTable[tab];
    bound = bound / divisor;
    //bound = constrain(bound, 24, 255); 
    int value = random(0, 1023);
    float falue = map(value, 0, 1023, 1, 3000) / 1000.0;
    lowpass = falue;
    int bond = bound * i;
    int vari = i * 3;
    bond = bond + vari;
    bond = bond / 5;
    delay(bond);

    for(int q = 0; q < 3; q++) {
      val[q] = analogRead(q);
      Serial.print(val[q]);
      Serial.print(F(" "));
    }
    Serial.print(noteTable[tab]);
    Serial.print(F(" "));
    Serial.print(tab);
    Serial.print(F(" "));
    Serial.println(floatPot);

    if (digitalRead(BUTTON_PIN) == LOW) {
      Serial.println(F("Button pressed. Exiting Program();"));
      break;
    }
  }
  Serial.println(F("Executing FlyingMarlin();"));
}


void loop() {
  // Check for button press and update the mode
  if (digitalRead(BUTTON_PIN) == LOW) {
    mode = (mode % 4) + 1; // Cycle through modes 1-8
    Serial.print(F("Switched to Program "));
    Serial.println(mode);
       // Write the current mode to EEPROM
    EEPROM.write(0, mode);
    delay(500); // Debounce delay
  }

  // Call the appropriate program based on the mode
  switch (mode) {
 case 1:
       FlyingMarlin(noteTable, 0, 30, 1, 5.1, 1.1);
      break;
    case 2:
      FlyingMarlin(noteTable, 0, 30, 4, 2.1, 1.9);
      break;
    case 3:
      FlyingMarlin(noteTable, 20, 40, 2, 0.5, 2.3);
      break;
    case 4:
      FlyingMarlin(noteTable, 26, 65, 1.33, 0.25, 4.2);
      break;
    default:
      break;  
      
  }

}

 /*  some other cool settings
     FlyingFlounder(weights1, noteTable, 0, 30, 1); //weighted random, scale, beginning count and end count, divisor of note
     FlyingFlounder(weights1, noteTable, 0, 30, 4);
     FlyingFlounder(weights2, noteTable, 10, 20, 2);
     FlyingFlounder(randomweights, noteTable, 0, 7, 0.5);
     FlyingFlounder(weightsRanat, ranatTable, 0, 7, 1); 
     FlyingFlounder(weightsPunong, PunongTable, 0, 20, 4);  
*/
