#include "Node.h"
#include <dirent.h>
#include <string>
#include <algorithm>
#include <random>
#include <fstream>

const char *messagesDirectory = "./../src/messages/"; // Messages directory
int maxSeq = 7;                                       // Max sequence
int windowSize = (maxSeq + 1) / 2;                    // Windows size
float maxTimeLimitAck = 2;                            // Time limit for receiving ack
float maxTimeLimitNak = 3;                            // Time limit for receiving nak
int margin = 1;                                       // Margin time
typedef enum
{
    tableEnum,      // From hub to nodes in initialize
    dataEnum,       // Data from another node
    nakEnum,        // Nack signal from receiver
    ackTimeOutEnum, // Ack time (receiver)
    timeOutEnum,    // Time out (sender)
    selfMsgEnum,    // From node to itself
    ackEnum,        // Ack enum
    endEnum         // End sending
} receivedMsgType;

Define_Module(Node);

void inc(int &a)
{
    a = a + 1 <= maxSeq ? a + 1 : 0;
}
bool between(int a, int b, int c)
{
    return ((a <= b) && (b < c)) || ((c < a) && (a <= b)) || ((b < c) && (c < a));
}

void Node::startAckTimer()
{
    stopAckTimer();
    EV << "Node #" << getIndex() << " : has started ACK timer \n";
    ackTimeout = new UserMsg_Base("");
    ackTimeout->setType(ackTimeOutEnum);
    scheduleAt(simTime() + maxTimeLimitAck, ackTimeout);
}

void Node::stopAckTimer()
{
    if (ackTimeout)
    {
        EV << "Node #" << getIndex() << " : has stopped ACK timer \n";
        cancelAndDelete(ackTimeout);
        ackTimeout = NULL;
    }
}

void Node::startTimer(int index, int frameNumber)
{
    stopTimer(index);
    EV << "Node #" << getIndex() << " : has started frame timer with SQN =  " << frameNumber << "\n";
    timeout[index] = new UserMsg_Base("");
    timeout[index]->setType(timeOutEnum);
    timeout[index]->setLine_nr(frameNumber);
    scheduleAt(simTime() + maxTimeLimitNak, timeout[index]);
}

void Node::stopTimer(int index)
{
    if (timeout[index])
    {
        EV << "Node #" << getIndex() << " : has stopped frame timer with SQN =  " << timeout[index]->getLine_nr() << "\n";
        cancelAndDelete(timeout[index]);
        timeout[index] = NULL;
    }
}

void Node::sendFrame(int frameType, int frameNum, int frameExp)
{
    UserMsg_Base *newMsg = new UserMsg_Base(to_string(receiver).c_str());
    newMsg->setType(frameType);
    newMsg->setLine_nr(frameNum);
    newMsg->setLine_expected((frameExp + maxSeq) % (maxSeq + 1));
    if (frameType == dataEnum)
    {
        newMsg->setPayload(sendBuffer[frameNum % windowSize].c_str());
        EV << "Node #" << getIndex() << " send \"" << newMsg->getPayload() << "\" with SQN = " << newMsg->getLine_nr() << " and ACK = " << newMsg->getLine_expected() << " \n";
    }
    if (frameType == nakEnum)
    {
        noNak = false;
        EV << "Node #" << getIndex() << " sent NAK with ACK = " << newMsg->getLine_expected() << " \n";
    }
    if (frameType == dataEnum && !stopSendingData)
    {
        startTimer(frameNum % windowSize, frameNum);
    }
    if (frameType == ackEnum)
    {
        EV << "Node #" << getIndex() << " sent ACK = " << newMsg->getLine_expected() << " \n";
    }
    if(frameType != endEnum)
        stopAckTimer();
    send(newMsg, "outs", 0);
}

// Returns all file names in messages directory
void Node::getMessageFiles()
{
    // Reading files in the "messages" directory and save them in files vector
    DIR *dir;
    struct dirent *diread;
    if ((dir = opendir(messagesDirectory)) != nullptr)
    {
        while ((diread = readdir(dir)) != nullptr)
        {
            int size = std::string(diread->d_name).size();
            std::string s = diread->d_name;
            if (s.size() > 3)
                s = s.substr(size - 3);
            if (s.compare("txt") == 0)
                files.push_back(diread->d_name);
        }
        closedir(dir);
    }
}

