#include "Node.h"

Define_Module(Node);

void Node::initialize()
{
    if (strcmp(getName(), "hub") != 0) {
        double interval = exponential(1 / par("lambda").doubleValue());
        scheduleAt(simTime() + interval, new cMessage(""));
    }
}

void Node::handleMessage(cMessage *msg)
{
    if (strcmp(getName(), "hub") == 0) {
        int dest, src;
        src = atoi(msg->getName());
        do {
            dest = uniform(0, gateSize("outs"));
        } while(dest == src);

        std::stringstream ss;
        ss << dest;
        EV << "Sending to node #"<< ss.str() <<" from " << getName() << "\n";
        delete msg;
        msg = new cMessage(ss.str().c_str());
        send(msg, "outs", dest);
    }
    else {
        if (msg->isSelfMessage()) {
            delete msg;
            msg = new cMessage(std::to_string(getIndex()).c_str());
            send(msg, "outs", 0);
            double interval = exponential(1 / par("lambda").doubleValue());
            EV << ". Scheduled a new packet after " << interval << "s";
            scheduleAt(simTime() + interval, new cMessage(""));
        }
        else {
            if (atoi(msg->getName()) == getIndex())
                bubble("Message received");
            else
                bubble("Wrong destination");
            delete msg;
        }
    }
}




