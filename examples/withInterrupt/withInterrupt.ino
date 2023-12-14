/* 
 * Example for using a rotary encoders with interrupts 
 */ 

#include <KY040.h>

// Rotary encoder
#define CLK_PIN 3 // aka. A
#define DT_PIN 2 // aka. B
KY040 g_rotaryEncoder(CLK_PIN,DT_PIN);

// Rotary encoder value (will be set in ISR)
volatile int v_value=0;

// ISR to handle the interrupts for CLK and DT
void ISR_rotaryEncoder() {
  // Process pin states for CLK and DT
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

  // Set interrupts for CLK and DT
  attachInterrupt(digitalPinToInterrupt(CLK_PIN), ISR_rotaryEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(DT_PIN), ISR_rotaryEncoder, CHANGE);
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