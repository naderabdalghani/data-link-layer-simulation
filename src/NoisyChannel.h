#ifndef NOISYCHANNEL_H_
#define NOISYCHANNEL_H_

#include <omnetpp/cchannel.h>
using namespace omnetpp;


class NoisyChannel : public cChannel
{
public:
    explicit NoisyChannel(const char *name=nullptr);
    static NoisyChannel *create(const char *name);
    virtual void processMessage(cMessage *msg, simtime_t t, result_t& result) override;
    virtual double getNominalDatarate() const override;
    virtual bool isTransmissionChannel() const override;
    virtual simtime_t calculateDuration(cMessage *msg) const override;
    virtual simtime_t getTransmissionFinishTime() const override;
    virtual bool isBusy() const override;
    virtual void forceTransmissionFinishTime(simtime_t t) override;
};

#endif
