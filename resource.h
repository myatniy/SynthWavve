#define PLUG_MFR "IlyaVarabei"
#define PLUG_NAME "Synthwavve"

#define PLUG_CLASS_NAME SynthWavve

#define BUNDLE_MFR "IlyaVarabei"
#define BUNDLE_NAME "Synthwavve"

#define PLUG_ENTRY SynthWavve_Entry
//#define PLUG_FACTORY SynthWavve_Factory
#define PLUG_VIEW_ENTRY SynthWavve_ViewEntry

#define PLUG_ENTRY_STR "SynthWavve_Entry"
#define PLUG_VIEW_ENTRY_STR "SynthWavve_ViewEntry"

#define VIEW_CLASS SynthWavve_View
#define VIEW_CLASS_STR "SynthWavve_View"

// Format        0xMAJR.MN.BG - in HEX! so version 10.1.5 would be 0x000A0105
#define PLUG_VER 0x00010000
#define VST3_VER_STR "1.0.0"

#define PLUG_UNIQUE_ID 'T999'
// make sure this is not the same as BUNDLE_MFR
#define PLUG_MFR_ID 'SYNT'

// ProTools stuff

#if (defined(AAX_API) || defined(RTAS_API)) && !defined(_PIDS_)
  #define _PIDS_
  const int PLUG_TYPE_IDS[2] = {'EFN1', 'EFN2'};
  const int PLUG_TYPE_IDS_AS[2] = {'EFA1', 'EFA2'}; // AudioSuite
#endif

#define PLUG_MFR_PT "IlyaVarabei\nIlyaVarabei\nAcme"
#define PLUG_NAME_PT "SynthWavve\nIPEF"
#define PLUG_TYPE_PT "Effect"
#define PLUG_DOES_AUDIOSUITE 1

/* PLUG_TYPE_PT can be "None", "EQ", "Dynamics", "PitchShift", "Reverb", "Delay", "Modulation", 
"Harmonic" "NoiseReduction" "Dither" "SoundField" "Effect" 
instrument determined by PLUG _IS _INST
*/

// #define PLUG_CHANNEL_IO "1-1 2-2"
#if (defined(AAX_API) || defined(RTAS_API)) 
#define PLUG_CHANNEL_IO "1-1 2-2"
#else
// no audio input. mono or stereo output
#define PLUG_CHANNEL_IO "0-1 0-2"
#endif

#define PLUG_LATENCY 0
#define PLUG_IS_INST 1

// if this is 0 RTAS can't get tempo info
#define PLUG_DOES_MIDI 1

#define PLUG_DOES_STATE_CHUNKS 0

// Unique IDs for each image resource.
#define BG_ID 101
#define WHITE_KEY_ID 102
#define BLACK_KEY_ID 103
#define WAVEFORM_ID 104
#define KNOB_ID 105
#define FILTER_ID 106

// Image resource locations for this plug.
#define BG_FN "resources/img/bg.png"
#define WHITE_KEY_FN "resources/img/whitekey.png"
#define BLACK_KEY_FN "resources/img/blackkey.png"
#define WAVEFORM_FN "resources/img/waveform.png"
#define KNOB_FN "resources/img/knob.png"
#define KNOB_SMALL_FN "resources/img/knob_s.png"
#define FILTER_FN "resources/img/filter.png"

// GUI default dimensions
#define GUI_WIDTH 869
#define GUI_HEIGHT 329

// on MSVC, you must define SA_API in the resource editor preprocessor macros as well as the c++ ones
#if defined(SA_API)
#include "app_wrapper/app_resource.h"
#endif

// vst3 stuff
#define MFR_URL ""
#define MFR_EMAIL "brokencoffeemachine@protonmail.com"
#define EFFECT_TYPE_VST3 "Fx"

/* "Fx|Analyzer"", "Fx|Delay", "Fx|Distortion", "Fx|Dynamics", "Fx|EQ", "Fx|Filter",
"Fx", "Fx|Instrument", "Fx|InstrumentExternal", "Fx|Spatial", "Fx|Generator",
"Fx|Mastering", "Fx|Modulation", "Fx|PitchShift", "Fx|Restoration", "Fx|Reverb",
"Fx|Surround", "Fx|Tools", "Instrument", "Instrument|Drum", "Instrument|Sampler",
"Instrument|Synth", "Instrument|Synth|Sampler", "Instrument|External", "Spatial",
"Spatial|Fx", "OnlyRT", "OnlyOfflineProcess", "Mono", "Stereo",
"Surround"
*/