// Returns a table of 2 nodes that will send to each others first will send and second will receive
void Node::createTable(int filesCount)
{
    int src = 0, dest;
    int halfFilesCount = filesCount / 2;
    while (halfFilesCount > 0)
    {
        if (src >= getParentModule()->par("n").intValue())
            src = 0;
        do
        {
            dest = uniform(0, getParentModule()->par("n").intValue());
        } while (dest == src);
        table.push_back(pair<int, int>(src++, dest));
        halfFilesCount--;
    }
};

// Send msg to all nodes to tell them the file they will send from and node it will send to
void Node::notifyNodes(pair<int, int> nodes)
{
    UserMsg_Base *msg;
    if (!files.empty())
    {
        auto rng = default_random_engine{};
        shuffle(begin(files), end(files), rng);
        string s = to_string(nodes.second) + "-" + files[0];
        msg = new UserMsg_Base(s.c_str()); // Name will be in type ("dest-file_name")
        msg->setType(tableEnum);           // Message type is 0 for first message from hub to nodes
        msg->setPayload(s.c_str());
        send(msg, "outs", nodes.first);
        files.erase(files.begin());
        if (!files.empty())
        {
            s = to_string(nodes.first) + "-" + files[0];
            msg = new UserMsg_Base(s.c_str()); // Name will be in type ("dest-file_name")
            msg->setType(tableEnum);
            msg->setPayload(s.c_str());
            send(msg, "outs", nodes.second);
            files.erase(files.begin());
        }
        else // Handle odd number of files
        {
            numEndInHub = 1;
            msg = new UserMsg_Base("");
            msg->setType(endEnum);
            send(msg, "outs", nodes.first);
        }
    }
}

// Returns all lines of the file in a sting vector
void Node::readFile(string fileName)
{
    fstream newfile;
    string openFile = string(messagesDirectory) + fileName;
    newfile.open(openFile, ios::in); // Open a file to perform read operation using file object
    if (newfile.is_open())
    { // Checking whether the file is open
        string tp;
        while (getline(newfile, tp))
        {                        // Read data from file object and put it into string.
            lines.push_back(tp); // Push all files
        }
        newfile.close(); // Close the file object.
    }
}

void Node::initialize()
{
    // Check if this node is hub
    if (strcmp(getName(), "hub") == 0)
    {
        // Get file messages from directory messages
        getMessageFiles();
        createTable(files.size());
        numEndInHub = 0;
        notifyNodes(table[0]); // Notify first pairs to talk
        table.erase(table.begin());
    }
}

