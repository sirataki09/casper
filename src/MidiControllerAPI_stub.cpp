#include "MidiControllerAPI.h"

// The upstream MIDI implementation uses the Windows Multimedia API.  The
// simulation does not consume MIDI input, so macOS exposes the same API as
// harmless no-op functions to keep the rest of the application unchanged.
bool OpenMidiController() { return false; }
bool CloseMidiController() { return false; }
bool GetSliderValues(float *, int, int) { return false; }
bool GetKnobValues(float *, int, int) { return false; }
bool GetButtonStates(bool *, int, int) { return false; }
bool GetSystemButtonStates(bool *, int, int) { return false; }
float Get1SliderValue(int) { return -1.0f; }
float Get1KnobValue(int) { return -1.0f; }
bool Get1ButtonState(int) { return false; }
bool Get1SystemButtonState(const char *) { return false; }
bool SetButtonOnInterval(int) { return false; }
