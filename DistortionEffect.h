#ifndef __DISTORTIONEFFECT__
#define __DISTORTIONEFFECT__

#include "IPlug_include_in_plug_hdr.h"
#include "Oscillator.h"

class DistortionEffect : public IPlug
{
public:
  DistortionEffect(IPlugInstanceInfo instanceInfo);
  ~DistortionEffect();

  void Reset();
  void OnParamChange(int paramIdx);
  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);

private:
  double mFrequency;
  void CreatePresets();
  Oscillator mOscillator;
};

#endif
