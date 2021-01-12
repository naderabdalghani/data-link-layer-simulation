#include "NoisyChannel.h"
using namespace std;

Define_Channel(NoisyChannel);

int endEnum = 7;

NoisyChannel::NoisyChannel(const char *name) : cChannel(name) {
    txFinishTime = -1;
}

void NoisyChannel::rereadPars()
{
    delayMin = par("delayMin").doubleValue();
    delayMax = par("delayMax").doubleValue();
    delayProbability = par("delayProbability").doubleValue();
    discardingProbability = par("discardingProbability").doubleValue();
    modificationProbability = par("modificationProbability").doubleValue();
    if (delayMin < 0 ||
        delayMax < 0 ||
        delayMin > delayMax ||
        delayProbability < 0 ||
        delayProbability > 100 ||
        discardingProbability < 0 ||
        discardingProbability > 100 ||
        modificationProbability < 0 ||
        modificationProbability > 100
        )
        throw cRuntimeError(this, "Invalid channel parameter(s)");
}

void NoisyChannel::handleParameterChange(const char *)
{
    rereadPars();
}

void NoisyChannel::processMessage(cMessage *msg, simtime_t t, result_t& result) {
    UserMsg_Base *userMsg = check_and_cast<UserMsg_Base *>(msg);
    double rand = uniform(0, 100);
    if (rand <= discardingProbability && userMsg->getType() != endEnum) {
        result.discard = true;
        EV << "Message discarded" << endl;
        numberOfDiscardedMsgs++;
        return;
    }

    rand = uniform(0, 100);
    SimTime delay = 0;
    if (rand <= delayProbability) {
        delay = uniform(delayMin, delayMax);
        EV << "Message delayed with value = " << delay << endl;
    }
    rand = uniform(0, 100);
    if (rand <= modificationProbability) {
        string payload = userMsg->getPayload();
        int randIndex = uniform(0, payload.length());
        payload[randIndex] = payload[randIndex] == '0' ? '1' : '0';
        userMsg->setPayload(payload.c_str());
        msg = userMsg;
        EV << "Message modified" << endl;
    }
    if (txFinishTime > t && txFinishTime > t + delay) {
        result.delay = txFinishTime - t + delay;
        txFinishTime = txFinishTime + delay;
    }
    else {
        result.delay = delay;
        txFinishTime = t + delay;
    }
}

bool NoisyChannel::isBusy() const { return false; }

bool NoisyChannel::isTransmissionChannel() const { return false; }

simtime_t NoisyChannel::getTransmissionFinishTime() const { return txFinishTime; }

void NoisyChannel::forceTransmissionFinishTime(simtime_t t) { txFinishTime = t; }

double NoisyChannel::getNominalDatarate() const { return 0; }

simtime_t NoisyChannel::calculateDuration(cMessage *msg) const { return SIMTIME_ZERO; }
