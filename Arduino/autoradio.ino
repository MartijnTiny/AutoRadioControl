//http://pastebin.com/fXbScxV4
// ############################################################################################################
// #  This code is meant to interface a 6-wire Renault Twingo / Clio steering wheel remote control with       #
// #                                                                                                          #
// ############################################################################################################
//

// Renault Twingo / Clio steering wheel remote wire functions
// pin_cycle_current  1     0              2
//         OUTPUTS    BLUE  GREEN          YELLOW
// INPUTS  PIN#       3     5              6
// BLACK   2          MUTE  TOP RIGHT BTN  TOP LEFT BTN
// RED     4          ++  BOTTOM BTN     VOL-
//                    HIGH  HIGH           LOW    SCROLL UP (CCW)
// BROWN   7          HIGH  LOW            HIGH  SCROLLWHEEL
//                    LOW   HIGH           HIGH   SCROLL DN (CW)
// Outputs are set LOW one at a time (the other outputs will be HIGH). Inputs (with internal pull-up) are then evaluated.
//   If an input is being pulled LOW this means a button is being pressed. Taking into account which output is currently LOW
//   we know which button this is. For example, is output pin 3 (Blue wire) is currently LOW and we also read LOW on
//   input pin 2 (Black) we know the MUTE button is being pressed.
// For the scrollwheel we must take into account its last known position in order to determine if there has been a change.
// We can determine the direction based on which pins are being pulled LOW.
 
// Connect Renault Twingo / Clio steering wheel remote wires to these pins
#define BLACKPIN    4 // D4 Bruin/Wit
#define BLUEPIN     5 // D5 Blauw
#define REDPIN      7 // D7 Oranje/Wit
#define GREENPIN    6 // D6 Groen
#define YELLOWPIN   3 // D3 Oranje
#define BROWNPIN    8 // D8 Bruin

#define LEDPIN A4
 
// On-board LED, useful for debugging
#define  VolP_PIN 10
#define  VolM_PIN 11
#define  Mute_PIN 12
#define  Next_PIN A3
#define  Prev_PIN A2
#define  Sel_PIN  13
#define  Play_PIN A0
#define  Mode_PIN A1

//Groen:    Vol+
//Groenwit: Vol-
//Blauw:    Mute/Pwr
//oranjewit:Next>>|  
//oranje:   Prev |<<
//Blauwwit: Sel
//Bruin:    1/play >
//Bruinwit: Mode

// Set number of output pins and put those pins in an array to cycle through when polling the input pins
#define OUT_PINS 3
unsigned char out_pins[OUT_PINS] = {GREENPIN, BLUEPIN, YELLOWPIN};
unsigned char oldKey = 0;
unsigned int dly = 500;
void setup() {
 
  // Set the pins connected to the steering wheel remote as input / output
  pinMode(BLACKPIN, INPUT_PULLUP);
  pinMode(BLUEPIN, OUTPUT);
  pinMode(REDPIN, INPUT_PULLUP);
  pinMode(GREENPIN, OUTPUT);
  pinMode(YELLOWPIN, OUTPUT);
  pinMode(BROWNPIN, INPUT_PULLUP);

  pinMode(VolP_PIN, OUTPUT);
  pinMode(VolM_PIN, OUTPUT);
  pinMode(Mute_PIN, OUTPUT);
  pinMode(Next_PIN, OUTPUT);
  pinMode(Prev_PIN, OUTPUT);
  pinMode(Sel_PIN, OUTPUT);
  pinMode(Play_PIN, OUTPUT);
  pinMode(Mode_PIN, OUTPUT);
   
  pinMode(LEDPIN, OUTPUT);                  // Set pin connected to on-board LED as output...
  digitalWrite(LEDPIN, LOW);                // ...and turn LED off
  for (unsigned char i = 0; i <= 7; i++) {  // Flash on-board LED a few times so it's easy to see when the Arduino is ready
    delay(100);
    digitalWrite(LEDPIN, !digitalRead(LEDPIN));
  }
  delay(100);
  digitalWrite(LEDPIN, LOW);                // Make sure LED ends up being off
}
 
