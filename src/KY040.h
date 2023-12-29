/* 
 * Class: KY040
 * 
 * Description:
 * Class for KY-040 rotary encoders. Without builtin pull up resistors for 
 * CLK/DT you have to set pinMode( ,INPUT_PULLUP) before using the class.
 * The class works with or without interrupts and prevents bounces 
 * by ignoring invalid CLK/DT sequences
 * 
 * License: 2-Clause BSD License
 * Copyright (c) 2023 codingABI
 * For details see: LICENSE.txt
 * 
 * Valid clockwise sequence for CLK/DT: Low/High->Low/Low->High/Low->High/High
 * 
 *        0 1 2 3
 *     --+   +----   High
 * CLK   |   |
 *       +---+       Low
 *       
 *     ----+   +--   High
 * DT      |   |
 *         +---+     Low
 * 
 *
 * Valid counter-clockwise sequence for CLK/DT: High/Low->Low/Low->Low/High->High/High    
 * 
 *        0 1 2 3
 *     ----+   +---  High
 * CLK     |   |
 *         +---+     Low
 *         
 *     --+   +-----  High
 * DT    |   |
 *       +---+       Low
 */ 
#pragma once

#define KY040_VERSION "1.0.1"

#include <arduino.h>

// When using sleep modes wait X milliseconds for next sleep after a CLK/DT sequence start do prevent missing signals
#define PREVENTSLEEPMS 150
// Pin idle state
#define INITSTEP 0b11
// Max steps for a signal sequence
#define MAXSEQUENCESTEPS 4

class KY040 {
  public:
    enum directions { IDLE, ACTIVE, CLOCKWISE, COUNTERCLOCKWISE };
    KY040(byte clk_pin, byte dt_pin) {
      m_clk_pin = clk_pin; // aka. A
      m_dt_pin = dt_pin; // aka. B
      v_state = 255;
      v_lastResult = IDLE;
      v_lastSequenceStartMillis = millis();
      v_sequenceStep = 0;
      v_direction = IDLE;
      v_oldState = INITSTEP;
    }
    // Get pin states for CLK and DT (First bit is for CLK, second bit is for DT). Should be called from ISR, when needed
    byte getState() {
      return v_state;
    }
    // Set pin states for CLK and DT (First bit is for CLK, second bit is for DT). Should be called from ISR, when needed
    void setState(byte state) {
      v_state = state;
    }
    // Returns true, if device was running long enough to get a full sequence (Do not use inside ISR)
    bool readyForSleep() {
      cli();
      unsigned long lastStepMillis = v_lastSequenceStartMillis;
      sei();
      return (millis()-lastStepMillis > PREVENTSLEEPMS);
    }
    // Get and reset the last rotary rotation (Do not use inside ISR)
    byte getAndResetLastRotation() {
      cli();
      byte result = v_lastResult;
      v_lastResult = IDLE;
      sei();
      return result;
    }
    // Get pin state for CLK and DT and returns the current rotation state  
    byte getRotation() { 
      setState((digitalRead(m_clk_pin)<<1)+digitalRead(m_dt_pin));
      return checkRotation();
    }
    /* Returns current rotation state:
     * - KY040::CLOCKWISE = CLK/DT Sequence for clockwise rotation has finished
     * - KY040::COUNTERCLOCKWISE = CLK/DT Sequence for counter-clockwise rotation has finished
     * - KY040::IDLE = Rotary encoder is not rotating
     * - KY040::ACTIVE = Rotary encoder is rotating, but the CLK/DT sequence has not finished
     * If you do not use interrupts, you have to start checkRotation() or a function using 
     * checkRotation() very frequently in your loop to prevent missing signals 
     */
    byte checkRotation() {
      byte result = IDLE;
   
      if (v_state != v_oldState) { // State changed?
        if (v_sequenceStep == 0) { // Check for begin of rotation
          if (v_state == c_signalSequenceCW[0]) { // Begin of CW
            v_direction=CLOCKWISE;
            v_sequenceStep = 1;
            v_lastSequenceStartMillis = millis();
          }
          if (v_state == c_signalSequenceCCW[0]) { // Begin of CCW
            v_direction=COUNTERCLOCKWISE; 
            v_sequenceStep = 1;
            v_lastSequenceStartMillis = millis();
          }
        } else {
          switch (v_direction) {
            case CLOCKWISE:
              if (v_state == c_signalSequenceCW[v_sequenceStep]) {
                v_sequenceStep++;
                if (v_sequenceStep >= MAXSEQUENCESTEPS) { // Sequence has finished
                  result=v_direction;
                  v_lastResult=result;
                  v_direction=IDLE;
                  v_sequenceStep=0;
                } else result=ACTIVE;
              } else { 
                // Invalid sequence
                if (v_state == INITSTEP) { // Reset sequence in init state
                  v_direction=IDLE;
                  v_sequenceStep=0;
                }
              }
              break;
            case COUNTERCLOCKWISE:
              if (v_state == c_signalSequenceCCW[v_sequenceStep]) {
                v_sequenceStep++;
                if (v_sequenceStep >= MAXSEQUENCESTEPS) { // Sequence has finished
                  result=v_direction;
                  v_lastResult=result;
                  v_direction=IDLE;
                  v_sequenceStep=0;
                } else result=ACTIVE;
              } else { 
                // Invalid sequence
                if (v_state == INITSTEP) { // Reset sequence in init state
                  v_direction=IDLE;
                  v_sequenceStep=0;
                }
              }
              break;
          }
        }
        v_oldState = v_state;
      }
      // Prevent unsigned long overrun
      if (millis() - v_lastSequenceStartMillis > PREVENTSLEEPMS) {
          v_lastSequenceStartMillis = millis() - PREVENTSLEEPMS - 1;
      }
      return result;
    }
  private:
    byte m_clk_pin; // aka. A
    byte m_dt_pin; // aka. B
    volatile byte v_state;
    volatile byte v_lastResult;
    volatile unsigned long v_lastSequenceStartMillis;
    volatile byte v_sequenceStep;
    volatile byte v_direction;
    volatile byte v_oldState;
    // CLK/DT sequence for a clockwise rotation (One byte instead of a byte array would be enough for the four 2-bit values, but are harder to read)
    const byte c_signalSequenceCW[MAXSEQUENCESTEPS] = {0b01,0b00,0b10,INITSTEP};
    // CLK/DT sequence for a counter-clockwise rotation (One byte instead of a byte array would be enough for the four 2-bit values, but are harder to read)
    const byte c_signalSequenceCCW[MAXSEQUENCESTEPS] = {0b10,0b00,0b01,INITSTEP};    

};