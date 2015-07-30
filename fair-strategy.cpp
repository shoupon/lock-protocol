//
//  fair-strategy.cpp
//  lock-protocol
//
//  Created by Shou-pon Lin on 7/28/15.
//  Copyright (c) 2015 Shou-pon Lin. All rights reserved.
//

#include <sstream>
using namespace std;

#include "fair-strategy.h"

map<int, IdStatePairs> FairStrategy::tracked_;

string FairStrategyState::toString() {
  stringstream ss;
  int k = 0;
  for (auto& mac_pairs : tracked_states_) {
    if (k++)
      ss << ",";
    ss << mac_pairs.first << ":[";
    int kk = 0;
    for (auto& pair : mac_pairs.second) {
      if (kk++)
        ss << ",";
      ss << "(" << pair.first << "," << pair.second << ")";
    }
    ss << "]";
  }
  return ss.str();
}

void FairStrategy::initialize(const map<int, IdStatePairs> tracked) {
  tracked_ = tracked;
}

bool FairStrategy::validChoice(int mac_id, int null_input_idx,
                               const vector<StateSnapshot*>& mstate,
                               StrategyState *strategy_state) {
  auto fss = dynamic_cast<FairStrategyState*>(strategy_state);
  auto& cur_tracked = fss->tracked_states_;
  auto tracked_it = cur_tracked.find(mac_id);
  if (tracked_it == cur_tracked.end())
    return true;

  // only available non-deterministic choices of lock is request
  if (null_input_idx != 0 && null_input_idx != 1)
    return true;

  // If
  for (auto& pair : tracked_it->second) {
    if (pair.second != -1)
      return false;
  }

  for (auto& pair : tracked_it->second) {
    int midx = pair.first - 1;
    pair.second = mstate[midx]->curStateId();
  }
  return true;
}

StrategyState* FairStrategy::createInitState() {
  FairStrategyState *ret = new FairStrategyState(tracked_);
  for (auto& individual : ret->tracked_states_) {
    for (auto& pair : individual.second)
      pair.second = -1;
  }
  return ret;
}

void FairStrategy::finalizeTransition(const vector<StateSnapshot*>& mstate,
                                      StrategyState *strategy_state) {
  auto fss = dynamic_cast<FairStrategyState*>(strategy_state);
  for (auto& mac_pairs : fss->tracked_states_) {
    for (auto& pair : mac_pairs.second) {
      int mac_id = pair.first;
      int midx = mac_id - 1;
      if (pair.second != -1 && pair.second != mstate[midx]->curStateId())
        pair.second = -1;
    }
  }
}

