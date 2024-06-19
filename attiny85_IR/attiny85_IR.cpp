/*
 * IRremote
 * Version 0.11 August, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 *
 * Modified by Kristian Lauszus to work with ATTiny85
 * For details, see http://blog.tkjelectronics.dk
 * Attiny85 adaptation by Nick Gammon http://arduino.cc/forum/index.php?topic=91488.msg687101#msg687101)
 * Spilt into two lib by philteta@gmail.com www.tetalab.org (IRTinyTX-lib-v1)
 *
 *Modified by Anor Neto on 04/01/2015 to send NEC codes
 *  http://github.com/anorneto
 */

#include "attiny85_IR.h"

void IRsend::sendNEC(unsigned long data, int nbits)
{
  enableIROut(38);
  mark(NEC_HDR_MARK);
  space(NEC_HDR_SPACE);
  for (int i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      mark(NEC_BIT_MARK);
      space(NEC_ONE_SPACE);
    } 
    else {
      mark(NEC_BIT_MARK);
      space(NEC_ZERO_SPACE);
    }
    data <<= 1;
  }
  mark(NEC_BIT_MARK);
  space(0);
}

/*void IRsend::sendRC6(unsigned long data, int nbits)
{
  enableIROut(36);
  data = data << (32 - nbits);
  mark(RC6_HDR_MARK);
  space(RC6_HDR_SPACE);
  mark(RC6_T1); // start bit
  space(RC6_T1);
  int t;
  for (int i = 0; i < nbits; i++) {
    if (i == 3) {
      // double-wide trailer bit
      t = 2 * RC6_T1;
    } 
    else {
      t = RC6_T1;
    }
    if (data & TOPBIT) {
      mark(t);
      space(t);
    } 
    else {
      space(t);
      mark(t);
    }

    data <<= 1;
  }
  space(0); // Turn off at end
} */

void IRsend::mark(int16_t time) {
    // Sends an IR mark for the specified number of microseconds.
    // The mark output is modulated at the PWM frequency.
    
    // Clear OC0A/OC0B on Compare Match when up-counting.
    // Set OC0A/OC0B on Compare Match when down-counting.    
    TCCR0A |= _BV(COM0B1); // Enable pin 6 (PB1) PWM output        
    delayMicroseconds(time);    
}

/* Leave pin off for time (given in microseconds) */
void IRsend::space(int16_t time) {
    // Sends an IR space for the specified number of microseconds.
    // A space is no output, so the PWM output is disabled.
    
    // Normal port operation, OC0A/OC0B disconnected.
    TCCR0A &= ~(_BV(COM0B1)); // Disable pin 6 (PB1) PWM output
    delayMicroseconds(time);    
}

void IRsend::enableIROut(uint8_t khz) {
  // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
  // The IR output will be on pin 6 (OC0B).
  // This routine is designed for 36-40KHz; if you use it for other values, it's up to you
  // to make sure it gives reasonable results.  (Watch out for overflow / underflow / rounding.)
  // TIMER0 is used in phase-correct PWM mode, with OCR0A controlling the frequency and OCR0B
  // controlling the duty cycle.
  // There is no prescaling, so the output frequency is 16MHz / (2 * OCR0A)
  // To turn the output on and off, we leave the PWM running, but connect and disconnect the output pin.
  // A few hours staring at the ATmega documentation and this will all make sense.
  // See my Secrets of Arduino PWM at http://arcfn.com/2009/07/secrets-of-arduino-pwm.html for details.
  
  DDRB |= _BV(IRLED); // Set as output

  PORTB &= ~(_BV(IRLED)); // When not sending PWM, we want it low

  // Normal port operation, OC0A/OC0B disconnected  
  // COM0A = 00: disconnect OC0A
  // COM0B = 00: disconnect OC0B; to send signal set to 10: OC0B non-inverted    
  // WGM0 = 101: phase-correct PWM with OCR0A as top
  // CS0 = 000: no prescaling
  TCCR0A = _BV(WGM00);
  TCCR0B = _BV(WGM02) | _BV(CS00);

  // The top value for the timer.  The modulation frequency will be SYSCLOCK / 2 / OCR0A.
  OCR0A = F_CPU / 2 / khz / 1000;
  OCR0B = OCR0A / 3; // 33% duty cycle
}
