//
//  channel.h
//  lock-protocol
//
//  Created by Shou-pon Lin on 8/29/12.
//  Copyright (c) 2012 Shou-pon Lin. All rights reserved.
//

#ifndef CHANNEL_H
#define CHANNEL_H

#include <memory>
#include <string>
#include <sstream>
#include <vector>
using namespace std;

#include "../prob_verify/statemachine.h"
#include "../prob_verify/sync.h"
#include "identifiers.h"
#include "lock.h"
#include "competitor.h"

class ChannelSnapshot;

class Channel: public StateMachine
{
public:
  Channel(int from, int to);
  ~Channel() {}
  
  int transit(MessageTuple* in_msg, vector<MessageTuple*>& out_msgs,
              bool& high_prob, int startIdx);
  int nullInputTrans(vector<MessageTuple*>& out_msgs,
                     bool& high_prob, int startIdx);
  void restore(const StateSnapshot* snapshot);
  StateSnapshot* curState();
  // Reset the machine to initial state
  void reset();
  string getName() const { return channel_name_; }
protected:
  MessageTuple* createDelivery();
  const int origin_;
  const int destination_;
  shared_ptr<LockMessage> msg_in_transit_;
  string channel_name_;
  int destination_id_;
};

// Used for restore the state of a state machine back to a certain point. Should contain
// the state in which the machine was, the internal variables at that point
class ChannelSnapshot: public StateSnapshot {
  friend class Channel;
public:
  ChannelSnapshot() {}
  ChannelSnapshot(const ChannelSnapshot* ss);
  ChannelSnapshot(const LockMessage* msg): ss_msg_(new LockMessage(*msg)) {}
  int curStateId() const { return !ss_msg_? 0: 1; }
  // Returns the name of current state as specified in the input file
  string toString() ;
  int toInt() { return curStateId(); }
  ChannelSnapshot* clone() const { return new ChannelSnapshot(*this) ; }
  
private:
  shared_ptr<LockMessage> ss_msg_;
};

#endif

