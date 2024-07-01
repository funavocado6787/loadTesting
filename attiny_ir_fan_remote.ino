//Anor Batista Esteves Neto 
//  04/01/2015  
//  Attiny send IR Remote Key with button or clap(still working on this)

#include <attiny85_IR.h>

// defining pins
#define buttonPin  4    // attiny physical pin 3, button with pull down resistor
#define ledPin     0      // attiny physical pin 5 , normal led for visual feedback of when we are sending an ir command

//defining constants
//#define TRIGGER 550


IRsend irsend; // IR led on physical pin 6 (D1) , the IR output will be always on attiny physical pin 6

// Variables that will change:
int buttonState;             // the current reading from the input pin
int lastButtonState = HIGH;   // the previous reading from the input pin
// the following variables are long's because the time, measured in miliseconds,
// will quickly overflow an int
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers

void setup() {
  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin,HIGH); //activating internal pull-up on button pin
  pinMode(ledPin,OUTPUT);
  digitalWrite(ledPin,LOW);
}

void loop() {
  check_button_state();
 }


void check_button_state( void ){
    // read the state of the switch into a local variable:
    int reading = digitalRead(buttonPin);
    
    // check to see if you just pressed the button 
    // (i.e. the input went from LOW to HIGH),  and you've waited 
    // long enough since the last press to ignore any noise:  

    // If the switch changed, due to noise or pressing:
    if (reading != lastButtonState) {
      // reset the debouncing timer
      lastDebounceTime = millis();
    } 

    if ((millis() - lastDebounceTime) > debounceDelay) {
      // whatever the reading is at, it's been there for longer
      // than the debounce delay, so take it as the actual current state:
      // if the button state has changed:
      if (reading != buttonState)  buttonState = reading;
      if (buttonState == LOW) send_ir();
    }
    // save the reading.  Next time through the loop,
    // it'll be the lastButtonState:
    lastButtonState = reading;
}

void send_ir(){
    irsend.sendNEC(150468615, 32); // ir code for my fan light
    digitalWrite(ledPin,HIGH); // just a visual feedback
    delay(200);
    digitalWrite(ledPin,LOW);
}
