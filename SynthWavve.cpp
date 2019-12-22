#include "SynthWavve.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "IKeyboardControl.h"
#include "resource.h"

#include <math.h>
#include <algorithm>

const int kNumPrograms = 5;
const double parameterStep = 0.001;

enum EParams {
  // Oscillator Section:
  mOsc1Waveform = 0,
  mOsc1PitchMod,
  mOscMix,
  mOsc2PitchMod,
  mOsc2Waveform,
  // Oscillator Section Envelope:
  mVolumeEnvAttack,
  mVolumeEnvDecay,
  mVolumeEnvSustain,
  mVolumeEnvRelease,
  // Filter Section:
  mFilterMode,
  mFilterCutoff,
  mFilterResonance,
  mFilterLfoAmount,
  mFilterEnvAmount,
  // Filter Section Envelope:
  mFilterEnvAttack,
  mFilterEnvDecay,
  mFilterEnvSustain,
  mFilterEnvRelease,
  // LFO Section:
  mLFOWaveform,
  mLFOFrequency,
  kNumParams
};


typedef struct {
  const char* name;
  const int x;
  const int y;
  const double defaultVal;
  const double minVal;
  const double maxVal;
} parameterProperties_struct;



const parameterProperties_struct parameterProperties[kNumParams] = {
  {"Osc 1 Waveform", 20, 29},
  {"Osc 1 Pitch Mod", 70, 20, 0.0, 0.0, 1.0},
  {"Osc Mix", 148, 20, 0.5, 0.0, 1.0},
  {"Osc 2 Pitch Mod", 226, 20, 0.0, 0.0, 1.0},
  {"Osc 2 Waveform", 286, 29},
  {"Volume Env Attack", 36, 130, 0.01, 0.01, 10.0},
  {"Volume Env Decay", 111, 130, 0.5, 0.01, 15.0},
  {"Volume Env Sustain", 185, 130, 0.1, 0.001, 1.0},
  {"Volume Env Release", 260, 130, 1.0, 0.01, 15.0},
  {"Filter Mode", 367, 29},
  {"Filter Cutoff", 418, 20, 0.99, 0.0, 0.99},
  {"Filter Resonance", 487, 20, 0.0, 0.0, 1.0},
  {"Filter LFO Amount", 557, 20, 0.0, 0.0, 1.0},
  {"Filter Envelope Amount", 618, 20, 0.0, -1.0, 1.0},
  {"Filter Env Attack", 381, 130, 0.01, 0.01, 10.0},
  {"Filter Env Decay", 456, 130, 0.5, 0.01, 15.0},
  {"Filter Env Sustain", 530, 130, 0.1, 0.001, 1.0},
  {"Filter Env Release", 605, 130, 1.0, 0.01, 15.0},
  {"LFO Waveform", 718, 29},
  {"LFO Frequency", 780, 20, 6.0, 0.01, 30.0}
};

enum ELayout {
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,
  kKeybX = 188,
  kKeybY = 254
};

SynthWavve::SynthWavve(IPlugInstanceInfo instanceInfo) 
: IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), 
lastVirtualKeyboardNoteNumber(virtualKeyboardMinimumNoteNumber - 1) {
  TRACE;
  
  CreateParams();
  CreateGraphics();
  CreatePresets();

  mMIDIReceiver.noteOn.Connect(this, &SynthWavve::onNoteOn);
  mMIDIReceiver.noteOff.Connect(this, &SynthWavve::onNoteOff);
  mEnvelopeGenerator.beganEnvelopeCycle.Connect(this, &SynthWavve::onBeganEnvelopeCycle);
  mEnvelopeGenerator.finishedEnvelopeCycle.Connect(this, &SynthWavve::onFinishedEnvelopeCycle);
}

SynthWavve::~SynthWavve() {}

void SynthWavve::CreatePresets() {}

void SynthWavve::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames) {
  double* leftOutput = outputs[0];
  double* rightOutput = outputs[1];

  processVirtualKeyboard();
  for (int i = 0; i < nFrames; ++i) {
    mMIDIReceiver.advance();
    int velocity = mMIDIReceiver.getLastVelocity();
    double LfoFilterModulation = mLFO.nextSample() * LfoFilterModAmount;
    mOscillator.setFrequency(mMIDIReceiver.getLastFrequency());
    mFilter.setCutoffMod((mFilterEnvelopeGenerator.nextSample() * filterEnvelopeAmount) + LfoFilterModulation);
    leftOutput[i] = rightOutput[i] = mFilter.process(mOscillator.nextSample() * mEnvelopeGenerator.nextSample() * velocity / 127.0);
  }

  mMIDIReceiver.Flush(nFrames);
}

