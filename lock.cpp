//
//  lock.cpp
//  Prob_verify
//
//  Created by Shou-pon Lin on 8/24/12.
//  Copyright (c) 2012 Shou-pon Lin. All rights reserved.
//
#include <sstream>
#include <string>
using namespace std;

#include "lock.h"

int Lock::num_locks_;
int Lock::clock_id_;

Lock::Lock(int k)
    : lock_id_(k), active_(false), front_(-1), back_(-1) {
  initialize();
  reset();
}

Lock::Lock(int k, int front, int back)
    : lock_id_(k), active_(true), front_(front), back_(back) {
  initialize();
  reset();
}

void Lock::initialize() {
  clock_id_ = StateMachine::machineToInt(CLOCK_NAME);
  stringstream ss_lock_name;
  ss_lock_name << LOCK_NAME << "(" << lock_id_ << ")";
  name_ = ss_lock_name.str();
  setId(machineToInt(name_));
  for (int i = 0; i < num_locks_; ++i) {
    if (i == lock_id_)
      continue;
    stringstream ss_channel_name;
    ss_channel_name << CHANNEL_NAME << "(" << lock_id_ << "," << i << ")";
    channel_names_[i] = ss_channel_name.str();
    channel_mac_ids_[i] = machineToInt(channel_names_[i]);
  }
}

int Lock::transit(MessageTuple* in_msg, vector<MessageTuple*>& outMsgs,
                  bool& high_prob, int startIdx) {
  outMsgs.clear();

  if (startIdx)
    return -1;
  high_prob = true ;

  if (typeid(*in_msg) == typeid(LockMessage)) {
    LockMessage *lmsg = dynamic_cast<LockMessage *>(in_msg);
    int from = lmsg->getCreator();
    string msg = IntToMessage(in_msg->destMsgId());
    switch (_state) {
      case 0:
        if (msg == REQUEST) {
          master_ = lmsg->getCreator();
          outMsgs.push_back(createResponse(in_msg, GRANTED, lock_id_, master_));
          outMsgs.push_back(createTiming(in_msg, SIGNUP, master_, macId()));
          _state = 1;
          master_ = from;
          return 3;
        } else {
          // ignore
          return 3;
        }
        break;
      case 1:
        if (msg == REQUEST) {
          outMsgs.push_back(createResponse(in_msg, DENIED, lock_id_,
                                           lmsg->getCreator()));
          return 3;
        } else {
          return 3;
        }
        break;
      case 2:
        if (msg == REQUEST)  {
          outMsgs.push_back(createResponse(in_msg, DENIED, lock_id_,
                                           lmsg->getCreator()));
          return 3;
        } else if (msg == GRANTED) {
          int from = lmsg->getCreator();
          if (from == front_) {
            _state = 3;
          } else if (from == back_) {
            _state = 13;
          } else {
            assert(false);
          }
          return 3;
        }
        break;
      case 3:
        if (msg == REQUEST)  {
          outMsgs.push_back(createResponse(in_msg, DENIED, lock_id_,
                                           lmsg->getCreator()));
          return 3;
        } else if (msg == GRANTED) {
          int from = lmsg->getCreator();
          if (from == back_)
            _state = 4;
          else
            assert(false);
          return 3;
        }
        break;
      case 13:
        if (msg == REQUEST)  {
          outMsgs.push_back(createResponse(in_msg, DENIED, lock_id_,
                                           lmsg->getCreator()));
          return 3;
        } else if (msg == GRANTED) {
          int from = lmsg->getCreator();
          if (from == front_)
            _state = 4;
          else
            assert(false);
          return 3;
        }
        break;
      case 4:
        if (msg == REQUEST) {
          outMsgs.push_back(createResponse(in_msg, DENIED, lock_id_,
                                           lmsg->getCreator()));
          return 3;
        } else {
          return 3;
        }
        break;
      default:
        assert(false);
        break;
    }
  } else if (typeid(*in_msg) == typeid(ClockMessage)) {
    //ClockMessage *cmsg = dynamic_cast<ClockMessage *>(in_msg);
    string msg = IntToMessage(in_msg->destMsgId());
    if (msg == ALARM) {
      _state = 0;
      return 3;
    }
  }
  return -1;
}

