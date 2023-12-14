/* 
 * Example for using the rotary encoder without interrupts
 * in polling mode
 */ 

#include <KY040.h>

#define CLK_PIN 6 // aka. A
#define DT_PIN 5 // aka. B
KY040 g_rotaryEncoder(CLK_PIN,DT_PIN);

void setup() {
  Serial.begin(9600); 
  // If you rotary encoder has no builtin pullup resistors for CLK (aka. A) and DT (aka. B) uncomment the following two lines
  // pinMode(CLK_PIN,INPUT_PULLUP);
  // pinMode(DT,INPUT_PULLUP);
}

void loop() {
  static int value=0;

  // You have to run getRotation() very frequently in loop to prevent missing rotary encoder signals
  // If this is not possible take a look at the pinChangeInterrupt examples
  switch (g_rotaryEncoder.getRotation()) {
    case KY040::CLOCKWISE:
      value++;
      Serial.println(value);
      break;
    case KY040::COUNTERCLOCKWISE:
      value--;
      Serial.println(value);
      break;
  }
}