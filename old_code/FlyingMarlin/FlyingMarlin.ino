#include <EEPROM.h>

#define SAMPLE_RATE 22050 // adjust sample rate to lower pitch, 11025 for octave lower for example, 22050 normal
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
uint8_t bound = SIZE; //bound is the period of the delayline. it's important for pitch and timing routines later.
int accum = 0;
int lowpass = 0.1; // 0 ... 1.0
bool trig = false;
int tempSensor = analogRead(0); /* define where the temperature sensor is. 
                                  Use 3 pin TMP36GT9Z from analog devices! easy.
                                  Note that for 105F° the ADC is approx 185.
                                  and for 15° the ADC is approx 83
                                  roughly one degree F per bit but not exactly.
                                                                            */
int solarPanel = analogRead(1);   /* define where the wobbly voltage is. 
                                this is just a voltage divider hooked up to the solar panel, 
                                should vary from 5v to 0v depending on sun conditions. 
                                 can be used later.
                                 value reliably ferret between 600 and 900 with 100k and 81k resistor*/
int foilAntennae = analogRead(2); /* define where the foil antennae sensor is. 
                                  experimental variable. maybe you can make a foil antennae 
                                  easily inside the case?*/

int mode = 1; // Variable to track the current program mode
int val[6];

// note root -- needs to be under the size of the delayline window, can't be less than 256.  
float pulse = 255.0;

// messing around with tuning--WTP 3/7 prism plus 11 3 and 7
float note1 = pulse / (1.0/1.0);
float note2 = pulse / (33.0/32.0);
float note3 = pulse / (9.0/8.0);
float note4 = pulse / (147.0/128.0);
float note5 = pulse / (77.0/64.0);
float note6 = pulse / (21.0/16.0);
float note7 = pulse / (11.0/8.0);
float note8 = pulse / (3.0/2.0);
float note9 = pulse / (49.0/32.0);
float note10 = pulse / (441.0/256.0);
float note11 = pulse / (7.0/4.0);
float note12 = pulse / (63.0/32.0);

float note13 = pulse / (2.0/1.0);
float note14 = pulse / (33.0/16.0);
float note15 = pulse / (9.0/4.0);
float note16 = pulse / (147.0/64.0);
float note17 = pulse / (77.0/32.0);
float note18 = pulse / (21.0/8.0);
float note19 = pulse / (11.0/4.0);
float note20 = pulse / (3.0/1.0);
float note21 = pulse / (49.0/16.0);
float note22 = pulse / (441.0/128.0);
float note23 = pulse / (7.0/2.0);
float note24 = pulse / (63.0/16.0);


float noteTable[24] = {
  note1, note2, note3, note4, note5, note6, note7, note8, note9, note10, note11, note12, 
  note13, note14, note15, note16, note17, note18, note19, note20, note21, note22, note23, note24 
};



ISR(TIMER1_COMPA_vect) {
  OCR2A = 0xff & out;

  if (trig) {
    for (int i = 0; i < SIZE; i++) delaymem[i] = random();
    trig = false;
  } else {
    delaymem[locat++] = out;
    if (locat >= bound) locat = 0;
    curr = delaymem[locat];
    out = accum >> lowpass;
    accum = accum - out + ((last >> 1) + (curr >> 1));
    last = curr;
  }
}

void startPlayback() {
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  ASSR &= ~(_BV(EXCLK) | _BV(AS2));
  TCCR2A |= _BV(WGM21) | _BV(WGM20);
  TCCR2B &= ~_BV(WGM22);
  TCCR2A = (TCCR2A | _BV(COM2A1)) & ~_BV(COM2A0);
  TCCR2A &= ~(_BV(COM2B1) | _BV(COM2B0));
  TCCR2B = (TCCR2B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);
  OCR2A = 0;
  cli();
  TCCR1B = (TCCR1B & ~_BV(WGM13)) | _BV(WGM12);
  TCCR1A = TCCR1A & ~(_BV(WGM11) | _BV(WGM10));
  TCCR1B = (TCCR1B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);
  OCR1A = F_CPU / SAMPLE_RATE;
  TIMSK1 |= _BV(OCIE1A);
  sei();
}

void stopPlayback() {
  TIMSK1 &= ~_BV(OCIE1A);
  TCCR1B &= ~_BV(CS10);
  TCCR2B &= ~_BV(CS10);
  digitalWrite(SPEAKER_PIN, LOW);
}

void setup() {
  startPlayback();
  Serial.begin(9600);
  randomSeed(analogRead(4));
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  if(EEPROM.read(0) == 255) {
    mode = 1;
  } else {
    mode = EEPROM.read(0);
  }
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

void FlyingMarlin(float* noteTable, int phraseBegin, int phraseEnd, float divisor) {
  int j = 1;
  int phraseStop = phraseBegin - 1;
  for (int i = phraseBegin; i > phraseStop; i = i + j) {
    if (i == phraseEnd) {
      j = -1;
    }

    tempSensor = constrain(tempSensor, 83, 185);
    int mappedTemp = map(tempSensor, 83, 185, 200, 400);

    solarPanel = constrain(solarPanel, 630, 950);
    int mappedSolar = map(solarPanel, 630, 950, 200, 400);

    LED_PORT ^= 1 << LED_BIT;
    trig = true;
  
    float noiseValue = (perlinNoise(i * 0.1) + 1.0) / 2.0; 
          // i / 20.0 still works but changes are so gradual...2.5 is good
          // i * 3.4 is ok... i * 2.0 also ok
    noiseValue = constrain(noiseValue, 0.0, 1.0);
    int tab = (int)(noiseValue * 23.0);  // Scale noise to select note from noteTable

    tab = constrain(tab, 0, 23);  // Ensure tab is within bounds

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
      Serial.print(" ");
    }
    Serial.println();

    if (digitalRead(BUTTON_PIN) == LOW) {
      Serial.println(F("Button pressed. Exiting FlyingMarlin();"));
      break;
    }
  }
  Serial.println(F("Executing FlyingMarlin();"));
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    mode = (mode % 4) + 1;
    Serial.print(F("Switched to Program "));
    Serial.println(mode);
    EEPROM.write(0, mode);
    delay(500);
  }

  switch (mode) {
    case 1:
      FlyingMarlin(noteTable, 0, 30, 1);
      break;
    case 2:
      FlyingMarlin(noteTable, 0, 30, 4);
      break;
    case 3:
      FlyingMarlin(noteTable, 10, 20, 2);
      break;
    case 4:
      FlyingMarlin(noteTable, 7, 15, 5);
      break;
    default:
      break;  
  }
}
