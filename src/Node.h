#ifndef __STAR_NODE_H_
#define __STAR_NODE_H_

#include <omnetpp.h>
#include "UserMsg_m.h"

using namespace omnetpp;
using namespace std;

class Node : public cSimpleModule
{
private:
  vector<string> files;           // Contains the files in messages directory
  vector<pair<int, int>> table;   // Contains the pairs of nodes that will communicate
  vector<string> lines;           // Contains file lines to be sent
  vector<string> sendBuffer;      // Keep track of sent messages that yet not have ack
  int lastAck;                    // Contains the last acknowledge received
  int receiver;                   // Receiver index
  int frameExpected;              // Next frame to be received (lower edge for receiver widnow)
  int tooFar;                     // Upper edge of receiver window + 1
  bool noNak;                     // No negative ack
  UserMsg_Base *ackTimeout;       // Time out for ack (receiver)
  vector<UserMsg_Base *> timeout; // Ack array for frames in send buffer (sender)
  vector<bool> arrived;           // Keep track of arrived frames
  vector<string> receiveBuffer;   // Store frames to be sent to network layer
  int ackExpected;                // Lower edge for sender window
  int nextFrameToSend;            // Upper edge for sender window + 1
  int nBuffered;                  // Number of buffered backets in send buffer
  int oldestFrame;                // Oldest frame in the send buffer
  int numEndInHub;                // Shows the count of nodes that want to end transimission (transimssion is ended if numEndInHub = 2)
  bool stopSendingData;           // Stop sending data from node
  int numSelfMsg;                 // Number of self msg sent by the node
  simtime_t lastSend;             // The last message scheduled to be sent
  int lastEndReceiver;          // Carries the id of the last receiver to receive an end signal

protected:
  virtual void initialize();
  virtual void handleMessage(cMessage *msg);
  void getMessageFiles();
  void createTable(int filesCount);
  void notifyNodes(pair<int, int>);
  void readFile(string fileName);
  void startAckTimer(); // Receiver
  void stopAckTimer();  // Receiver
  void sendFrame(int frameType, int frameNum, int frameExp);
  void startTimer(int index, int frameNumber); // Sender
  void stopTimer(int index);                   // Sender
};

#endif