void Node::handleMessage(cMessage *cmsg)
{
    UserMsg_Base *msg = check_and_cast<UserMsg_Base *>(cmsg);
    // Hub
    if (strcmp(getName(), "hub") == 0)
    {
        if (msg->getType() != endEnum)
        {
            // messageName -> receiver, messagePayload -> line,
            bubble(msg->getPayload());
            send(msg, "outs", atoi(msg->getName()));
        }
        else
        {
            EV << "One node ended\n";
            numEndInHub++;
            send(msg, "outs", atoi(msg->getName()));
            if (numEndInHub >= 2)
            {
                EV << "End Pair";
                if (!table.empty())
                {
                    numEndInHub = 0;
                    notifyNodes(table[0]);
                    table.erase(table.begin());
                }
                else
                {
                    EV << "Transmission done for all nodes!!!\n";
                }
            }
        }
    }
    // For all nodes except hub
    else
    {
        switch (msg->getType())
        {
        case tableEnum: // Receive (receiver and file to read from)
        {
            frameExpected = 0;
            ackExpected = 0;
            nextFrameToSend = 0;
            tooFar = windowSize;
            lastAck = 0;
            receiver = -1;
            noNak = true;
            nBuffered = 0;
            oldestFrame = maxSeq + 1;
            ackTimeout = NULL;
            stopSendingData = false;
            numSelfMsg = 1;
            arrived.clear();
            receiveBuffer.clear();
            sendBuffer.clear();
            timeout.clear();
            for (int i = 0; i < windowSize; i++)
            {
                arrived.push_back(false);
                receiveBuffer.push_back("");
                sendBuffer.push_back("");
                timeout.push_back(NULL);
            }
            vector<string> stringArray;
            string name = msg->getName();
            stringstream ss;
            ss << name;
            string value;
            while (getline(ss, value, '-'))
            {
                stringArray.push_back(value);
            }
            receiver = atoi(stringArray[0].c_str()); // Get the receiver index
            if (stringArray.size() > 1)
            {
                EV << "Node #" << getIndex() << " will send file \"" << stringArray[1] << "\" to node #" << stringArray[0] << " \n";
                readFile(stringArray[1].c_str()); // Get lines from file
                UserMsg_Base *selfMsg = new UserMsg_Base();
                selfMsg->setType(selfMsgEnum);           // Set type to be self message
                scheduleAt(simTime() + margin, selfMsg); // Send self msg at the time generated by hub + margin
            }
            else
            {
                sendFrame(endEnum, 0, 0);
            }
            break;
        }
        case selfMsgEnum: // Read first windwoSize lines from lines to send buffer and send them
        {
            nBuffered++;
            numSelfMsg--;
            sendBuffer[nextFrameToSend % windowSize] = lines[0];
            lines.erase(lines.begin());
            sendFrame(dataEnum, nextFrameToSend, frameExpected);
            inc(nextFrameToSend);
            break;
        }
        case dataEnum: // Receive message
        {
            EV << "Node #" << getIndex() << " received \"" << msg->getPayload() << "\" with SQN = " << msg->getLine_nr() << " and ACK = " << msg->getLine_expected() << " \n";
            if ((msg->getLine_nr() != frameExpected) && noNak) // If we received not expected frame then send Nak to expected frame
            {
                sendFrame(nakEnum, 0, frameExpected);
            }
            else // If we received the expected frame reset ackTimer
            {
                startAckTimer();
            }
            if (between(frameExpected, msg->getLine_nr(), tooFar) && arrived[msg->getLine_nr() % windowSize] == false)
            { // If we got what we expected move the receiver window
                arrived[msg->getLine_nr() % windowSize] = true;
                receiveBuffer[msg->getLine_nr() % windowSize] = msg->getPayload();
                while (arrived[frameExpected % windowSize])
                {
                    bubble(receiveBuffer[frameExpected % windowSize].c_str()); // Send to network layer
                    noNak = true;
                    arrived[frameExpected % windowSize] = false;
                    inc(frameExpected);
                    inc(tooFar);
                    startAckTimer();
                }
            }
            while (between(ackExpected, msg->getLine_expected(), nextFrameToSend)) // Move sender window if we got ack of the oldest element
            {
                nBuffered--;
                stopTimer(ackExpected % windowSize);
                inc(ackExpected);
            }
            break;
        }
        case nakEnum:
        {
            EV << "Node #" << getIndex() << " received NAK with ACK = "
               << msg->getLine_expected() << " \n";
            if (between(ackExpected, (msg->getLine_expected() + 1) % (maxSeq + 1), nextFrameToSend))
            {
                sendFrame(dataEnum, (msg->getLine_expected() + 1) % (maxSeq + 1), frameExpected); // Send frame that we received nak for
            }
            while (between(ackExpected, msg->getLine_expected(), nextFrameToSend))
            {
                nBuffered--;
                stopTimer(ackExpected % windowSize);
                inc(ackExpected);
            }
            break;
        }
        case timeOutEnum:
        {
            sendFrame(dataEnum, msg->getLine_nr(), frameExpected);
            break;
        }
        case ackTimeOutEnum:
        {
            sendFrame(ackEnum, 0, frameExpected);
            break;
        }
        case ackEnum:
        {
            EV << "Node #" << getIndex() << " received ACK with ACK = "
               << msg->getLine_expected() << " \n";
            while (between(ackExpected, msg->getLine_expected(), nextFrameToSend)) // Move sender window if we got ack of the oldest element
            {
                nBuffered--;
                stopTimer(ackExpected % windowSize);
                inc(ackExpected);
            }
            break;
        }
        case endEnum:
        {
            stopAckTimer();
            break;
        }
        default:
            break;
        }
        if (nBuffered < windowSize && msg->getType() != tableEnum && numSelfMsg < int(lines.size()) && lastSend != simTime())
        {
            lastSend = simTime();
            UserMsg_Base *newMsg = new UserMsg_Base("");
            newMsg->setType(selfMsgEnum);
            numSelfMsg++;
            scheduleAt(simTime() + 1, newMsg);
        }
        if (lines.empty() && nBuffered == 0 && !stopSendingData)
        {
            EV << "End, Node: " << getIndex() << endl;
            sendFrame(endEnum, 0, 0);
            stopSendingData = true;
        }
    }
}