// The steering wheel remote has 6 buttons and a scrollwheel, interfaced via 6 wires.
// This function will cycle through the output pins, setting one pin LOW at a time.
// It will then poll the input pins to see which input pins - if any - are pulled LOW.
unsigned char GetInput(void) {
  static unsigned char pin_cycle_current = 0;  // To keep track of which output pin is currently LOW
  static unsigned char pin_cycle_stored;       // To store the last known scrollwheel position
  static boolean first_run = true;             // After booting, there is no known last position for the scrollwheel
                                               // So on the first poll of the scrollwheel just store the current position and don't send a command

  unsigned char i;
 
  if (++pin_cycle_current > (OUT_PINS - 1)) pin_cycle_current = 0;     // Reset pin_cycle_current counter after last pin
 
  for (i = 0; i < OUT_PINS; i++) {                                     // Cycle through the output pins, setting one of them LOW and the rest HIGH
    if (i == pin_cycle_current)
      digitalWrite(out_pins[i], LOW);
    else
      digitalWrite(out_pins[i], HIGH);
  }
  
  if (!digitalRead(BROWNPIN)) {                                        // We're only interested if this pin is being pulled LOW
    if (pin_cycle_current != pin_cycle_stored) {                       // If the output that's currently LOW is different from the one that was LOW the last time   
                                                                       //   we came through here, then the scrollwheel has changed position
      signed char scrollwheel_current = pin_cycle_current - pin_cycle_stored; // Result of this calculation can range from -2 to 2
      pin_cycle_stored = pin_cycle_current;                            // Store which output pin is currently LOW
      if (first_run) {                                                 // If this is the first run, don't send a command
        first_run = false;                                             //   (since there was no previously known scrollwheel position)
        return 1;
      }
      if ((scrollwheel_current == 1) || (scrollwheel_current == -2)) { // If above calculation resulted in 1 or -2 the scrollwheel was rotated up (ccw)
        return Prev_PIN;
      }else {                                        // If above calculation resulted in anything else the scrollwheel was rotated down (cw)
        return Next_PIN;
      }
    }
  }
 
  if (!digitalRead(REDPIN)) {   // We're only interested if this pin is being pulled LOW
    switch(pin_cycle_current) {
    case 0:                     // RED (input) is LOW while GREEN (output) is LOW: bottom button pressed
      return Mode_PIN;
      break;
    case 1:                     // RED (input) is LOW while BLUE (output) is LOW: volume + button pressed
      return VolP_PIN;
      break;
    case 2:                     // RED (input) is LOW while YELLOW (output) is LOW: volume - button pressed
      return VolM_PIN;
      break;
    }
  }
 
  if (!digitalRead(BLACKPIN)) { // We're only interested if this pin is being pulled LOW
    switch(pin_cycle_current) {
    case 0:                     // BLACK (input) is LOW while GREEN (output) is LOW: top right button is pressed
      //aButtons[Play] = true;
      return Play_PIN;
      break;
    case 1:                     // BLACK (input) is LOW while BLUE (output) is LOW: mute button is pressed
      //aButtons[Mute] = true;
      return Mute_PIN;
      break;
    case 2:                     // BLACK (input) is LOW while YELLOW (output) is LOW: top left button is pressed
      //aButtons[Mode] = true;
      return Mode_PIN;
      break;
    }
  }
 
  return 0;
}
 
void loop() {
  unsigned char Key = GetInput();  // If any buttons are being pressed the GetInput() function will return the appropriate command code
  if (Key) {  // If no buttons are being pressed the function will have returned 0 and no command will be sent
    //digitalWrite(VolP_PIN,aButtons[VolP]);    
    digitalWrite(Key,HIGH);
    delay(20);
    digitalWrite(Key,LOW);
    if(oldKey == Key)
    {
      delay(dly);
      dly = dly - 20;
      if(dly < 100)
      {
        dly = 100;
      }
    }
    else
    {
      delay(100);
    }
  }
  if(oldKey != Key)
  {
      dly = 500;
  }
  oldKey = Key;  
}
 

