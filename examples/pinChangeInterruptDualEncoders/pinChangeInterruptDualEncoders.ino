/* 
 * Example for using two rotary encoders with pin change interrupts 
 */ 

#include <KY040.h>

// First rotary encoder
#define X_CLK_PIN 5 // aka. A
#define X_DT_PIN 4 // aka. B
KY040 g_rotaryEncoderX(X_CLK_PIN,X_DT_PIN);

// Second rotary encoder
#define Y_CLK_PIN 7 // aka. A
#define Y_DT_PIN 6 // aka. B
KY040 g_rotaryEncoderY(Y_CLK_PIN,Y_DT_PIN);

// Rotary encoder values (will be set in ISR)
volatile int v_valueX=0;
volatile int v_valueY=0;

// Enable pin change interrupt
void pciSetup(byte pin) {
  *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
  PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
  PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

// ISR to handle pin change interrupts for D0 to D7 here
ISR (PCINT2_vect) { 
  // Read pin states with PIND (Faster replacement for digitalRead, better for fast interrupts, but harder to read)
  byte state = PIND;
  byte stateX = ((state & 0b00110000)>>4);
  byte stateY = ((state & 0b11000000)>>6);

  // Process first rotary encoder
  g_rotaryEncoderX.setState(stateX); // Store CLK/DT states
  // Process stored state
  switch (g_rotaryEncoderX.checkRotation()) {
    case KY040::CLOCKWISE:
      v_valueX++;
      break;
    case KY040::COUNTERCLOCKWISE:
      v_valueX--;
      break;
  }

  // Process second rotary encoder
  g_rotaryEncoderY.setState(stateY); // Store CLK/DT states
  // Process stored state
  switch (g_rotaryEncoderY.checkRotation()) {
    case KY040::CLOCKWISE:
      v_valueY++;
      break;
    case KY040::COUNTERCLOCKWISE:
      v_valueY--;
      break;
  }
}

void setup() {
  Serial.begin(9600);

  // Set pin change interrupt for CLK and DT
  pciSetup(X_CLK_PIN);
  pciSetup(X_DT_PIN);
  pciSetup(Y_CLK_PIN);
  pciSetup(Y_DT_PIN);
}

void loop() {
  static int lastValueX = 0;
  static int lastValueY = 0;

  int valueX, valueY;

  // Get rotary encoder values set in ISR
  cli();
  valueX = v_valueX;
  valueY = v_valueY;
  sei();

  // Show, if value has changed
  if ((lastValueX != valueX) || (lastValueY != valueY)) {
    Serial.print("X:");
    Serial.print(valueX);
    Serial.print(" Y:");
    Serial.println(valueY);
    lastValueX = valueX;
    lastValueY = valueY;
  }
}