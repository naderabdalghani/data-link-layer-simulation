#include "Node.h"
#include "UserMsg_m.h"
#include <dirent.h>
#include <string>
using namespace std;

const char* messages_directory = "./../src/messages";

Define_Module(Node);
void Node::initialize()
{
    // Reading files in the "Msg files" directory and save them in files vector
    DIR *dir;
    struct dirent *diread;
    vector<char *> files;
    if ((dir = opendir(messages_directory)) != nullptr)
    {
        while ((diread = readdir(dir)) != nullptr)
        {
            int size = std::string(diread->d_name).size();
            std::string s = diread->d_name;
            if (s.size() > 3)
                s = s.substr(size - 3);
            //s = s[size - 3] + s[size - 2] + s[size - 1];
            if (s.compare("txt") == 0)
                files.push_back(diread->d_name);
        }
        closedir(dir);
    }
    for (auto file : files)
        std::cout << file << "\n ";

    if (strcmp(getName(), "hub") != 0)
    {
        double interval = exponential(1 / par("lambda").doubleValue());
        scheduleAt(simTime() + interval, new UserMsg_Base(""));
    }
}

void Node::handleMessage(cMessage *cmsg)
{
    UserMsg_Base *msg = check_and_cast<UserMsg_Base *>(cmsg);
    // Hub
    if (strcmp(getName(), "hub") == 0)
    {
        int dest, src;
        src = atoi(msg->getName());
        do
        {
            dest = uniform(0, gateSize("outs"));
        } while (dest == src);

        std::stringstream ss;
        ss << dest;
        EV << "Sending to node #" << ss.str() << " from " << getName() << "\n";
        delete msg;
        msg = new UserMsg_Base(ss.str().c_str());
        send(msg, "outs", dest);
    }
    // For all nodes except hub
    else
    {
        if (msg->isSelfMessage())
        {
            delete msg;
            msg = new UserMsg_Base(std::to_string(getIndex()).c_str());
            send(msg, "outs", 0);
            double interval = exponential(1 / par("lambda").doubleValue());
            EV << ". Scheduled a new packet after " << interval << "s";
            scheduleAt(simTime() + interval, new UserMsg_Base(""));
        }
        else
        {
            if (atoi(msg->getName()) == getIndex())
                bubble("Message received");
            else
                bubble("Wrong destination");
            delete msg;
        }
    }
}
