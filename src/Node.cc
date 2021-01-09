#include "Node.h"
#include <dirent.h>
#include <string>
#include <algorithm>
#include <random>
#include <fstream>

const char *messagesDirectory = "./../src/messages"; // Messages directory
int interval = 5;                                    // Message interval
int maxSeq = 7;                                      // Max sequence
int windowSize = (maxSeq + 1) / 2;                   // Windows size
float maxTimeLimit = 2;                              // Time limit for receiving ack
float epsilon = 0.0005;                              // Small time between the first send from the first node and first send from the second node

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
    cancelAndDelete(ackTimeout);
    ackTimeout = new UserMsg_Base("");
    ackTimeout->setType(ackTimeOutEnum);
    scheduleAt(simTime() + maxTimeLimit, ackTimeout);
}

void Node::stopAckTimer()
{
    cancelAndDelete(ackTimeout);
}

void Node::startTimer(int index)
{
    cancelAndDelete(timeout[index]);
    timeout[index] = new UserMsg_Base("");
    timeout[index]->setType(timeOutEnum);
    scheduleAt(simTime() + maxTimeLimit, timeout[index]);
}

void Node::stopTimer(int index)
{
    cancelAndDelete(timeout[index]);
}

void Node::sendFrame(int frameType, int frameNum, int frameExp)
{
    UserMsg_Base *newMsg = new UserMsg_Base(to_string(receiver).c_str());
    newMsg->setType(frameType);
    if (frameType == dataEnum)
    {
        newMsg->setPayload(sendBuffer[frameNum % windowSize].c_str());
    }
    newMsg->setLine_nr(frameNum);
    newMsg->setLine_expected((frameExp + maxSeq) % (maxSeq + 1));
    if (frameType == nakEnum)
    {
        noNak = false;
    }
    send(newMsg, "outs", 0);
    if (frameType == dataEnum)
    {
        startTimer(frameNum % windowSize);
    }
    stopAckTimer();
}

// Returns all file names in messages directory
vector<char *> Node::getMessageFiles()
{
    // Reading files in the "messages" directory and save them in files vector
    DIR *dir;
    struct dirent *diread;
    vector<char *> files; // Will contain the message files form message directory
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
    return files;
}

// Returns a table of 2 nodes that will send to each others first will send and second will receive
vector<pair<int, int>> Node::createTable(int filesCount)
{
    vector<pair<int, int>> table;
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
    return table;
};

// Send msg to all nodes to tell them the file they will send from and node it will send to
void Node::notifyNodes()
{
    UserMsg_Base *msg;
    int i = 0;
    int margin = 1; // Start the first send after 1 second
    while (!files.empty())
    {
        auto rng = default_random_engine{};
        shuffle(begin(files), end(files), rng);
        int time = interval * i + margin;
        string s = to_string(table[i].second) + "-" + files[0] + "-" + to_string(time);
        msg = new UserMsg_Base(s.c_str()); // Name will be in type ("dest-file_name-time")
        msg->setType(tableEnum); // Message type is 0 for first message from hub to nodes
        send(msg, "outs", table[i].first);
        files.erase(files.begin());
        if (!files.empty())
        {
            s = to_string(table[i].first) + "-" + files[0] + "-" + to_string(time + epsilon);
            msg = new UserMsg_Base(s.c_str()); // Name will be in type ("dest-file_name-time")
            msg->setType(tableEnum);
            send(msg, "outs", table[i].second);
            files.erase(files.begin());
        }
        i++;
    }
}

// Returns all lines of the file in a sting vector
vector<string> Node::readFile(string fileName)
{
    vector<string> lines;
    fstream newfile;
    newfile.open(messagesDirectory + fileName, ios::in); // Open a file to perform read operation using file object
    if (newfile.is_open())
    { // Checking whether the file is open
        string tp;
        while (getline(newfile, tp))
        {                        // Read data from file object and put it into string.
            lines.push_back(tp); // Push all files
        }
        newfile.close(); // Close the file object.
    }
    return lines;
}

