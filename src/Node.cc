#include "Node.h"
#include "UserMsg_m.h"
#include <dirent.h>
#include <string>
#include <algorithm>
#include <random>
#include <fstream>

const char *messages_directory = "./../src/messages"; // Messages directory
int interval = 5;                                     // Message interval
int maxSeq = 7; // Max sequence
int windowSize = (maxSeq + 1) / 2; // Windows size
float timeLimit = 1.125;
typedef enum {recieveData, recieveTable, sendMessage, err, timeout, ackTimeout, end} eventType;

Define_Module(Node);

// Returns all file names in messages directory
vector<char *> Node::getMessageFiles()
{
    // Reading files in the "Msg files" directory and save them in files vector
    DIR *dir;
    struct dirent *diread;
    vector<char *> files; // Will contain the message files form message directory
    if ((dir = opendir(messages_directory)) != nullptr)
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
    while (filesCount > 0)
    {
        if (src >= getParentModule()->par("n").intValue())
            src = 0;
        do
        {
            dest = uniform(0, getParentModule()->par("n").intValue());
        } while (dest == src);
        table.push_back(pair<int, int>(src++, dest));
        filesCount--;
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
        msg = new UserMsg_Base("");
        msg->setType(0); // Message type is 0 for first message from hub to nodes
        int time = interval * i + margin;
        string s = to_string(table[i].second) + "-" + files[0] + "-" + to_string(time);
        msg->setPayload(s.c_str()); // Pay load will be in type ("dest-file_name-time")
        send(msg, "outs", table[i].first);
        files.erase(files.begin());
        i++;
    }
}

// Returns all lines of the file in a sting vector
vector<string> Node::readFile(string fileName)
{
    vector<string> lines;
    fstream newfile;
    newfile.open(messages_directory + fileName, ios::in); // Open a file to perform read operation using file object
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
}

void Node::handleMessage(cMessage *cmsg)
{
    UserMsg_Base *msg = check_and_cast<UserMsg_Base *>(cmsg);
    // Hub
    if (strcmp(getName(), "hub") == 0)
    {
        int dest, src;
        vector<string> stringArray;
        stringstream s(msg->getName());
        string value;
        while (getline(s, value, '-'))
        {
            stringArray.push_back(value);
        }
        src = atoi(stringArray[0].c_str());
        dest = atoi(stringArray[1].c_str());
        std::stringstream ss;
        ss << dest;
        EV << "IN HUB: Sending from node #" << to_string(src) << " to " << to_string(dest) << ", Message= " << msg->getPayload() << "\n";
        bubble(msg->getPayload());
        send(msg, "outs", dest);
    }
    // For all nodes except hub
    else
    {
        vector<string> lines;
        vector <pair <int , clock_t >> sentAckTimeout; // Keep track of sent messages that yet not have ack
        vector <int> recievedLines; // Keep track of received lines
        clock_t receiveTimeout ; // Keep track of time that i dont receive anything
        int sf,sn,sl; // First, Now, Last
        while (true){
            if( !msg->SelfMessage() && msg->Type == 0 ) // Receive first time from hub
                eventType = recieveTable;
            else if (!msg->SelfMessage() && msg->Type == 1) // Receive data
                eventType = recieveData;
            else if (timeout.size() != 0 && float(clock () - timeout[0].second)  >= timeLimit) // Check on first line you send and yet didn't receive ack
            {
                eventType = ackTimeout;
            }
            else if (false) // Hamming == true
            {
                eventType = err;
            }
            else if (float(clock ()-timeout)){
                eventType = timeout;
            }
            else if (sn < lines.size())
                eventType = sendMessage;
            else
                eventType = end;
            switch (eventType){

            }
        }



        if (msg->isSelfMessage())// Sender
        {
            vector<string> stringArray;
            stringstream s(msg->getPayload());
            string value;
            while (getline(s, value, '-'))
            {
                stringArray.push_back(value);
            }
            vector<string> lines = readFile(stringArray[1]);
            delete msg;

            for (int i = 0; i < lines.size(); i++)
            {
                string srcDest = to_string(getIndex()) + '-' + stringArray[0];// (src , destination)
                msg = new UserMsg_Base(srcDest.c_str());
                msg->setPayload(lines[i].c_str());
                send(msg, "outs", 0);
            }
            EV << "Node : " << to_string(getIndex()) << " send file : " << stringArray[1] << " to Node : " << stringArray[0] << "\n";
        }
        else // Receiver
        {
            if (msg->getType() == 0)
            {
                vector<string> stringArray;
                stringstream s(msg->getPayload());
                string value;
                while (getline(s, value, '-'))
                {
                    stringArray.push_back(value);
                }
                string destFileName = stringArray[0] + '-' + stringArray[1]; // (dest,filename)
                scheduleAt(atoi(stringArray[2].c_str()), new UserMsg_Base(destFileName.c_str()));
            }
            if (atoi(msg->getName()) == getIndex())
                bubble("Message received");
            else
                bubble("Wrong destination");
            delete msg;
        }
    }
}
