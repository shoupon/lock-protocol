//
//  ya_channel.h
//  lock-protocol
//
//  Created by Shou-pon Lin on 2/11/15
//  Copyright (c) 2015 Shou-pon Lin. All rights reserved.
//

#ifndef LOCKPROTOCOL_CHANNEL_H
#define LOCKPROTOCOL_CHANNEL_H

#include <cassert>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <sstream>
using namespace std;

#include "../prob_verify/statemachine.h"
#include "../prob_verify/sync.h"
#include "identifiers.h"
#include "lock.h"

class ChannelSnapshot;

class Channel: public StateMachine {
public:
  Channel(int from, const vector<int>& tos);
  Channel(int from, int to1, int to2);
  ~Channel() {}
  int transit(MessageTuple* inMsg, vector<MessageTuple*>& outMsgs,
              bool& high_prob, int startIdx = 0);
  int nullInputTrans(vector<MessageTuple*>& outMsgs,
                     bool& high_prob, int startIdx = 0);
  void restore(const StateSnapshot *snapshot);
  StateSnapshot* curState();
  void reset();

private:
  void initialize(const vector<int>& tos);

  unique_ptr<LockMessage> msg_in_transit_;
  vector<bool> msg_exist_;
  int deadline_;

  int src_lock_id_;
  vector<int> dest_lock_ids_;
  string channel_name_;
  string src_name_;
  vector<string> dest_names_;
  int num_combinations_;
};

class ChannelSnapshot: public StateSnapshot {
  friend class Channel;
public:
  ChannelSnapshot(const vector<bool>& exist, int deadline)
      : ss_msg_(nullptr), ss_exist_(exist), ss_deadline_(deadline) {}
  ChannelSnapshot(const LockMessage* site_msg, const vector<bool>& exist,
                  int deadline)
      : ss_msg_(new LockMessage(*site_msg)), ss_deadline_(deadline) {
    ss_exist_ = exist;
  }
  ~ChannelSnapshot() {}
  int curStateId() const;
  string toString();
  int toInt();
  StateSnapshot* clone() const;
  bool match(StateSnapshot* other);

protected:
  unique_ptr<LockMessage> ss_msg_;
  vector<bool> ss_exist_;
  int ss_deadline_;
};

#endif // LOCKPROTOCOL_CHANNEL_H
