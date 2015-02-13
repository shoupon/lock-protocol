//
//  clock.cpp
//  lock-protocol
//
//  Created by Shou-pon Lin on 2/13/15
//  Copyright (c) 2015 Shou-pon Lin. All rights reserved.
//

#include "clock.h"

int Clock::transit(MessageTuple* in_msg, vector<MessageTuple*>& out_msgs,
                   bool& high_prob, int start_idx) {
  auto cmsg = dynamic_cast<ClockMessage*>(in_msg);
  if (start_idx)
    return -1;
  int creator = cmsg->getMaster();
  int follower_mac_id = cmsg->getFollwer();
  string msg = IntToMessage(cmsg->destMsgId());
  if (msg == SIGNUP) {
    // determine if creator is already on the list
    for (int i = 0; i < creators_.size(); ++i) {
      if (creators_[i] == creator) {
        followers_[i].insert(follower_mac_id);
        return 1;
      }
    }
    // if not on the list
    creators_.push_back(creator);
    followers_.push_back(set<int>());
    followers_.back().insert(follower_mac_id);
    return 1;
  } else if (msg == SIGNOFF) {
    for (int i = 0; i < creators_.size(); ++i) {
      if (creators_[i] == creator) {
        followers_[i].erase(followers_[i].find(follower_mac_id));
        if (followers_[i].size() == 0) {
          followers_.erase(followers_.begin() + i);
          creators_.erase(creators_.begin() + i);
        }
        return 1;
      }
    }
    return -1;
  } else {
    assert(false);
    return -1;
  }
}

int Clock::nullInputTrans(vector<MessageTuple*>& out_msgs,
                          bool& high_prob, int start_idx) {
  if (start_idx)
    return -1;
  high_prob = true;
  int msg_id = messageToInt(ALARM);
  for (auto mac_id : followers_.front())
    out_msgs.push_back(new ClockMessage(0, mac_id, 0, msg_id, macId(),
                                        creators_.front(), mac_id));
  creators_.erase(creators_.begin());
  followers_.erase(followers_.begin());
  return 1;
}

void Clock::restore(const StateSnapshot* snapshot) {
  auto css = dynamic_cast<const ClockSnapshot*>(snapshot);
  creators_ = css->ss_creators_;
  followers_ = css->ss_followers_;
}

StateSnapshot* Clock::curState() {
  return new ClockSnapshot(creators_, followers_);
}

void Clock::reset() {
  creators_.clear();
  followers_.clear();
}

ClockSnapshot::ClockSnapshot(const vector<int>& creators,
                             const vector<set<int> >& followers)
    : ss_creators_(creators), ss_followers_(followers) {
  ;
}

string ClockSnapshot::toString() {
  stringstream ss;
  ss << "[";
  int k = 0;
  for (int i = 0; i < ss_creators_.size(); ++i) {
    if (k++)
      ss << ",";
    ss << ss_creators_[i] << ":(";
    int l = 0;
    for (auto mid : ss_followers_[i]) {
      if (l++)
        ss << ",";
      ss << mid;
    }
    ss << ")";
  }
  ss << "]";
  return ss.str();
}

ClockMessage::ClockMessage(int src, int dest, int src_msg, int dest_msg,
                           int subject, int master_id, int follower_id)
    : MessageTuple(src, dest, src_msg, dest_msg, subject),
      master_id_(master_id), follower_id_(follower_id) {
  ;
}

string ClockMessage::toString() {
  stringstream ss;
  ss << MessageTuple::toString()
     << "(" << master_id_ << "," << follower_id_ << ")";
  return ss.str();
}
