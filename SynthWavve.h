#include "IPlug_include_in_plug_hdr.h"
#include "Oscillator.h"
#include "MIDIReceiver.h"
#include "EnvelopeGenerator.h"
#include "Filter.h"

class SynthWavve : public IPlug {
  public:
    SynthWavve(IPlugInstanceInfo instanceInfo);
    ~SynthWavve();

    void Reset();
    void OnParamChange(int paramIdx);
    void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);
    void ProcessMidiMsg(IMidiMsg* pMsg); // MIDI
    inline int GetNumKeys() const { return mMIDIReceiver.getNumKeys(); }; // for GUI of keyboard
    inline bool GetKeyStatus(int key) const { return mMIDIReceiver.getKeyStatus(key); };
    static const int virtualKeyboardMinimumNoteNumber = 48;
    int lastVirtualKeyboardNoteNumber;

    inline void onBeganEnvelopeCycle() { mOscillator.setMuted(false); }
    inline void onFinishedEnvelopeCycle() { mOscillator.setMuted(true); }

  private:
    //double mFrequency;
    void CreatePresets();
    Oscillator mOscillator;
    MIDIReceiver mMIDIReceiver;
    IControl* mVirtualKeyboard;
    void processVirtualKeyboard();
    EnvelopeGenerator mEnvelopeGenerator;

    inline void onNoteOn(const int noteNumber, const int velocity) {
      mEnvelopeGenerator.enterStage(EnvelopeGenerator::ENVELOPE_STAGE_ATTACK);
      mFilterEnvelopeGenerator.enterStage(EnvelopeGenerator::ENVELOPE_STAGE_ATTACK);
    };
    inline void onNoteOff(const int noteNumber, const int velocity) {
      mEnvelopeGenerator.enterStage(EnvelopeGenerator::ENVELOPE_STAGE_RELEASE);
      mFilterEnvelopeGenerator.enterStage(EnvelopeGenerator::ENVELOPE_STAGE_RELEASE);
    };

    Filter mFilter;
    EnvelopeGenerator mFilterEnvelopeGenerator;
    double filterEnvelopeAmount;

    Oscillator mLFO;
    double LfoFilterModAmount;

    void CreateParams();
    void CreateGraphics();
};
