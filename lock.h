//
//  lock.h
//  Prob_verify
//
//  Created by Shou-pon Lin on 8/24/12.
//  Copyright (c) 2012 Shou-pon Lin. All rights reserved.
//

#ifndef LOCK_H
#define LOCK_H

#include "../prob_verify/statemachine.h"
#include "../prob_verify/sync.h"
#include "lock_utils.h"
#include "clock.h"

#define GRANTED "granted"
#define DENIED "denied"
#define FAILED "failed"
#define REQUEST "request"
#define RELEASE "release"
#define SIGNUP "signup"
#define SIGNOFF "SIGNOFF"
#define ALARM "ALARM"

#define CHANNEL_NAME "channel"
#define CLOCK_NAME "clock"
#define LOCK_NAME "lock"

class LockMessage;

class Lock : public StateMachine {
  static int num_locks_;
  const static int clock_id_;
public:
  // Constructor.
  // k: Identifier of the lock
  // delta: the duration of the lock
  // num: total number of vehicles
  Lock(int k);
  Lock(int k, int front, int back);
  
  int transit(MessageTuple* inMsg, vector<MessageTuple*>& outMsgs,
                      bool& high_prob, int startIdx = 0);
  int nullInputTrans(vector<MessageTuple*>& outMsgs,
                             bool& high_prob, int startIdx = 0);
  void restore(const StateSnapshot* snapshot) ;
  StateSnapshot* curState() ;
  void reset() ;

  static void setNumLocks(int n_locks) { num_locks_ = n_locks; }
  
private:
  void initialize();

  MessageTuple* createResponse(MessageTuple* in_msg, const string& msg,
                               int from, int to);
  MessageTuple* createTiming(MessageTuple* in_msg, const string& msg,
                             int creator_id, int my_id);

  const int lock_id_;
  const bool active_;
  const int front_;
  const int back_;
  int master_;

  string name_;
  vector<string> channel_names_;
};

class LockMessage : public MessageTuple
{
public:
    // Constructor: src, dest, srcMsg, destMsg, subject all retain the same implication as
    // is ancestor, MessageTuple;
    // k: the ID of the source lock
    // lock: the ID of the destination competitor. (exception: when the message is
    // "complete", which is to notify the controller that the lock is released, the field
    // is used to tell the controller which competitor this lock was associated to
    LockMessage(int src, int dest, int srcMsg, int destMsg, int subject, int k,
                int to, int t)
    :MessageTuple(src, dest, srcMsg, destMsg, subject), _k(k), _to(to), _t(t) {}
    
    LockMessage( const LockMessage& msg )
    :MessageTuple(msg._src, msg._dest, msg._srcMsg, msg._destMsg, msg._subject)
    , _k(msg._k), _to(msg._to), _t(msg._t) {}
    
    LockMessage(int src, int dest, int srcMsg, int destMsg, int subject,
                      const LockMessage& msg)
    :MessageTuple(src, dest, srcMsg, destMsg, subject)
    , _k(msg._k), _to(msg._to), _t(msg._t) {}
    
    ~LockMessage() {}
    
    size_t numParams() {return 3; }
    int getParam(size_t arg) { return (arg==2)?_t:((arg==1)?_to:_k); }
    
    string toString();
    LockMessage* clone() const ;

    int getCreator() const { return _k; }
private:    
    const int _k ;
    const int _to ;
    const int _t;
};

class LockSnapshot : public StateSnapshot {
public:
  friend class Lock;

  LockSnapshot(int state, int master)
      : ss_state_(state), ss_master_(master) {}
  ~LockSnapshot() {} ;
  int curStateId() const { return ss_state_; }
  // Returns the name of current state as specified in the input file
  string toString() ;
  // Cast the Snapshot into a integer. Used in HashTable
  int toInt() { return ((ss_master_ << 16) + ss_state_); }
  LockSnapshot* clone() const { return new LockSnapshot(ss_state_,
                                                        ss_master_); }
    
private:
  int ss_state_;
  int ss_master_;
};



#endif
