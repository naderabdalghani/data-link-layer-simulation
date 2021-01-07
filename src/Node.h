#ifndef __STAR_NODE_H_
#define __STAR_NODE_H_

#include <omnetpp.h>

using namespace omnetpp;

class Node : public cSimpleModule
{
protected:
  virtual void initialize();
  virtual void handleMessage(cMessage *msg);
};

#endif
