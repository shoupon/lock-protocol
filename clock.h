//
//  clock.h
//  lock-protocol
//
//  Created by Shou-pon Lin on 2/12/15
//  Copyright (c) 2015 Shou-pon Lin. All rights reserved.
//

#ifndef LOCKPROTOCOL_CLOCK_H
#define LOCKPROTOCOL_CLOCK_H

#include <map>
#include <set>
using namespace std;

#include "../prob_verify/statemachine.h"
#include "identifiers.h"

class ClockMessage;

class Clock: public StateMachine {
public:
  Clock();
  ~Clock() {}
  int transit(MessageTuple* in_msg, vector<MessageTuple*>& out_msgs,
              bool& high_prob, int start_idx);
  int nullInputTrans(vector<MessageTuple*>& out_msgs,
                     bool& high_prob, int start_idx);
  void restore(const StateSnapshot* snapshot);
  StateSnapshot* curState();
  void reset();
  string getName() const { return "clock"; }

  void registerChannel(int channel_id) { channel_ids_.push_back(channel_id); }
  void clearChannel() { channel_ids_.clear(); }

  // obsolete
  int moreImminent(int a, int b);
private:
  void sendAlarm(map<int, set<int> >::const_iterator d,
                 vector<MessageTuple*>& out_msgs);

  vector<int> channel_ids_;
  map<int, set<int> > registrants_;
};

class ClockSnapshot: public StateSnapshot {
  friend class Clock;

  static const int kReadable;
  static const int kString;
public:
  ClockSnapshot(const map<int, set<int> >& registrants);
  int curStateId() const { return ss_registrants_.size(); }
  string toString() const;
  string toReadable() const;
  int toInt() { return curStateId(); }
  ClockSnapshot* clone() const { return new ClockSnapshot(*this); }
private:
  string stringify(int type) const;

  map<int, set<int> > ss_registrants_;
};

class ClockMessage: public MessageTuple {
public:
  ClockMessage(int src, int dest, int src_msg, int dest_msg, int subject,
               int master_id, int follower_id);
  string toString() const;
  ClockMessage* clone() const { return new ClockMessage(*this); }
  size_t numParams() { return 2; }
  int getParams(size_t arg) { return (!arg)? master_id_: follower_id_; }

  int getMaster() const { return master_id_; }
  int getFollwer() const { return follower_id_; }
private:
  int master_id_;   // deadline id (should coincide with lock master's mac_id
  int follower_id_; // registrant mac_id
};


#endif // LOCKPROTOCOL_CLOCK_H