void Node::initialize()
{
    // Check if this node is hub
    if (strcmp(getName(), "hub") == 0)
    {
        // Get file messages from directory messages
        files = getMessageFiles();
        table = createTable(files.size());
        notifyNodes();
    }
    else
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
        for (int i = 0; i < windowSize; i++)
        {
            arrived.push_back(false);
            receiveBuffer.push_back("");
            sendBuffer.push_back("");
            timeout.push_back(NULL);
        }
    }
}

void Node::handleMessage(cMessage *cmsg)
{
    UserMsg_Base *msg = check_and_cast<UserMsg_Base *>(cmsg);
    // Hub
    if (strcmp(getName(), "hub") == 0)
    {
        // messageName -> receiver, messagePayload -> line,
        bubble(msg->getPayload());
        send(msg, "outs", atoi(msg->getName()));
    }
    // For all nodes except hub
    else
    {
        switch (msg->getType())
        {
        case tableEnum: // Receive (receiver and file to read from)
        {
            vector<string> stringArray;
            stringstream s(msg->getName());
            string value;
            while (getline(s, value, '-'))
            {
                stringArray.push_back(value);
            }
            receiver = atoi(stringArray[0].c_str());  // Get the receiver index
            lines = readFile(stringArray[1].c_str()); // Get lines from file
            UserMsg_Base *selfMsg = new UserMsg_Base();
            selfMsg->setType(selfMsgEnum);                                 // Set type to be self message
            scheduleAt(simTime() + atoi(stringArray[2].c_str()), selfMsg); // Send self msg at the time generated by hub
            break;
        }
        case selfMsgEnum: // Read first windwoSize lines from lines to send buffer and send them
        {
            while (!lines.empty() && nBuffered <= windowSize)
            {
                nBuffered++;
                sendBuffer[nextFrameToSend % windowSize] = lines[0];
                lines.erase(lines.begin());
                sendFrame(dataEnum, nextFrameToSend, frameExpected);
                inc(nextFrameToSend);
            }
            break;
        }
        case dataEnum: // Receive message
        {
            if ((msg->getLine_nr() != frameExpected) && noNak)
            {
                UserMsg_Base *newMsg = new UserMsg_Base("");
                newMsg->setType(nakEnum);
                newMsg->setLine_expected(frameExpected); // TODO: Send frame expected in noisy channel
                sendFrame(nakEnum, 0, frameExpected);
            }
            else
            {
                startAckTimer();
            }
            if (between(frameExpected, msg->getLine_nr(), tooFar) && arrived[msg->getLine_nr() % windowSize] == false)
            {
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
            while (between(ackExpected, msg->getLine_expected(), nextFrameToSend))
            {
                nBuffered--;
                stopTimer(ackExpected % windowSize);
                inc(ackExpected);
                if (!lines.empty()) // If there is a message in the network layer then add it to send buffer
                {
                    nBuffered++;
                    // Add line to the send buffer and remove it from window lines vector (network layer)
                    sendBuffer[nextFrameToSend % windowSize] = lines[0];
                    lines.erase(lines.begin());
                    sendFrame(dataEnum, nextFrameToSend, frameExpected);
                    inc(nextFrameToSend);
                }
            }
            break;
        }
        case nakEnum:
        {
            if (between(ackExpected, (msg->getLine_expected() + 1) % (maxSeq + 1), nextFrameToSend))
            {
                sendFrame(dataEnum, (msg->getLine_expected() + 1) % (maxSeq + 1), frameExpected); // Send frame that we recevied nak for
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
            sendFrame(dataEnum, oldestFrame, frameExpected);
            break;
        }
        case ackTimeOutEnum:
        {
            sendFrame(ackEnum, 0, frameExpected);
            break;
        }
        default:
            break;
        }
        while (!lines.empty() && nBuffered <= windowSize)
        {
            nBuffered++;
            sendBuffer[nextFrameToSend % windowSize] = lines[0];
            lines.erase(lines.begin());
            sendFrame(dataEnum, nextFrameToSend, frameExpected);
            inc(nextFrameToSend);
        }
    }
}
