#include "NoisyChannel.h"

Define_Channel(NoisyChannel);

simsignal_t NoisyChannel::channelBusySignal;
simsignal_t NoisyChannel::messageSentSignal;
simsignal_t NoisyChannel::messageDiscardedSignal;

NoisyChannel::NoisyChannel(const char *name) : cChannel(name) {
    txFinishTime = -1;
}

void NoisyChannel::initialize()
{
    channelBusySignal = registerSignal("channelBusy");
    messageSentSignal = registerSignal("messageSent");
    messageDiscardedSignal = registerSignal("messageDiscarded");
}

void NoisyChannel::rereadPars()
{
    delayMin = par("delayMin").doubleValue();
    delayMax = par("delayMax").doubleValue();
    delayProbability = par("delayProbability").doubleValue();
    discardingProbability = par("discardingProbability").doubleValue();
    if (delayMin < 0 ||
        delayMax < 0 ||
        delayMin > delayMax ||
        delayProbability < 0 ||
        delayProbability > 100 ||
        discardingProbability < 0 ||
        discardingProbability > 100
        )
        throw cRuntimeError(this, "Invalid channel parameter(s)");
}

void NoisyChannel::handleParameterChange(const char *)
{
    rereadPars();
}

void NoisyChannel::finish()
{
    if (txFinishTime != -1 && mayHaveListeners(channelBusySignal)) {
        cTimestampedValue tmp(txFinishTime, 0L);
        emit(channelBusySignal, &tmp);
    }
}

void NoisyChannel::processMessage(cMessage *msg, simtime_t t, result_t& result) {
    double rand = uniform(0, 100);
    if (rand <= discardingProbability) {
        result.discard = true;
        cTimestampedValue tmp(t, msg);
        emit(messageDiscardedSignal, &tmp);
        EV << "Message discarded" << endl;
        return;
    }

    if (txFinishTime != -1 && mayHaveListeners(channelBusySignal)) {
        cTimestampedValue tmp(txFinishTime, 0L);
        emit(channelBusySignal, &tmp);
    }

    rand = uniform(0, 100);
    if (rand <= delayProbability) {
        result.delay = uniform(delayMin, delayMax);
        txFinishTime = t + result.delay;
        EV << "Message delayed with value = " << result.delay << endl;
    }
    if (mayHaveListeners(messageSentSignal)) {
        MessageSentSignalValue tmp(t, msg, &result);
        emit(messageSentSignal, &tmp);
    }
}

bool NoisyChannel::isTransmissionChannel() const { return true; }

simtime_t NoisyChannel::getTransmissionFinishTime() const { return txFinishTime; }

void NoisyChannel::forceTransmissionFinishTime(simtime_t t) { txFinishTime = t; }

double NoisyChannel::getNominalDatarate() const { return 0; }

simtime_t NoisyChannel::calculateDuration(cMessage *msg) const { return SIMTIME_ZERO; }
