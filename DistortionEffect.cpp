#include "DistortionEffect.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"

const int kNumPrograms = 5;

enum EParams
{
  kFrequency = 0,
  kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,

  kFrequencyX = 79,
  kFrequencyY = 62,
  kKnobFrames = 128
};

DistortionEffect::DistortionEffect(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), mFrequency(1.)
{
  TRACE;

  //arguments are: name, defaultVal, minVal, maxVal, step, label
  GetParam(kFrequency)->InitDouble("Frequency", 440.0, 50.0, 20000.0, 0.01, "Hz");
  GetParam(kFrequency)->SetShape(2.0);

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  //pGraphics->AttachPanelBackground(&COLOR_GRAY_NEW);
  pGraphics->AttachBackground(BACKGROUND_ID, BACKGROUND_FN);

  IBitmap knob = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, kKnobFrames);

  pGraphics->AttachControl(new IKnobMultiControl(this, kFrequencyX, kFrequencyY, kFrequency, &knob));

  AttachGraphics(pGraphics);

  //MakePreset("preset 1", ... );
  //MakeDefaultPreset((char *) "-", kNumPrograms);
  CreatePresets();
}

DistortionEffect::~DistortionEffect() {}

void DistortionEffect::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
    // Mutex is already locked for us.

    double* leftOutput = outputs[0];
    double* rightOutput = outputs[1];

    mOscillator.generate(leftOutput, nFrames);

    // Copy left buffer into right buffer:
    for (int s = 0; s < nFrames; ++s) {
        rightOutput[s] = leftOutput[s];
    }
}

void DistortionEffect::Reset()
{
    TRACE;
    IMutexLock lock(this);
    mOscillator.setSampleRate(GetSampleRate());
}

void DistortionEffect::OnParamChange(int paramIdx)
{
    IMutexLock lock(this);

    switch (paramIdx)
    {
    case kFrequency:
        mOscillator.setFrequency(GetParam(kFrequency)->Value());
        break;

    default:
        break;
    }
}

void DistortionEffect::CreatePresets() {
    MakePreset("clean", 440.0);
}
