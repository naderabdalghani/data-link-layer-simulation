#ifndef NOISYCHANNEL_H_
#define NOISYCHANNEL_H_

#include <omnetpp/cchannel.h>
#include "UserMsg_m.h"
using namespace omnetpp;


class NoisyChannel : public cChannel
{
private:
    simtime_t delayMin;
    simtime_t delayMax;
    simtime_t txFinishTime;
    double delayProbability;
    double discardingProbability;
    double modificationProbability;

    void checkState() const  {
        if (!parametersFinalized()) throw cRuntimeError(this, E_PARAMSNOTREADY);
    }
protected:
    void rereadPars();
    virtual void handleParameterChange(const char *parname) override;
public:
    static int numberOfDiscardedMsgs;
    explicit NoisyChannel(const char *name=nullptr);
    virtual void processMessage(cMessage *msg, simtime_t t, result_t& result) override;
    virtual bool isTransmissionChannel() const override;
    virtual simtime_t getTransmissionFinishTime() const override;
    virtual void forceTransmissionFinishTime(simtime_t t) override;
    virtual double getNominalDatarate() const override;
    virtual simtime_t calculateDuration(cMessage *msg) const override;
};

#endif
