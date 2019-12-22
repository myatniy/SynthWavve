#include <math.h>

class Oscillator {
public:
  enum OscillatorMode {
    OSCILLATOR_MODE_SINE = 0,
    OSCILLATOR_MODE_SAW,
    OSCILLATOR_MODE_SQUARE,
    OSCILLATOR_MODE_TRIANGLE,
    kNumOscillatorModes
  };

  void setMode(OscillatorMode mode);
  void setFrequency(double frequency);
  void setSampleRate(double sampleRate);
  void generate(double* buffer, int nFrames);
  virtual double nextSample();
  void setPitchMod(double amount);
  void reset() { mPhase = 0.0; }
  

  Oscillator() :
    mOscillatorMode(OSCILLATOR_MODE_SINE),
    mPI(2 * acos(0.0)),
    twoPI(2 * mPI),
    mFrequency(440.0),
    mPitchMod(0.0),
    mPhase(0.0) { updateIncrement(); };

protected:
  double naiveWaveformForMode(OscillatorMode mode);
  OscillatorMode mOscillatorMode;
  static double mSampleRate;
  const double mPI;
  const double twoPI;
  double mFrequency;
  double mPitchMod;
  double mPhase;
  double mPhaseIncrement;
  void updateIncrement();
};
