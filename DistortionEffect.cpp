#include "DistortionEffect.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "IKeyboardControl.h"
#include "resource.h"
//#include "DistortionEffect.rc"

#include <math.h>
#include <algorithm>

const int kNumPrograms = 5;

enum EParams
{
  mWaveform = 0,
  mAttack,
  mDecay,
  mSustain,
  mRelease,
  mFilterMode,
  mFilterCutoff,
  mFilterResonance,
  mFilterAttack,
  mFilterDecay,
  mFilterSustain,
  mFilterRelease,
  mFilterEnvelopeAmount,
  kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,
  kKeybX = 1,
  kKeybY = 230
};

DistortionEffect::DistortionEffect(IPlugInstanceInfo instanceInfo) 
    : IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo),
        lastVirtualKeyboardNoteNumber(virtualKeyboardMinimumNoteNumber - 1) {

  TRACE;

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachBackground(BG_ID, BG_FN);

  IBitmap whiteKeyImage = pGraphics->LoadIBitmap(WHITE_KEY_ID, WHITE_KEY_FN, 6);
  IBitmap blackKeyImage = pGraphics->LoadIBitmap(BLACK_KEY_ID, BLACK_KEY_FN);

  //                            C#     D#          F#      G#      A#
  int keyCoordinates[12] = { 0, 7, 12, 20, 24, 36, 43, 48, 56, 60, 69, 72 };
  mVirtualKeyboard = new IKeyboardControl(this, kKeybX, kKeybY, virtualKeyboardMinimumNoteNumber, /* octaves: */ 5, &whiteKeyImage, &blackKeyImage, keyCoordinates);

  pGraphics->AttachControl(mVirtualKeyboard);
        
  // Waveform switch
  GetParam(mWaveform)->InitEnum("Waveform", OSCILLATOR_MODE_SINE, kNumOscillatorModes);
  GetParam(mWaveform)->SetDisplayText(0, "Sine"); // Needed for VST3, thanks plunntic
  IBitmap waveformBitmap = pGraphics->LoadIBitmap(WAVEFORM_ID, WAVEFORM_FN, 4);
  pGraphics->AttachControl(new ISwitchControl(this, 24, 36, mWaveform, &waveformBitmap));

  // Knob bitmap for ADSR
  IBitmap knobBitmap = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, 128);
  // Attack knob:
  GetParam(mAttack)->InitDouble("Attack", 0.01, 0.01, 10.0, 0.001);
  GetParam(mAttack)->SetShape(3);
  pGraphics->AttachControl(new IKnobMultiControl(this, 103, 36, mAttack, &knobBitmap));
  // Decay knob:
  GetParam(mDecay)->InitDouble("Decay", 0.5, 0.01, 15.0, 0.001);
  GetParam(mDecay)->SetShape(3);
  pGraphics->AttachControl(new IKnobMultiControl(this, 185, 36, mDecay, &knobBitmap));
  // Sustain knob:
  GetParam(mSustain)->InitDouble("Sustain", 0.1, 0.001, 1.0, 0.001);
  GetParam(mSustain)->SetShape(2);
  pGraphics->AttachControl(new IKnobMultiControl(this, 268, 36, mSustain, &knobBitmap));
  // Release knob:
  GetParam(mRelease)->InitDouble("Release", 1.0, 0.001, 15.0, 0.001);
  GetParam(mRelease)->SetShape(3);
  pGraphics->AttachControl(new IKnobMultiControl(this, 346, 36, mRelease, &knobBitmap));

  AttachGraphics(pGraphics);

  // Filter
  GetParam(mFilterMode)->InitEnum("Filter Mode", Filter::FILTER_MODE_LOWPASS, Filter::kNumFilterModes);
  IBitmap filtermodeBitmap = pGraphics->LoadIBitmap(FILTER_ID, FILTER_FN, 3);
  pGraphics->AttachControl(new ISwitchControl(this, 24, 123, mFilterMode, &filtermodeBitmap));

  // Knobs for filter cutoff and resonance
  IBitmap smallKnobBitmap = pGraphics->LoadIBitmap(KNOB_SMALL_ID, KNOB_SMALL_FN, 128);
  // Cutoff knob:
  GetParam(mFilterCutoff)->InitDouble("Cutoff", 0.99, 0.01, 0.99, 0.001);
  GetParam(mFilterCutoff)->SetShape(2);
  pGraphics->AttachControl(new IKnobMultiControl(this, 5, 177, mFilterCutoff, &smallKnobBitmap));
  // Resonance knob:
  GetParam(mFilterResonance)->InitDouble("Resonance", 0.01, 0.01, 1.0, 0.001);
  pGraphics->AttachControl(new IKnobMultiControl(this, 61, 177, mFilterResonance, &smallKnobBitmap));

  CreatePresets();
  mMIDIReceiver.noteOn.Connect(this, &DistortionEffect::onNoteOn);
  mMIDIReceiver.noteOff.Connect(this, &DistortionEffect::onNoteOff);

  mEnvelopeGenerator.beganEnvelopeCycle.Connect(this, &DistortionEffect::onBeganEnvelopeCycle);
  mEnvelopeGenerator.finishedEnvelopeCycle.Connect(this, &DistortionEffect::onFinishedEnvelopeCycle);

}

