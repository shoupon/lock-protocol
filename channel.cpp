//
//  channel.cpp
//  Prob_verify
//
//  Created by Shou-pon Lin on 8/29/12.
//  Copyright (c) 2012 Shou-pon Lin. All rights reserved.
//

#include "channel.h"
#define DELAY_LOW

Channel::Channel(int from, int to)
    : origin_(from), destination_(to) {
   stringstream ss_channel_name;
   ss_channel_name << CHANNEL_NAME << "(" << from << "," << to << ")";
   channel_name_ = ss_channel_name.str();
   setId(machineToInt(channel_name_));
   stringstream ss_destination_name;
   ss_destination_name << LOCK_NAME << "(" << to << ")";
   destination_id_ = machineToInt(ss_destination_name.str());
}
   
int Channel::transit(MessageTuple* in_msg, vector<MessageTuple*>& out_msgs,
                     bool &high_prob, int start_idx) {
  high_prob = true;
  if (typeid(*in_msg) == typeid(ClockMessage)) {
    if (!start_idx) {
      auto cmsg = dynamic_cast<ClockMessage*>(in_msg);
      int did = cmsg->getSession();
      auto msg_it = msgs_in_transit_.begin();
      while (msg_it != msgs_in_transit_.end()) {
        if ((*msg_it)->getSession() == did) {
          msgs_in_transit_.erase(msg_it);
#ifdef DELAY_LOW
          high_prob = false;
#endif
          msg_it = msgs_in_transit_.begin();
        } else {
          msg_it++;
        }
      }
      return 3;
    } else {
      return -1;
    }
  }

  if (msgs_in_transit_.size() > 2)
    return -1;
  if (typeid(*in_msg) == typeid(LockMessage)) {
    if (!start_idx) {
      high_prob = true;
      auto lmsg = dynamic_cast<LockMessage*>(in_msg);
      msgs_in_transit_.push_back(shared_ptr<LockMessage>(
          new LockMessage(*lmsg)));
      return 1;
    } else if (start_idx == 1) {
      high_prob = false;
      return 2;
    } else {
      return -1;
    }
  } else {
    return -1;
  }
}

int Channel::nullInputTrans(vector<MessageTuple*>& out_msgs,
                            bool &high_prob, int start_idx) {
  high_prob = true;
  if (start_idx || msgs_in_transit_.empty())
    return -1;
  else {
    out_msgs.push_back(createDelivery());
    msgs_in_transit_.erase(msgs_in_transit_.begin());
    return 1;
  }
}

void Channel::restore(const StateSnapshot* snapshot) {
  auto css = dynamic_cast<const ChannelSnapshot*>(snapshot);
  msgs_in_transit_.clear();
  for (auto msg : css->ss_msgs_)
    msgs_in_transit_.push_back(shared_ptr<LockMessage>(new LockMessage(*msg)));
}

StateSnapshot* Channel::curState() {
  if (msgs_in_transit_.size())
    return new ChannelSnapshot(msgs_in_transit_);
  else
    return new ChannelSnapshot();
}

void Channel::reset() {
  msgs_in_transit_.clear();
} 

MessageTuple* Channel::createDelivery() {
  assert(msgs_in_transit_.size());
  auto msg = msgs_in_transit_[0];
  return new LockMessage(0, destination_id_,
                         0, msg->destMsgId(), macId(), *msg);
}

ChannelSnapshot::ChannelSnapshot(const vector<shared_ptr<LockMessage>>& msgs) {
  for (auto msg : msgs)
    ss_msgs_.push_back(shared_ptr<LockMessage>(new LockMessage(*msg)));
}

size_t ChannelSnapshot::getBytes() const {
  shared_ptr<LockMessage> ptr;
  size_t sz = sizeof(ss_msgs_) + sizeof(ptr) * ss_msgs_.size();
  for (auto p : ss_msgs_)
    sz += p->getBytes();
  return sz;
}

string ChannelSnapshot::toString() const {
  if (ss_msgs_.size() == 2)
    return string("[") + ss_msgs_[0]->toString() + ","
        + ss_msgs_[1]->toReadable() + "]";
  else if (ss_msgs_.size() == 1)
    return string("[") + ss_msgs_[0]->toString() + "]";
  else
    return "e";
}

string ChannelSnapshot::toReadable() const {
  if (ss_msgs_.size() == 2)
    return string("[") + ss_msgs_[0]->toReadable() + ","
        + ss_msgs_[1]->toReadable() + "]";
  else if (ss_msgs_.size() == 1)
    return string("[") + ss_msgs_[0]->toReadable() + "]";
  else
    return "[]";
}

bool ChannelSnapshot::match(StateSnapshot* other) {
  assert(typeid(*other) == typeid(ChannelSnapshot));
  auto css = dynamic_cast<ChannelSnapshot*>(other);
  if (ss_msgs_.empty()) {
    return css->ss_msgs_.empty();
  } else if (ss_msgs_.size() != css->ss_msgs_.size()) {
    return false;
  } else {
    for (int i = 0; i < ss_msgs_.size(); ++i) {
      if (!(*ss_msgs_[i] == *css->ss_msgs_[i]))
        return false;
    }
  }
  return true;
}