int Lock::nullInputTrans(vector<MessageTuple*>& outMsgs,
                         bool& high_prob, int startIdx) {
  high_prob = true;
  switch (_state) {
    case 0:
      if (!startIdx && active_) {
        outMsgs.push_back(createResponse(nullptr, REQUEST, lock_id_, front_));
        outMsgs.push_back(createResponse(nullptr, REQUEST, lock_id_, back_));
        outMsgs.push_back(createTiming(nullptr, SIGNUP, lock_id_, macId()));
        _state = 2;
        master_ = lock_id_;
        return 3;
      } else {
        return -1;
      }
      break;
#ifdef RELEASING
    case 2:  // releasing
      if (!startIdx) {
        outMsgs.push_back(createTiming(nullptr, SIGNOFF, lock_id_, macId()));
        _current = 0;
        return 3;
      } else {
        return -1;
      }
      break;
    case 3:  // releasing
      if (!startIdx) {
        outMsgs.push_back(createResponse(nullptr, RELEASE, lock_id_, front_));
        outMsgs.push_back(createTiming(nullptr, SIGNOFF, lock_id_, macId()));
        _current = 0;
        return 3;
      } else {
        return -1;
      }
      break;
    case 13:
      if (!startIdx) {
        outMsgs.push_back(createResponse(nullptr, RELEASE, lock_id_, back_));
        outMsgs.push_back(createTiming(nullptr, SIGNOFF, lock_id_, macId()));
        _current = 0;
        return 3;
      } else {
        return -1;
      }
      break;
    case 4:
      if (!startIdx) {
        outMsgs.push_back(createResponse(nullptr, RELEASE, lock_id_, front_));
        outMsgs.push_back(createResponse(nullptr, RELEASE, lock_id_, back_));
        outMsgs.push_back(createTiming(nullptr, SIGNOFF, lock_id_, macId()));
        _current = 0;
        return 3;
      } else {
        return -1;
      }
      break;
#endif
    default:
      break;
  }
  return -1;
}

void Lock::restore(const StateSnapshot* snapshot) {
  assert( typeid(*snapshot) == typeid(LockSnapshot) ) ;
  const LockSnapshot* sslock = dynamic_cast<const LockSnapshot*>(snapshot);
  _state = sslock->ss_state_;
  master_ = sslock->ss_master_;
}

StateSnapshot* Lock::curState() {
  return new LockSnapshot(_state, master_);
}


void Lock::reset() {
  _state = 0;
  master_ = -1;
}

MessageTuple* Lock::createResponse(MessageTuple* in_msg,
                                   const string& msg, int from, int to) {
  int dest_id = channel_mac_ids_[to];
  if (in_msg) {
    auto lmsg = dynamic_cast<LockMessage*>(in_msg);
    return new LockMessage(lmsg->subjectId(), dest_id,
                           lmsg->destMsgId(), messageToInt(msg),
                           macId(), from);
  } else {
    return new LockMessage(0, dest_id, 0, messageToInt(msg),
                           macId(), from);
  }
}

MessageTuple* Lock::createTiming(MessageTuple* in_msg, const string& msg,
                                 int creator_id, int my_id) {
  if (in_msg)
    return new ClockMessage(in_msg->subjectId(), clock_id_,
                            in_msg->destMsgId(), messageToInt(msg),
                            macId(), creator_id, my_id);
  else
    return new ClockMessage(0, clock_id_, 0, messageToInt(msg),
                            macId(), creator_id, my_id);
}



string LockMessage::toString() {
  stringstream ss ;
  ss << MessageTuple::toString() << "(m=" << master_ << ")" ;
  return ss.str() ;
}

LockMessage* LockMessage::clone() const {
  return new LockMessage(*this);
}

string LockSnapshot::toString() {
  stringstream ss;
  ss << ss_state_ << "(" << ss_master_ << ")";
  return ss.str();
}
