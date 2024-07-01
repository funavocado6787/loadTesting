//Anor Batista Esteves Neto 
//  08/01/2015  
//  Attiny send IR Remote Key with button or clap
// clap detection based on example from Manoj Kunthu (http://imanoj-tech.blogspot.com.br/2013/02/arduino-clapper.html)

#include <attiny85_IR.h>

// defining pins
#define buttonPin  4    // attiny physical pin 3, button with pull down resistor
#define ledPin     0      // attiny physical pin 5 , normal led for visual feedback of when we are sending an ir command
#define micPin    3   // Analog Pin 3 = attiny physical pin 2

//defining constants
#define TRIGGER 60 //point above average that is considered a clap
#define CLAPS_TO_ACTIVATE 2 //The number of claps detected before output is toggled
#define CLAP_TIME 4000 // Time between claps



IRsend irsend; // IR led on physical pin 6 (D1) , the IR output will be always on attiny physical pin 6

// Variables that will change:
int buttonState = HIGH;             // the current reading from the input pin
int lastButtonState = HIGH;   // the previous reading from the input pin
// the following variables are long's because the time, measured in miliseconds,
// will quickly overflow an int
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers

//variable to store the value read from the mic
int micValue = 0;

//CLAP STATE CONSTANTS
const int FINAL_DETECTED = 0, LOST_CONTINUITY = 1, CLAP_NOT_DETECTED = 2;

int SIZE = 3;
int buffer[3];
int loopIteration = 0;
int average = 0;
int total = 0;


void setup() {
  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin,HIGH); //activating internal pull-up on button pin
  pinMode(ledPin,OUTPUT);
  digitalWrite(ledPin,LOW);
}

void loop() {
  check_button();
  check_claps();
 }

//Check the button, if pressed send ir command.
void check_button( void ){
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

// Send ir command to toggle the fan light
void send_ir(){
    irsend.sendNEC(150468615, 32); // ir code for my fan light
    digitalWrite(ledPin,HIGH); // just a visual feedback
    delay(200);
    digitalWrite(ledPin,LOW);
}

//read the mic value and store it in a global variable
void read_mic(void){
  micValue = analogRead(micPin);  
}

// a clap is detected when the sensor value is greater than the average plus trigger value
boolean clap_detected()
{
    return micValue > average + TRIGGER;
}

void check_claps(){
    int clapState = detect_claps(CLAPS_TO_ACTIVATE);
    
    if(clapState == FINAL_DETECTED)
    {
      // indicate_clap(1);//turn off any clap indicating lights
       send_ir();
    }
    else if(clapState == LOST_CONTINUITY)
    {
      // indicate_clap(0);//turn off any clap indicating lights
    }
}

//funcion to detect claps, uses recursion
int detect_claps(int numClaps)
{
  int clapNum = numClaps;
  
  //Base Case - if clapNum is 0, then all claps have been accounted.
  if(clapNum == 0)
  {
    return FINAL_DETECTED;
  }
  
  //Read from mic and update ambient noise levels.
  read_mic();

  total = (total - buffer[loopIteration]) + micValue; 
  average = (total/SIZE);
  buffer[loopIteration] = micValue;
  
  loopIteration = (loopIteration+1)%SIZE;
  
  if(clap_detected())
  { 
    delay(50);
    for(int i = 0; i < CLAP_TIME; i++)
    {
      int clapState = detect_claps(clapNum - 1);   
      
      if(clapState == FINAL_DETECTED || clapState == LOST_CONTINUITY)
      {
         return clapState;
      }
    }
    return LOST_CONTINUITY;
  }
  return CLAP_NOT_DETECTED;
}