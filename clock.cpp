//
//  clock.cpp
//  lock-protocol
//
//  Created by Shou-pon Lin on 2/13/15
//  Copyright (c) 2015 Shou-pon Lin. All rights reserved.
//

#include "clock.h"


Clock::Clock() {
  setId(machineToInt(getName()));
}

int Clock::transit(MessageTuple* in_msg, vector<MessageTuple*>& out_msgs,
                   bool& high_prob, int start_idx) {
  auto cmsg = dynamic_cast<ClockMessage*>(in_msg);
  if (start_idx)
    return -1;
  high_prob = true;
  int session = cmsg->getSession();
  int registrant = cmsg->getRegistrant();
  string msg = IntToMessage(cmsg->destMsgId());
  if (msg == SIGNUP) {
    // determine if owner is already on the list
    // owner should be the first one who signs up
    if (registrants_.find(session) == registrants_.end()) {
      // also record the deadlines submitted before this submission
      precursors_[session] = set<int>();
      for (auto pair : registrants_)
        precursors_[session].insert(pair.first);

      registrants_[session] = set<int>();
    }
    registrants_[session].insert(registrant);
    return 1;
  } else {
    assert(false);
    return -1;
  }
}

int Clock::nullInputTrans(vector<MessageTuple*>& out_msgs,
                          bool& high_prob, int start_idx) {
  if (registrants_.empty())
    return -1;
  if (start_idx < 0 || start_idx >= registrants_.size())
    return -1;

  high_prob = true;
  auto active_set = registrants_.begin();
  int k = start_idx;
  while (k--)
    active_set++;
  sendAlarm(active_set, out_msgs);
  precursors_.erase(precursors_.find(active_set->first));
  registrants_.erase(active_set);
  return start_idx + 1;
}

// Send alarm messages to all registrants that are registered with deadline id
// in d->first, also to all channels
void Clock::sendAlarm(map<int, set<int> >::const_iterator d,
                      vector<MessageTuple*>& out_msgs) {
  int did = d->first;
  int msg_id = messageToInt(ALARM);
  for (auto mac_id : d->second)
    out_msgs.push_back(new ClockMessage(0, mac_id, 0, msg_id, macId(),
                                        did, did));
  for (auto mac_id : channel_ids_)
    out_msgs.push_back(new ClockMessage(0, mac_id, 0, msg_id, macId(),
                                        did, did));
  auto pit = precursors_.find(did);
  for (auto session : pit->second) {
    for (auto mac_id : channel_ids_)
      out_msgs.push_back(new ClockMessage(0, mac_id, 0, msg_id, macId(),
                                          session, session));
  }
}

void Clock::restore(const StateSnapshot* snapshot) {
  auto css = dynamic_cast<const ClockSnapshot*>(snapshot);
  registrants_ = css->ss_registrants_;
  precursors_ = css->ss_precursors_;
}

StateSnapshot* Clock::curState() {
  return new ClockSnapshot(registrants_, precursors_);
}

void Clock::reset() {
  registrants_.clear();
}

int Clock::moreImminent(int a, int b) {
  // obsolete
  return -1;
}

const int ClockSnapshot::kReadable = 1;
const int ClockSnapshot::kString = 2;

ClockSnapshot::ClockSnapshot(const map<int, set<int> >& registrants,
                             const map<int, set<int> >& precursors)
    : ss_registrants_(registrants), ss_precursors_(precursors) {
  ;
}

string ClockSnapshot::toString() const {
  return stringify(kString);
}

string ClockSnapshot::toReadable() const {
  return stringify(kReadable);
}

string ClockSnapshot::stringify(int type) const {
  stringstream ss;
  ss << "[";
  int k = 0;
  auto pit = ss_precursors_.begin();
  for (const auto pair : ss_registrants_) {
    if (k++)
      ss << ",";
    ss << pair.first << ":(";
    int l = 0;
    for (auto mid : pair.second) {
      if (l++)
        ss << ",";
      if (type == kReadable)
        ss << StateMachine::IntToMachine(mid);
      else if (type == kString)
        ss << mid;
      else
        assert(false);
    }
    ss << ")";

    ss << ",(";
    l = 0;
    for (auto did : pit->second) {
      if (l++)
        ss << ",";
      ss << did;
    }
    ss << ")";
    pit++;
  }
  ss << "]";
  return ss.str();
}

ClockMessage::ClockMessage(int src, int dest, int src_msg, int dest_msg,
                           int subject, int session_id, int registrant_id)
    : MessageTuple(src, dest, src_msg, dest_msg, subject),
      session_id_(session_id), registrant_id_(registrant_id) {
  ;
}

string ClockMessage::toString() const {
  stringstream ss;
  ss << MessageTuple::toString()
     << "(" << session_id_ << "," << registrant_id_ << ")";
  return ss.str();
}
