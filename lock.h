//
//  lock.h
//  Prob_verify
//
//  Created by Shou-pon Lin on 8/24/12.
//  Copyright (c) 2012 Shou-pon Lin. All rights reserved.
//

#ifndef LOCK_H
#define LOCK_H

#include "../prob_verify/pverify.h"
#include "../prob_verify/statemachine.h"
#include "../prob_verify/sync.h"
#include "clock.h"
#include "identifiers.h"

class LockMessage;

class Lock : public StateMachine {
  static int num_locks_;
  static int clock_id_;
public:
  // Constructor.
  // k: Identifier of the lock
  Lock(int k);
  Lock(int k, int front, int back);
  
  int transit(MessageTuple* inMsg, vector<MessageTuple*>& outMsgs,
                      bool& high_prob, int startIdx = 0);
  int nullInputTrans(vector<MessageTuple*>& outMsgs,
                             bool& high_prob, int startIdx = 0);
  void restore(const StateSnapshot* snapshot) ;
  StateSnapshot* curState() ;
  void reset() ;

  string getName() const { return name_; }
  static void setNumLocks(int n_locks) { num_locks_ = n_locks; }
  int determineSession(LockMessage* in_msg);
  
private:
  void initialize();

  MessageTuple* createResponse(MessageTuple* in_msg, const string& msg,
                               int from, int to, int session);
  MessageTuple* createTiming(MessageTuple* in_msg, const string& msg,
                             int session, int my_id);

  const int lock_id_;
  const bool active_;
  const int front_;
  const int back_;
  int master_;

  string name_;
  unordered_map<int, string> channel_names_;
  unordered_map<int, int> channel_mac_ids_;
};

class LockMessage : public MessageTuple
{
public:
  LockMessage(int src, int dest, int srcMsg, int destMsg, int subject,
              int master, int session)
      : MessageTuple(src, dest, srcMsg, destMsg, subject),
        master_(master), session_(session) {}
  LockMessage(const LockMessage& msg)
      : MessageTuple(msg._src, msg._dest,
                     msg._srcMsg, msg._destMsg, msg._subject),
        master_(msg.master_), session_(msg.session_) {}
  LockMessage(int src, int dest, int srcMsg, int destMsg, int subject,
              const LockMessage& msg)
      : MessageTuple(src, dest, srcMsg, destMsg, subject),
        master_(msg.master_), session_(msg.session_) {}
  ~LockMessage() {}
  size_t numParams() { return 2; }
  int getParam(size_t arg) { return arg? session_: master_; }
  size_t getBytes() const { return MessageTuple::getBytes() + sizeof(int) * 2; }
  
  string toString() const;
  LockMessage* clone() const;

  int getCreator() const { return master_; }
  int getSession() const { return session_; }
private:    
  const int master_;
  const int session_;
};

class LockSnapshot : public StateSnapshot {
public:
  friend class Lock;

  LockSnapshot(): ss_state_(0), ss_master_(-1) {}
  LockSnapshot(int state, int master)
      : ss_state_(state), ss_master_(master) {}
  ~LockSnapshot() {} ;
  int curStateId() const { return ss_state_; }
  size_t getBytes() const { return sizeof(int) * 2; }
  // Returns the name of current state as specified in the input file
  string toString() const;
  // Cast the Snapshot into a integer. Used in HashTable
  int toInt() { return ((ss_master_ << 16) + ss_state_); }
  LockSnapshot* clone() const { return new LockSnapshot(ss_state_,
                                                        ss_master_); }
  bool match(StateSnapshot* other);
    
private:
  int ss_state_;
  int ss_master_;
};



#endif
