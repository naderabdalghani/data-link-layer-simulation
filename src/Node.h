#ifndef __STAR_NODE_H_
#define __STAR_NODE_H_

#include <omnetpp.h>

using namespace omnetpp;
using namespace std;

class Node : public cSimpleModule
{
private:
  vector<char *> files;
  vector<pair<int, int>> table;


protected:
  virtual void initialize();
  virtual void handleMessage(cMessage *msg);
  vector<char *> getMessageFiles();
  vector<pair<int, int>> createTable(int filesCount);
  void notifyNodes();
  vector<string> readFile(string fileName);
};

#endif
