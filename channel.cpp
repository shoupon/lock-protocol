//
//  channel.cpp
//  Prob_verify
//
//  Created by Shou-pon Lin on 8/29/12.
//  Copyright (c) 2012 Shou-pon Lin. All rights reserved.
//

#include "channel.h"

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
  auto lmsg = dynamic_cast<LockMessage*>(in_msg);
  if (msg_in_transit_)
    return -1;
  if (!start_idx) {
    high_prob = true;
    msg_in_transit_.reset(new LockMessage(*lmsg));
    out_msgs.push_back(new ClockMessage(in_msg->srcID(),
                                        machineToInt(CLOCK_NAME),
                                        in_msg->srcMsgId(),
                                        messageToInt(SIGNUP),
                                        macId(), lmsg->getCreator(), macId()));
    return 1;
  } else if (start_idx == 1) {
    high_prob = false;
    return 2;
  } else {
    return -1;
  }
}

int Channel::nullInputTrans(vector<MessageTuple*>& out_msgs,
                            bool &high_prob, int start_idx) {
  if (!start_idx)
    return -1;
  if (!msg_in_transit_)
    return -1;
  else {
    auto lmsg = dynamic_cast<LockMessage*>(msg_in_transit_.get());
    out_msgs.push_back(createDelivery());
    out_msgs.push_back(new ClockMessage(0, machineToInt(CLOCK_NAME),
                                        0, messageToInt(SIGNOFF),
                                        macId(), lmsg->getCreator(), macId()));
    reset();
    return 1;
  }
}

void Channel::restore(const StateSnapshot* snapshot) {
  auto css = dynamic_cast<const ChannelSnapshot*>(snapshot);
  if (css->ss_msg_)
    msg_in_transit_.reset(css->ss_msg_->clone());
  else
    msg_in_transit_.reset();
}

StateSnapshot* Channel::curState() {
  if (msg_in_transit_)
    return new ChannelSnapshot(msg_in_transit_.get());
  else
    return new ChannelSnapshot();
}

void Channel::reset() {
  msg_in_transit_.reset();
} 

MessageTuple* Channel::createDelivery() {
  assert(msg_in_transit_);
  return new LockMessage(0, destination_id_,
                         0, msg_in_transit_->destMsgId(), macId(),
                         *(msg_in_transit_.get()));
}

ChannelSnapshot::ChannelSnapshot(const ChannelSnapshot* ss) {
  if (ss->ss_msg_)
    ss_msg_.reset(new LockMessage(*(ss->ss_msg_)));
}

string ChannelSnapshot::toString() {
  if (ss_msg_)
    return string("[") + ss_msg_->toReadable() + "]";
  else
    return "[]";
}
