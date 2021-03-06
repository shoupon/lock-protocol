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
    : lock_id_(k), active_(false), front_(-1), back_(-1),
      front_second_(front_), back_second_(back_),
      uni_direction_(false) {
  initialize();
  reset();
}

Lock::Lock(int k, int front, int back)
    : lock_id_(k), active_(true), front_(front), back_(back),
      front_second_(front_), back_second_(back_),
      uni_direction_(false) {
  initialize();
  reset();
}

Lock::Lock(int k, int front, int back, bool uni_direction)
    : lock_id_(k), active_(true), front_(front), back_(back),
      front_second_(front_), back_second_(back_),
      uni_direction_(uni_direction) {
  initialize();
  reset();
}

Lock::Lock(int k, int front, int back, int front_second, int back_second)
    : lock_id_(k), active_(true), front_(front), back_(back),
      front_second_(front_second), back_second_(back_second),
      uni_direction_(false) {
  initialize();
  reset();
}

Lock::Lock(int k, int front, int back, int front_second, int back_second,
           bool uni_direction)
    : lock_id_(k), active_(true), front_(front), back_(back),
      front_second_(front_second), back_second_(back_second),
      uni_direction_(uni_direction) {
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
    string msg = IntToMessage(in_msg->destMsgId());
    switch (_state) {
      case 0:
        if (msg == REQUEST) {
          int session = lmsg->getSession();
          if (session < 0)
            return 3;
          int origin = lmsg->getOrigin();
          master_ = origin;

          // ignores outdated deadline
          if (master_ < 0)
            return 3;

          outMsgs.push_back(createResponse(in_msg, GRANTED, master_, master_));
          outMsgs.push_back(createTiming(in_msg, SIGNUP, master_, macId()));
          _state = 1;
          return 3;
        } else {
          // ignore
          return 3;
        }
        break;
      case 1:
        if (msg == REQUEST) {
#ifdef DENYING
          outMsgs.push_back(createResponse(in_msg, DENIED, lmsg->getSession(),
                                           lmsg->getSession()));
#endif
          return 3;
        } else {
          return 3;
        }
        break;
      case 2:
        if (msg == REQUEST)  {
#ifdef DENYING
          outMsgs.push_back(createResponse(in_msg, DENIED, lmsg->getSession(),
                                           lmsg->getSession()));
#endif
          return 3;
        } else if (msg == GRANTED) {
          int from = lmsg->getOrigin();
          int session = lmsg->getSession();
          if (session < 0)
            return 3;

          if (from == front_) {
            _state = 3;
          } else if (from == back_) {
            _state = 13;
          } else {
            assert(false);
          }
          return 3;
#ifdef DENYING
        } else if (msg == DENIED) {
          _state = 5;
          return 3;
#endif
        } else {
          return 3;
        }
        break;
      case 3:
        if (msg == REQUEST)  {
#ifdef DENYING
          outMsgs.push_back(createResponse(in_msg, DENIED, lmsg->getSession(),
                                           lmsg->getSession()));
#endif
          return 3;
        } else if (msg == GRANTED) {
          int from = lmsg->getOrigin();
          int session = lmsg->getSession();
          if (session < 0)
            return 3;

          if (from == back_)
            _state = 4;
          else
            assert(false);
          return 3;
#ifdef DENYING
        } else if (msg == DENIED) {
          _state = 5;
          return 3;
#endif
        } else {
          return 3;
        }
        break;
      case 13:
        if (msg == REQUEST)  {
#ifdef DENYING
          outMsgs.push_back(createResponse(in_msg, DENIED, lmsg->getSession(),
                                           lmsg->getSession()));
#endif
          return 3;
        } else if (msg == GRANTED) {
          int from = lmsg->getOrigin();
          int session = lmsg->getSession();
          if (session < 0)
            return 3;

          if (from == front_)
            _state = 4;
          else {
            cout << "from = " << from << ", front_ = " << front_ << endl;
            assert(false);
          }
          return 3;
#ifdef DENYING
        } else if (msg == DENIED) {
          _state = 5;
          return 3;
#endif
        } else {
          return 3;
        }
        break;
      case 6:
        if (msg == REQUEST)  {
#ifdef DENYING
          outMsgs.push_back(createResponse(in_msg, DENIED, lmsg->getSession(),
                                           lmsg->getSession()));
#endif
          return 3;
        } else if (msg == GRANTED) {
          int from = lmsg->getOrigin();
          int session = lmsg->getSession();
          if (session < 0)
            return 3;

          if (from == front_second_) {
            _state = 7;
          } else if (from == back_second_) {
            _state = 17;
          } else {
            assert(false);
          }
          return 3;
#ifdef DENYING
        } else if (msg == DENIED) {
          _state = 5;
          return 3;
#endif
        } else {
          return 3;
        }
        break;
      case 7:
        if (msg == REQUEST)  {
#ifdef DENYING
          outMsgs.push_back(createResponse(in_msg, DENIED, lmsg->getSession(),
                                           lmsg->getSession()));
#endif
          return 3;
        } else if (msg == GRANTED) {
          int from = lmsg->getOrigin();
          int session = lmsg->getSession();
          if (session < 0)
            return 3;

          if (from == back_second_)
            _state = 4;
          else
            assert(false);
          return 3;
#ifdef DENYING
        } else if (msg == DENIED) {
          _state = 5;
          return 3;
#endif
        } else {
          return 3;
        }
        break;
      case 17:
        if (msg == REQUEST)  {
#ifdef DENYING
          outMsgs.push_back(createResponse(in_msg, DENIED, lmsg->getSession(),
                                           lmsg->getSession()));
#endif
          return 3;
        } else if (msg == GRANTED) {
          int from = lmsg->getOrigin();
          int session = lmsg->getSession();
          if (session < 0)
            return 3;

          if (from == front_second_)
            _state = 4;
          else
            assert(false);
          return 3;
#ifdef DENYING
        } else if (msg == DENIED) {
          _state = 5;
          return 3;
#endif
        } else {
          return 3;
        }
        break;
      case 4:
      case 5:
        if (msg == REQUEST) {
#ifdef DENYING
          outMsgs.push_back(createResponse(in_msg, DENIED, lmsg->getSession(),
                                           lmsg->getSession()));
#endif
          return 3;
        } else {
          return 3;
        }
        break;
      default:
        throw ProtocolError::kAssertError;
        assert(false);
        break;
    }
  } else if (typeid(*in_msg) == typeid(ClockMessage)) {
    //ClockMessage *cmsg = dynamic_cast<ClockMessage *>(in_msg);
    string msg = IntToMessage(in_msg->destMsgId());
    if (msg == ALARM) {
      if (uni_direction_) {
        if (_state == 4) {
          reset();
          _state = 10;
          return 3;
        } else if (_state != 1) {
          reset();
          _state = 9;
          return 3;
        }
      }
      reset();
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
        outMsgs.push_back(createResponse(nullptr, REQUEST, front_,
                                         lock_id_));
        outMsgs.push_back(createResponse(nullptr, REQUEST, back_,
                                         lock_id_));
        outMsgs.push_back(createTiming(nullptr, SIGNUP, lock_id_, macId()));
        _state = 2;
        master_ = lock_id_;
        return 1;
      } else if (startIdx == 1 && active_ &&
                 (front_ != front_second_ || back_ != back_second_)) {
        outMsgs.push_back(createResponse(nullptr, REQUEST,
                                         front_second_, lock_id_));
        outMsgs.push_back(createResponse(nullptr, REQUEST,
                                         back_second_, lock_id_));
        outMsgs.push_back(createTiming(nullptr, SIGNUP, lock_id_, macId()));
        _state = 6;
        master_ = lock_id_;
        return 2;
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
        outMsgs.push_back(createResponse(nullptr, RELEASE, front_,
                                         lock_id_));
        outMsgs.push_back(createTiming(nullptr, SIGNOFF, lock_id_, macId()));
        _current = 0;
        return 3;
      } else {
        return -1;
      }
      break;
    case 13:
      if (!startIdx) {
        outMsgs.push_back(createResponse(nullptr, RELEASE, back_,
                                         lock_id_));
        outMsgs.push_back(createTiming(nullptr, SIGNOFF, lock_id_, macId()));
        _current = 0;
        return 3;
      } else {
        return -1;
      }
      break;
    case 4:
      if (!startIdx) {
        outMsgs.push_back(createResponse(nullptr, RELEASE, front_,
                                         lock_id_));
        outMsgs.push_back(createResponse(nullptr, RELEASE, back_,
                                         lock_id_));
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

int Lock::determineSession(LockMessage* in_msg) {
  auto clock_ptr = dynamic_cast<Clock*>(ProbVerifier::getMachine(CLOCK_NAME));
  return clock_ptr->moreImminent(in_msg->getSession(), master_);
}

MessageTuple* Lock::createResponse(MessageTuple* in_msg,
                                   const string& msg, int to,
                                   int session) {
  int dest_id = channel_mac_ids_[to];
  if (in_msg) {
    auto lmsg = dynamic_cast<LockMessage*>(in_msg);
    return new LockMessage(lmsg->subjectId(), dest_id,
                           lmsg->destMsgId(), messageToInt(msg),
                           macId(), session, lock_id_);
  } else {
    return new LockMessage(0, dest_id, 0, messageToInt(msg),
                           macId(), session, lock_id_);
  }
}

MessageTuple* Lock::createTiming(MessageTuple* in_msg, const string& msg,
                                 int session, int my_id) {
  if (in_msg)
    return new ClockMessage(in_msg->subjectId(), clock_id_,
                            in_msg->destMsgId(), messageToInt(msg),
                            macId(), session, my_id);
  else
    return new ClockMessage(0, clock_id_, 0, messageToInt(msg),
                            macId(), session, my_id);
}



string LockMessage::toString() const {
  stringstream ss;
  ss << MessageTuple::toString() << "(" << session_ << ")";
  return ss.str();
}

string LockMessage::toReadable() const {
  stringstream ss;
  ss << MessageTuple::toReadable() << "(session = " << session_ << ")";
  return ss.str();
}

LockMessage* LockMessage::clone() const {
  return new LockMessage(*this);
}

string LockSnapshot::toString() const {
  stringstream ss;
  ss << ss_state_ << "(" << ss_master_ << ")";
  return ss.str();
}

bool LockSnapshot::match(StateSnapshot *other) {
  assert(typeid(*other) == typeid(LockSnapshot));
  auto lss = dynamic_cast<LockSnapshot*>(other);
  return ss_state_ == lss->ss_state_ && ss_master_ == lss->ss_master_;
}
