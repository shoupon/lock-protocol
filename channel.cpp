//
//  ya_channel.cpp
//  lock-protocol
//
//  Created by Shou-pon Lin on 2/11/15
//  Copyright (c) 2015 Shou-pon Lin. All rights reserved.
//

#include "channel.h"

Channel::Channel(int from, int to1, int to2)
    : src_lock_id_(from) {
  vector<int> tos;
  tos.push_back(to1);
  tos.push_back(to2);
  initialize(tos);
}

Channel::Channel(int from, const vector<int>& tos)
    : src_lock_id_(from) {
  initialize(tos);
}

int Channel::transit(MessageTuple *inMsg, vector<MessageTuple*> &outMsgs,
                     bool &high_prob, int startIdx) {
  outMsgs.clear();

  if (startIdx >= num_combinations_ || startIdx < 0)
    return -1;

  string msg = IntToMessage(inMsg->destMsgId());
  if (msg == DEADLINE) {
    if (!startIdx) {
      if (msg_in_transit_) {
        high_prob = false;
      } else {
        high_prob = true;
      }
      reset();
      return 2;
    } else {
      return -1;
    }
  } else if (typeid(*inMsg) == typeid(LockMessage)) {
    // only allow one message in channel at a time
    // Site does not send more than one message in one period
    assert(!msg_in_transit_);

    LockMessage* lMsg = dynamic_cast<LockMessage*>(inMsg);
    msg_in_transit_.reset(new LockMessage(*lMsg));
    deadline_ = lMsg->getTime();

    if (startIdx) {
      high_prob = false;
    } else {
      high_prob = true;
    }

    int k = startIdx;
    for (int i = 0; i < msg_exist_.size(); ++i) {
      if (k & 1)
        msg_exist_[i] = false;
      else
        msg_exist_[i] = true;
      k >>= 1;
    }

    return startIdx + 1;
  } else {
    assert(false); // there shouldn't be other type of messages
  }
}

int Channel::nullInputTrans(vector<MessageTuple*>& outMsgs,
                            bool& high_prob, int startIdx) {
  outMsgs.clear();
  high_prob = true;

  if (startIdx)
    return -1;

  if (!msg_in_transit_)
    return -1;

  int skip = startIdx;
  for (int i = 0; i < msg_exist_.size(); ++i) {
    if (msg_exist_[i] && skip--) {
      outMsgs.push_back(new LockMessage(0, machineToInt(dest_names_[i]),
                                        0, msg_in_transit_->destMsgId(),
                                        macId(), *msg_in_transit_));
      msg_exist_[i] = 0;
      break;
    }
  }

  if (outMsgs.empty()) {
    return -1;
  } else {
    int remain = 0;
    for (auto e : msg_exist_)
      if (e)
        remain = 1;
    if (remain)
      deadline_ = -1;
    return startIdx + 1;
  }
} 

void Channel::restore(const StateSnapshot *snapshot) {
  assert(typeid(*snapshot) == typeid(ChannelSnapshot));
  const ChannelSnapshot *css = dynamic_cast<const ChannelSnapshot*>(snapshot);
  assert(msg_exist_.size() == css->ss_exist_.size());
  if (css->ss_msg_) {
    msg_in_transit_.reset(new LockMessage(*(css->ss_msg_)));
  } else {
    msg_in_transit_.reset(0);
  }
  msg_exist_ = css->ss_exist_;
  deadline_ = css->ss_deadline_;
}

StateSnapshot* Channel::curState() {
  if (msg_in_transit_)
    return new ChannelSnapshot(msg_in_transit_.get(), msg_exist_, deadline_);
  else
    return new ChannelSnapshot(msg_exist_, deadline_);
}

void Channel::reset() {
  msg_in_transit_.reset(0);
  for (int i = 0; i < msg_exist_.size(); ++i)
    msg_exist_[i] = true;
  deadline_ = -1;
}

void Channel::initialize(const vector<int>& tos) {
  stringstream src_ss;
  src_ss << LOCK_NAME << "(" << src_lock_id_ << ")";
  src_name_ = src_ss.str();
  stringstream dest_ids_ss;

  num_combinations_ = 1;
  int k = 0;
  for (auto i : tos) {
    if (k++)
      dest_ids_ss << ",";
    dest_ids_ss << i;
    stringstream dest_ss;
    dest_ss << LOCK_NAME << "(" << i << ")";
    dest_names_.push_back(string());
    dest_ss >> dest_names_.back();
    num_combinations_ <<= 1;
    msg_exist_.push_back(true);
  }
  stringstream channel_ss;
  channel_ss << CHANNEL_NAME << "(" << src_lock_id_ << ")";
  channel_name_ = channel_ss.str();

  setId(machineToInt(channel_name_));
  reset();
}

int ChannelSnapshot::curStateId() const {
  if (ss_msg_)
    return 1;
  else 
    return 0;
}

string ChannelSnapshot::toString() {
  stringstream ss;
  if (ss_msg_) {
    int k = 0;
    for (int i = 0; i < ss_exist_.size(); ++i) {
      if (k++)
        ss << ",";
      ss << ss_exist_[i];
    }
    ss << ":" << ss_deadline_;
    return string("[") + ss_msg_->toString() + "]:" + ss.str();
  } else {
    return "[]";
  }
}

int ChannelSnapshot::toInt() {
  return ss_msg_->subjectId();
}

StateSnapshot* ChannelSnapshot::clone() const {
  if (ss_msg_)
    return new ChannelSnapshot(ss_msg_.get(), ss_exist_, ss_deadline_);
  else
    return new ChannelSnapshot(ss_exist_, ss_deadline_);
}

bool ChannelSnapshot::match(StateSnapshot* other) {
  assert(typeid(*other) == typeid(ChannelSnapshot));
  ChannelSnapshot *other_ss = dynamic_cast<ChannelSnapshot*>(other);
  if (!ss_msg_)
    return !other_ss->ss_msg_;
  else
    return *ss_msg_ == *(other_ss->ss_msg_) &&
           ss_exist_ == other_ss->ss_exist_ &&
           ss_deadline_ == other_ss->ss_deadline_;
}
