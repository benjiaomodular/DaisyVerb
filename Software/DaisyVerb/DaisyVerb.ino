// Title: reverbsc
// Description: Applies reverb to input signal
// Hardware: Daisy Seed
// Author: Stephen Hensley

#include "DaisyDuino.h"

DaisyHardware hw;

size_t num_channels;

ReverbSc DSY_SDRAM_BSS verb;
PitchShifter DSY_SDRAM_BSS ps;
static Jitter jitter;

static float dryLevel, wetLevel, send;
float jitterMix;


float CtrlVal(uint8_t pin) { return (analogRead(pin)) / 1023.f; }

void MyCallback(float **in, float **out, size_t size) {
  float dryL, dryR, verbL, verbR;
  float shifted;
  float jitter_out;

  for (size_t i = 0; i < size; i++) {
    dryL = in[0][i];
    dryR = in[1][i];

    jitter_out = jitter.Process();
 
    verb.Process(dryL, dryR, &verbL, &verbR);
    shifted = ps.Process(verbL);

    out[0][i] = (dryL * dryLevel) + shifted * ((1-jitterMix) + (jitter_out * jitterMix));
    out[1][i] = (dryR * dryLevel) + verbR;
  }
}

void setup() {
  float sample_rate;
  
  // Initialize for Daisy pod at 48kHz
  hw = DAISY.init(DAISY_SEED, AUDIO_SR_48K);
  num_channels = hw.num_channels;
  sample_rate = DAISY.get_samplerate();
 
  // Initialize reverb
  verb.Init(sample_rate);
  verb.SetFeedback(0.95f);
  verb.SetLpFreq(18000.0f);

  // Initialize pitch shifter
  ps.Init(sample_rate);
  ps.SetTransposition(.0f);

  jitter.Init(sample_rate);
  jitter.SetAmp(1);
  jitter.SetCpsMin(1);
  jitter.SetCpsMax(30);

  wetLevel = 0.1f;
  DAISY.begin(MyCallback);
}

void loop() {
  dryLevel = CtrlVal(A0);
  verb.SetFeedback(.50f + CtrlVal(A1) * .49f);
  
  // Minimum offset a workaround for weird noise when set to 0
  ps.SetTransposition(0.000001 + CtrlVal(A2) * 12.0f);

  jitterMix = CtrlVal(A3);
  verb.SetLpFreq(CtrlVal(A4) * 20000.0f);

}
