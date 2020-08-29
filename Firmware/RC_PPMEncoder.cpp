//Taken from Christopher Schirner
// https://github.com/schinken/PPMEncoder

#include "RC_PPMEncoder.h"

// UNUSED: the macro sets or clears the appropriate bit in port D if the pin is less than 8 or port B if between 8 and 13
//#define fastWrite(_pin_, _state_) ( _pin_ < 8 ? (_state_ ?  PORTD |= 1 << _pin_ : PORTD &= ~(1 << _pin_ )) : (_state_ ?  PORTB |= 1 << (_pin_ -8) : PORTB &= ~(1 << (_pin_ -8)  )))

// Macro to use PORTD access for D10 on Mega2560
//#define FAST_WRITE_D10_MEGA2560

// Singleton
PPMEncoder ppmEncoder;

uint16_t PPMEncoder::PPM_INTERVAL_LENGTH_us = 300;
uint16_t PPMEncoder::PPM_FRAME_LENGTH_us = 20000;
uint16_t PPMEncoder::MIN_us = 1000;
uint16_t PPMEncoder::MED_us = 1600;
uint16_t PPMEncoder::MAX_us = 2200;

void PPMEncoder::begin(uint8_t pin) {
  begin(pin, PPM_DEFAULT_CHANNELS);
}

void PPMEncoder::begin(uint8_t pin, uint8_t ch) {
  // Stop interupts
  cli();

  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);

  state = true;
  elapsed_us = 0;
  currentChannel = 0;

  if (ch>MAX_CHANNEL) {
    ch = MAX_CHANNEL;
  }
  numChannels = ch;
  outputPin = pin;
  state = PPM_STATE::WAITING_START_FRAME;
  
  for (uint8_t ch = 0; ch < numChannels; ch++) {
    setChannelPercent(ch, 0);
  }

  // Setup for 16000000/8(prescaler) = 2MHz timer1 clock (so 1 tick equals 0.5us)
  TCCR1B = (1 << WGM12) | (1 << CS11); // CTC mode + Prescaler 8
  // enable timer1 compare interrupt
  TIMSK1 = (1 << OCIE1A); 

  // Prepare timer1 values for first run
  TCCR1A = 0;
  OCR1A = 100<<1; // x2 = 100us
  
  // Enable interrupts
  sei();
}

void PPMEncoder::setChannel(uint8_t channel, uint16_t value) {
  if (channel>=numChannels)
    return;
  channels[channel] = constrain(value, PPMEncoder::MIN_us, PPMEncoder::MAX_us);
}

void PPMEncoder::setChannelPercent(uint8_t channel, uint8_t percent) {
  percent = constrain(percent, 0, 100);
  setChannel(channel, map(percent, 0, 100, PPMEncoder::MIN_us, PPMEncoder::MAX_us));
}

void PPMEncoder::setPinLevel(int level) {
#ifdef FAST_WRITE_D10_MEGA2560
  if (level==HIGH)
    PORTB |= 1<<4; // D10 on Mega2560
  else
    PORTB &= ~(1<<4); // D10 on Mega2560
#else
  digitalWrite(outputPin, level);
#endif  
}

void PPMEncoder::interrupt() {
  // Clear counter to reset interrupt flag
  TCNT1 = 0;
  
  // Should start with this
  if (state==PPM_STATE::WAITING_START_FRAME) {
    // Starting first pulse
    setPinLevel(HIGH);
    
    // Will wait for channel duration
    OCR1A = (channels[currentChannel]<<1) - 4; // 4cycles for processing time

    // Add channel pulse time
    elapsed_us = elapsed_us + channels[currentChannel];
    // State is now pulsing channel
    state = PPM_STATE::PULSING_CHANNEL;
    
  } else if (state==PPM_STATE::WAITING_INTERVAL) {
    // Starting first pulse
    setPinLevel(HIGH);
    
    // Interval done, wait for next channel length
    OCR1A = ((channels[currentChannel])<<1) - 4; // 4cycles for processing time
    
    elapsed_us = elapsed_us + channels[currentChannel];
    state = PPM_STATE::PULSING_CHANNEL;
    
  } else if (state==PPM_STATE::PULSING_CHANNEL) {
    
    // Terminating pulse
    setPinLevel(LOW);
    
    // Either add a 300us step and go to next channel, or wait until next frame
    currentChannel++;
    if (currentChannel < numChannels) {
      // Insert a wait interval
      OCR1A = (PPM_INTERVAL_LENGTH_us<<1) - 6; // 6cycles for processing time
      // Add interval pulse time
      elapsed_us = elapsed_us + PPM_INTERVAL_LENGTH_us;
      // State is now waiting interval
      state = PPM_STATE::WAITING_INTERVAL;      
    } else {
      // All done, wait until next frame

      // Configure timer for next frame start
      OCR1A = ((PPM_FRAME_LENGTH_us - elapsed_us)<<1) - 6; // 6cycles for processing time

      // Next Channel will be channel 0
      currentChannel = 0;
      // Reset frame timing and go back to waiting start
      elapsed_us = 0;
      state = PPM_STATE::WAITING_START_FRAME;
    }
  }
  
}

ISR(TIMER1_COMPA_vect) {
  ppmEncoder.interrupt();
}