DistortionEffect::~DistortionEffect() {}

void DistortionEffect::CreatePresets() {}

void DistortionEffect::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames) {
  double* leftOutput = outputs[0];
  double* rightOutput = outputs[1];

  processVirtualKeyboard();
  for (int i = 0; i < nFrames; ++i) {
      mMIDIReceiver.advance();
      int velocity = mMIDIReceiver.getLastVelocity();
      mOscillator.setFrequency(mMIDIReceiver.getLastFrequency());
      leftOutput[i] = rightOutput[i] = mFilter.process(mOscillator.nextSample() * mEnvelopeGenerator.nextSample() * velocity / 127.0);
  }

  mMIDIReceiver.Flush(nFrames);
}

void DistortionEffect::Reset() {
    TRACE;
    IMutexLock lock(this);
    mOscillator.setSampleRate(GetSampleRate());
    mEnvelopeGenerator.setSampleRate(GetSampleRate());
}

void DistortionEffect::OnParamChange(int paramIdx) {
  IMutexLock lock(this);
  switch (paramIdx) {
  case mWaveform:
      mOscillator.setMode(static_cast<OscillatorMode>(GetParam(mWaveform)->Int()));
      break;
  case mAttack:
  case mDecay:
  case mSustain:
  case mRelease:
      mEnvelopeGenerator.setStageValue(static_cast<EnvelopeGenerator::EnvelopeStage>(paramIdx), GetParam(paramIdx)->Value());
      break;
  case mFilterCutoff:
    mFilter.setCutoff(GetParam(paramIdx)->Value());
    break;
  case mFilterResonance:
    mFilter.setResonance(GetParam(paramIdx)->Value());
    break;
  case mFilterMode:
    mFilter.setFilterMode(static_cast<Filter::FilterMode>(GetParam(paramIdx)->Int()));
    break;
  }
}

void DistortionEffect::ProcessMidiMsg(IMidiMsg* pMsg) {
  mMIDIReceiver.onMessageReceived(pMsg);
  mVirtualKeyboard->SetDirty();
}

void DistortionEffect::processVirtualKeyboard() {
  IKeyboardControl* virtualKeyboard = (IKeyboardControl*)mVirtualKeyboard;
  int virtualKeyboardNoteNumber = virtualKeyboard->GetKey() + virtualKeyboardMinimumNoteNumber;

  if (lastVirtualKeyboardNoteNumber >= virtualKeyboardMinimumNoteNumber
        && virtualKeyboardNoteNumber != lastVirtualKeyboardNoteNumber) {
    // The note number has changed from a valid key to something else (valid key or nothing). Release the valid key:
    IMidiMsg midiMessage;
    midiMessage.MakeNoteOffMsg(lastVirtualKeyboardNoteNumber, 0);
    mMIDIReceiver.onMessageReceived(&midiMessage);
  }

  if (virtualKeyboardNoteNumber >= virtualKeyboardMinimumNoteNumber 
        && virtualKeyboardNoteNumber != lastVirtualKeyboardNoteNumber) {
    // A valid key is pressed that wasn't pressed the previous call. Send a "note on" message to the MIDI receiver:
    IMidiMsg midiMessage;
    midiMessage.MakeNoteOnMsg(virtualKeyboardNoteNumber, virtualKeyboard->GetVelocity(), 0);
    mMIDIReceiver.onMessageReceived(&midiMessage);
  }

  lastVirtualKeyboardNoteNumber = virtualKeyboardNoteNumber;
}
