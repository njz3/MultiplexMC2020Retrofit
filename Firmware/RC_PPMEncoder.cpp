//Taken from Christopher Schirner
// https://github.com/schinken/PPMEncoder
#include "Config.h"
#include "RC_PPMEncoder.h"

// UNUSED: the macro sets or clears the appropriate bit in port D if the pin is less than 8 or port B if between 8 and 13
//#define fastWrite(_pin_, _state_) ( _pin_ < 8 ? (_state_ ?  PORTD |= 1 << _pin_ : PORTD &= ~(1 << _pin_ )) : (_state_ ?  PORTB |= 1 << (_pin_ -8) : PORTB &= ~(1 << (_pin_ -8)  )))

//#define TEST_PPM_CONST_CHANNEL

// Macro to use PORTD access for D10 on Mega2560
//#define FAST_WRITE_D10_MEGA2560


#if defined(MC2020_MEGA)
# define PPM_CORRECTION    7
#elif defined(MC2020_NANO)
# define PPM_CORRECTION    10 // TODO: measure the correct value here.
#else
# error "pouet"
# define PPM_CORRECTION    0
#endif

/* Sens de modulation Multiplex */
#define SET_PIN_LEVEL_INTERVAL()    setPinLevel(HIGH);
#define SET_PIN_LEVEL_PULSE()       setPinLevel(LOW);
#define SET_PIN_LEVEL_SYNC()        setPinLevel(LOW);


extern void Task_Adc2Ppm(void);

// Singleton
PPMEncoder ppmEncoder;

uint16_t PPMEncoder::PPM_INTERVAL_LENGTH_us = 300;
uint16_t PPMEncoder::MIN_us = 1000;
uint16_t PPMEncoder::MED_us = 1500; /**< arbitrary median value */
uint16_t PPMEncoder::MAX_us = 2200;
uint16_t PPMEncoder::PPM_FRAME_LENGTH_us = 20000;
uint16_t PPMEncoder::PPM_SYNC_MIN_LENGTH_us = 3000;

void PPMEncoder::begin(uint8_t pin) {
  begin(pin, PPM_DEFAULT_CHANNELS);
}

void PPMEncoder::begin(uint8_t pin, uint8_t ch) {
  // Stop interupts
  cli();

  state = true;
  elapsed_us = 0;
  currentChannel = 0;

  if (ch>MAX_CHANNEL) {
    ch = MAX_CHANNEL;
  }
  numChannels = ch;
  outputPin = pin;

  pinMode(pin, OUTPUT);
  SET_PIN_LEVEL_SYNC();

  state = PPM_STATE::WAITING_START_FRAME;
  
  for (uint8_t ch = 0; ch < numChannels; ch++) {
    setChannel(ch, PPMEncoder::MED_us); // init with arbitrary
  }

  /*******************/
  Task_Adc2Ppm();
  /*******************/

#ifndef SIMULATION_PC
  // Setup for 16000000/8(prescaler) = 2MHz timer1 clock (so 1 tick equals 0.5us)
  TCCR1B = (1 << WGM12) | (1 << CS11); // CTC mode + Prescaler 8
  // enable timer1 compare interrupt
  TIMSK1 = (1 << OCIE1A); 

  // Prepare timer1 values for first run
  TCCR1A = 0;
  TCNT1 = 0;
  OCR1A = 200*2; /* program the 1st interrupt 200us later */
#endif // SIMULATION_PC
  // Enable interrupts
  sei();
}

void PPMEncoder::setNbChannel(uint8_t numChannels) {
   if (numChannels<MAX_CHANNEL)
      this->numChannels = numChannels;
}

void PPMEncoder::setChannel(uint8_t channel, uint16_t value) {
   if (channel<numChannels)
   {
      switch(channel)
      {
#ifdef TEST_PPM_CONST_CHANNEL
         case 0: channels[0] = 1000; break;
         case 1: channels[1] = 1100; break;
         case 2: channels[2] = 1200; break;
         case 3: channels[3] = 1300; break;
#endif
         default:
            channels[channel] = constrain(value, PPMEncoder::MIN_us, PPMEncoder::MAX_us);
            break;
      }
   }
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
  /* Caution: digitalWrite runtime might be not the same for Nano and Mega !! */
  digitalWrite(outputPin, level);
#endif //FAST_WRITE_D10_MEGA2560
}

#ifndef SIMULATION_PC
void PPMEncoder::interrupt() {
   // Clear counter to reset interrupt flag
   TCNT1 = 0;
  
   switch(state)
   {
      case WAITING_START_FRAME:     // Should start with this
         currentChannel = 0;
         SET_PIN_LEVEL_INTERVAL();  // Starting the interval before the first pulse
         // next interrupt will happen after interval duration
         OCR1A = PPM_INTERVAL_LENGTH_us*2 - PPM_CORRECTION; // some cycles for processing time;
         elapsed_us = PPM_INTERVAL_LENGTH_us;  // set initial value
         state = WAITING_INTERVAL;
         break;

      case WAITING_INTERVAL:    // interval done, so now starting pulse or starting sync
         if (currentChannel < numChannels)
         {
            SET_PIN_LEVEL_PULSE(); // Starting pulse
            // next interrupt will happen after pulse duration
            // notice that interval duration is part of the channel duration in Multiplex modulation coding
            OCR1A = (channels[currentChannel]-PPM_INTERVAL_LENGTH_us)*2 - PPM_CORRECTION; // some cycles for processing time
            elapsed_us += channels[currentChannel]-PPM_INTERVAL_LENGTH_us;    // add pulse duration
            state = PULSING_CHANNEL;
            currentChannel++;
         }
         else
         {
            SET_PIN_LEVEL_SYNC(); // Starting sync
            // next interrupt will happen after sync duration
            if(PPM_SYNC_MIN_LENGTH_us < (PPM_FRAME_LENGTH_us - elapsed_us) )
               OCR1A = ((PPM_FRAME_LENGTH_us - elapsed_us)*2) - PPM_CORRECTION; // some cycles for processing time
            else
               OCR1A = ((PPM_SYNC_MIN_LENGTH_us)*2) - PPM_CORRECTION; // some cycles for processing time
            state = WAITING_START_FRAME;
            currentChannel = 0; // optional: anyway reseted at case WAITING_START_FRAME

            /**********************/
            Task_Adc2Ppm();
            /**********************/
         }
         break;

      case PULSING_CHANNEL:  // pulse done, so now starting interval
         SET_PIN_LEVEL_INTERVAL();  // Starting the interval
         // next interrupt will happen after interval duration
         OCR1A = PPM_INTERVAL_LENGTH_us*2 - PPM_CORRECTION; // some cycles for processing time
         elapsed_us += PPM_INTERVAL_LENGTH_us;  // add interval duration
         state = WAITING_INTERVAL;
         break;
  }
}

ISR(TIMER1_COMPA_vect) {
   ppmEncoder.interrupt();
}
#endif // SIMULATION_PC
