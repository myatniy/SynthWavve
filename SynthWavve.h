#include "IPlug_include_in_plug_hdr.h"
#include "MIDIReceiver.h"
#include "VoiceManager.h"

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

  private:
    //double mFrequency;
    VoiceManager voiceManager;
    void CreatePresets();
    MIDIReceiver mMIDIReceiver;
    IControl* mVirtualKeyboard;
    void processVirtualKeyboard();

    void CreateParams();
    void CreateGraphics();
};
