// Simple Karplus-Strong implemented for solar sounder, with temperature sensor.
// DFISHKIN, 2021—2024. Thanks electro-music.com forums for inspiration!
//

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
                                 can be used later.*/
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



int weights1[24] = {20, 1, 1, 1, 1, 1, 1, 20, 1, 1, 20, 1, 20, 1, 1, 1, 1, 1, 1, 20, 1, 1, 20, 1};
/*by increasing numbers for each index in the weighted array, you make an item more likely to be chosen. 
zero for no choices, all 1s for regular random algo*/

int weights2[24] = {1, 40, 1, 1, 1, 1, 40, 1, 1, 1, 1, 1, 1, 40, 1, 1, 1, 1, 40, 1, 1, 1, 1, 1};
//try to emphasize 7

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
   // Set up Timer 2 to do pulse width modulation on the speaker pin.
    
    ASSR &= ~(_BV(EXCLK) | _BV(AS2));   // Use internal clock (datasheet p.160)
    TCCR2A |= _BV(WGM21) | _BV(WGM20);
    TCCR2B &= ~_BV(WGM22);  // Set fast PWM mode  (p.157)

    TCCR2A = (TCCR2A | _BV(COM2A1)) & ~_BV(COM2A0);
    TCCR2A &= ~(_BV(COM2B1) | _BV(COM2B0));    
    // Do non-inverting PWM on pin OC2A (p.155)-- pin 11.
    
    TCCR2B = (TCCR2B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);   // No prescaler (p.158)
    OCR2A = 0;   // Set initial pulse width to zero/
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
  randomSeed(analogRead(0)); //consider another analog pin, or moving to the regular void loop.
    pinMode(BUTTON_PIN, INPUT_PULLUP); // Configure the button pin as input with pull-up resistor
    
  if(EEPROM.read(0)==255){ // when first uploading program, eeprom is empty (or 255)
    mode=1;           //so we need to check that and write a default state
  } else {
  mode = EEPROM.read(0);
  }
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

void FlyingFish(int* weights, float* noteTable, int phraseEnd, float divisor) {
  int j = 1;
  for (int i = 0; i > -1; i = i + j) {
                   // Wraps entire rhythm structure in this loop.
    if (i == phraseEnd) { // Catch the edge of the timer! this number 30 could be resized.
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
    int bond = bound * i; // Takes the bound, multiplies it by the expanding variable, will go to 30!
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
      Serial.println(F("Button pressed. Exiting FlyingFish();"));
      break;
    }
  }
  Serial.println(F("Executing FlyingFish();")); 
}



void loop() {
  // Check for button press and update the mode
  if (digitalRead(BUTTON_PIN) == LOW) {
    mode = (mode % 4) + 1; // Cycle through modes 1, 2, 3, 4
    Serial.print(F("Switched to Program "));
    Serial.println(mode);
       // Write the current mode to EEPROM
    EEPROM.write(0, mode);
    delay(500); // Debounce delay
  }

  // Call the appropriate program based on the mode
  switch (mode) {
    case 1:
     FlyingFish(weights1, noteTable, 30, 1);
     break;
    case 2:
     FlyingFish(weights1, noteTable, 30, 4);
     break;
     case 3:
     FlyingFish(weights2, noteTable, 20, 2);
     break;
     case 4:
     FlyingFish(randomweights, noteTable, 7, 0.5); 
     default:
      // Optional: handle unexpected values or do nothing
      break;  
      
  }

}
