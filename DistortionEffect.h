#include "IPlug_include_in_plug_hdr.h"
#include "Oscillator.h"
#include "MIDIReceiver.h"

class DistortionEffect : public IPlug
{
public:
  DistortionEffect(IPlugInstanceInfo instanceInfo);
  ~DistortionEffect();

  void Reset();
  void OnParamChange(int paramIdx);
  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);
  void ProcessMidiMsg(IMidiMsg* pMsg);
  inline int GetNumKeys() const { return mMIDIReceiver.getNumKeys(); };
  inline bool GetKeyStatus(int key) const { return mMIDIReceiver.getKeyStatus(key); };
  static const int virtualKeyboardMinimumNoteNumber = 48;
  int lastVirtualKeyboardNoteNumber;

private:
    double mFrequency;
    void CreatePresets();
    Oscillator mOscillator;
    MIDIReceiver mMIDIReceiver;
    IControl* mVirtualKeyboard;
    void processVirtualKeyboard();
};
