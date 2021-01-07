#include "NoisyChannel.h"

Define_Channel(NoisyChannel);

NoisyChannel::NoisyChannel(const char *name) : cChannel(name) {}

void NoisyChannel::processMessage(cMessage *msg, simtime_t t, result_t& result) {}

double NoisyChannel::getNominalDatarate() const {return 0;}

bool NoisyChannel::isTransmissionChannel() const {return false;}

simtime_t NoisyChannel::calculateDuration(cMessage *msg) const {return SIMTIME_ZERO;}

simtime_t NoisyChannel::getTransmissionFinishTime() const {return SIMTIME_ZERO;}

bool NoisyChannel::isBusy() const {return false;}

void NoisyChannel::forceTransmissionFinishTime(simtime_t t) {}
