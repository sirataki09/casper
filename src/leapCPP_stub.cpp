#include "leapCPP.h"

// Simulation playback reads recorded hand transforms from resource/recordings.
// These definitions preserve the upstream LeapCPP interface without loading the
// Ultraleap service or requiring a physical controller.
LeapCPP::LeapCPP(bool pollMode, bool, _eLeapTrackingMode, bool verbose)
    : m_poll(pollMode), m_verbose(verbose)
{
    IsConnected = false;
}

LeapCPP::~LeapCPP() = default;

void LeapCPP::OpenConnection() {}
void LeapCPP::kill() {}
void LeapCPP::setPollMode(bool pollMode) { m_poll = pollMode; }
void LeapCPP::setImageMode(bool) {}
void LeapCPP::setTrackingMode(_eLeapTrackingMode) {}
LEAP_DEVICE_INFO *LeapCPP::GetDeviceProperties() { return nullptr; }
void LeapCPP::setDevice(const LEAP_DEVICE_INFO *) {}
void LeapCPP::setFrame(const LEAP_TRACKING_EVENT *) {}
void LeapCPP::setImage(const LEAP_IMAGE_EVENT *) {}
LEAP_TRACKING_EVENT *LeapCPP::getFrame() { return nullptr; }

bool LeapCPP::getImage(std::vector<uint8_t> &, std::vector<uint8_t> &, uint32_t &width, uint32_t &height)
{
    width = 0;
    height = 0;
    return false;
}

bool LeapCPP::getDistortion(std::vector<float> &, std::vector<float> &, uint32_t &width, uint32_t &height)
{
    width = 0;
    height = 0;
    return false;
}

std::vector<float> LeapCPP::getIndexTip() { return {}; }