void SynthWavve::Reset() {
    TRACE;
    IMutexLock lock(this);
    mOscillator.setSampleRate(GetSampleRate());
    mEnvelopeGenerator.setSampleRate(GetSampleRate());
    mFilterEnvelopeGenerator.setSampleRate(GetSampleRate());
    mLFO.setSampleRate(GetSampleRate());
}

void SynthWavve::OnParamChange(int paramIdx) {
  IMutexLock lock(this);
}

void SynthWavve::ProcessMidiMsg(IMidiMsg* pMsg) {
  mMIDIReceiver.onMessageReceived(pMsg);
  mVirtualKeyboard->SetDirty();
}

void SynthWavve::processVirtualKeyboard() {
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

void SynthWavve::CreateParams() {
  for (int i = 0; i < kNumParams; i++) {
    IParam* param = GetParam(i);
    const parameterProperties_struct& properties = parameterProperties[i];
    switch (i) {
      // Enum Parameters:
    case mOsc1Waveform:
    case mOsc2Waveform:
      param->InitEnum(properties.name,
        Oscillator::OSCILLATOR_MODE_SAW,
        Oscillator::kNumOscillatorModes);
      break;
    case mLFOWaveform:
      param->InitEnum(properties.name,
        Oscillator::OSCILLATOR_MODE_TRIANGLE,
        Oscillator::kNumOscillatorModes);
      break;
    case mFilterMode:
      param->InitEnum(properties.name,
        Filter::FILTER_MODE_LOWPASS,
        Filter::kNumFilterModes);
      break;
      // Double Parameters:
    default:
      param->InitDouble(properties.name,
                        properties.defaultVal,
                        properties.minVal,
                        properties.maxVal,
                        parameterStep);
      break;
    }
  }
  GetParam(mFilterCutoff)->SetShape(2);
  GetParam(mVolumeEnvAttack)->SetShape(3);
  GetParam(mFilterEnvAttack)->SetShape(3);
  GetParam(mVolumeEnvDecay)->SetShape(3);
  GetParam(mFilterEnvDecay)->SetShape(3);
  GetParam(mVolumeEnvSustain)->SetShape(2);
  GetParam(mFilterEnvSustain)->SetShape(2);
  GetParam(mVolumeEnvRelease)->SetShape(3);
  GetParam(mFilterEnvRelease)->SetShape(3);
  for (int i = 0; i < kNumParams; i++) {
    OnParamChange(i);
  }
}

void SynthWavve::CreateGraphics() {
  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  pGraphics->AttachBackground(BG_ID, BG_FN);
  IBitmap whiteKeyImage = pGraphics->LoadIBitmap(WHITE_KEY_ID, WHITE_KEY_FN, 6);
  IBitmap blackKeyImage = pGraphics->LoadIBitmap(BLACK_KEY_ID, BLACK_KEY_FN);
  //                            C#     D#          F#      G#      A#
  int keyCoordinates[12] = { 0, 10, 17, 29, 35, 52, 61, 68, 79, 85, 97, 102 };
  mVirtualKeyboard = new IKeyboardControl(this, kKeybX, kKeybY, virtualKeyboardMinimumNoteNumber, /* octaves: */ 4, &whiteKeyImage, &blackKeyImage, keyCoordinates);
  pGraphics->AttachControl(mVirtualKeyboard);
  IBitmap waveformBitmap = pGraphics->LoadIBitmap(WAVEFORM_ID, WAVEFORM_FN, 4);
  IBitmap filterModeBitmap = pGraphics->LoadIBitmap(FILTER_ID, FILTER_FN, 3);
  IBitmap knobBitmap = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, 128);
  for (int i = 0; i < kNumParams; i++) {
    const parameterProperties_struct& properties = parameterProperties[i];
    IControl* control;
    IBitmap* graphic;
    switch (i) {
      // Switches:
    case mOsc1Waveform:
    case mOsc2Waveform:
    case mLFOWaveform:
      graphic = &waveformBitmap;
      control = new ISwitchControl(this, properties.x, properties.y, i, graphic);
      break;
    case mFilterMode:
      graphic = &filterModeBitmap;
      control = new ISwitchControl(this, properties.x, properties.y, i, graphic);
      break;
      // Knobs:
    default:
      graphic = &knobBitmap;
      control = new IKnobMultiControl(this, properties.x, properties.y, i, graphic);
      break;
    }
    pGraphics->AttachControl(control);
  }
  AttachGraphics(pGraphics);
}