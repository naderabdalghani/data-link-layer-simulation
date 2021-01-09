#ifndef __STAR_NODE_H_
#define __STAR_NODE_H_

#include <omnetpp.h>

using namespace omnetpp;
using namespace std;

class Node : public cSimpleModule
{
private:
  vector<char *> files;           // Contains the files in messages directory
  vector<pair<int, int>> table;   // Contains the pairs of nodes that will communicate
  vector<string> lines;           // Contains file lines to be sent
  vector<string> sendBuffer;      // Keep track of sent messages that yet not have ack
  int lastAck;                    // Contains the last acknowledge received
  int receiver;                   // Receiver index
  int lineToSendToSendBuffer;     // Next line to move to send buffer
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

protected:
  virtual void initialize();
  virtual void handleMessage(cMessage *msg);
  vector<char *> getMessageFiles();
  vector<pair<int, int>> createTable(int filesCount);
  void notifyNodes();
  vector<string> readFile(string fileName);
  void startAckTimer(); // Receiver
  void stopAckTimer();  // Receiver
  void sendFrame(int frameType, int frameNum, int frameExp, string payload);
  void startTimer(int index); // Sender
  void stopTimer(int index);  // Sender
};

#endif
