#ifndef NOISYCHANNEL_H_
#define NOISYCHANNEL_H_

#include <omnetpp/cchannel.h>
using namespace omnetpp;


class NoisyChannel : public cChannel
{
private:
    simtime_t delayMin;
    simtime_t delayMax;
    simtime_t txFinishTime;
    double delayProbability;
    double discardingProbability;

    void checkState() const  {
        if (!parametersFinalized()) throw cRuntimeError(this, E_PARAMSNOTREADY);
    }
protected:
    static simsignal_t channelBusySignal;
    static simsignal_t messageSentSignal;
    static simsignal_t messageDiscardedSignal;
    void rereadPars();
    virtual void handleParameterChange(const char *parname) override;
    virtual void finish() override;
public:
    explicit NoisyChannel(const char *name=nullptr);
    virtual void initialize() override;
    virtual void processMessage(cMessage *msg, simtime_t t, result_t& result) override;
    virtual bool isTransmissionChannel() const override;
    virtual simtime_t getTransmissionFinishTime() const override;
    virtual void forceTransmissionFinishTime(simtime_t t) override;
    virtual double getNominalDatarate() const override;
    virtual simtime_t calculateDuration(cMessage *msg) const override;
};

#endif
