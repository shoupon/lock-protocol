//
//  clock.h
//  lock-protocol
//
//  Created by Shou-pon Lin on 2/12/15
//  Copyright (c) 2015 Shou-pon Lin. All rights reserved.
//

#ifndef LOCKPROTOCOL_CLOCK_H
#define LOCKPROTOCOL_CLOCK_H

#include <set>
using namespace std;

#include "../prob_verify/statemachine.h"
#include "identifiers.h"

class ClockMessage;

class Clock: public StateMachine {
public:
  Clock() {}
  ~Clock() {}
  int transit(MessageTuple* in_msg, vector<MessageTuple*>& out_msgs,
              bool& high_prob, int start_idx);
  int nullInputTrans(vector<MessageTuple*>& out_msgs,
                     bool& high_prob, int start_idx);
  void restore(const StateSnapshot* snapshot);
  StateSnapshot* curState();
  void reset();
private:
  vector<int> creators_;
  vector<set<int> > followers_;
};

class ClockSnapshot: public StateSnapshot {
  friend class Clock;
public:
  ClockSnapshot(const vector<int>& creators,
                const vector<set<int> >& followers);
  int curStateId() const { return ss_creators_.size(); }
  string toString();
  int toInt() { return curStateId(); }
  ClockSnapshot* clone() const { return new ClockSnapshot(*this); }
private:
  vector<int> ss_creators_;
  vector<set<int> > ss_followers_;
};

class ClockMessage: public MessageTuple {
public:
  ClockMessage(int src, int dest, int src_msg, int dest_msg, int subject,
               int master_id, int follower_id);
  string toString();
  ClockMessage* clone() const { return new ClockMessage(*this); }
  size_t numParams() { return 2; }
  int getParams(size_t arg) { return (!arg)? master_id_: follower_id_; }

  int getMaster() const { return master_id_; }
  int getFollwer() const { return follower_id_; }
private:
  int master_id_;
  int follower_id_;
};


#endif // LOCKPROTOCOL_CLOCK_H
