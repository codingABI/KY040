/* 
 * Example for using the rotary encoder with pin change interrupts 
 */ 

#include <KY040.h>

#define CLK_PIN 5 // aka. A
#define DT_PIN 4 // aka. B
KY040 g_rotaryEncoder(CLK_PIN,DT_PIN);

// Rotary encoder value (will be set in ISR)
volatile int v_value=0;

// Enable pin change interrupt
void pciSetup(byte pin) {
  *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
  PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
  PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

// ISR to handle pin change interrupt for D0 to D7 here
ISR (PCINT2_vect) { 
  // Process pin state
  switch (g_rotaryEncoder.getRotation()) {
    case KY040::CLOCKWISE:
      v_value++;
      break;
    case KY040::COUNTERCLOCKWISE:
      v_value--;
      break;
  }
}

void setup() {
  Serial.begin(9600);

  // Set pin change interrupt for CLK and DT
  pciSetup(CLK_PIN);
  pciSetup(DT_PIN);
}

void loop() {
  static int lastValue = 0;
  int value;

  // Get rotary encoder value set in ISR
  cli();
  value = v_value;
  sei();

  // Show, if value has changed
  if (lastValue != value) {
    Serial.println(value);
    lastValue = value;
  }
}