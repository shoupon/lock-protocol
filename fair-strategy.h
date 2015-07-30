//
//  fair-strategy.h
//  lock-protocol
//
//  Created by Shou-pon Lin on 7/28/15.
//  Copyright (c) 2015 Shou-pon Lin. All rights reserved.
//

#ifndef LOCKPROTOCOL_FAIRSTRATEGY_H
#define LOCKPROTOCOL_FAIRSTRATEGY_H

#include <cassert>

#include <iostream>
#include <vector>
using namespace std;

#include "../prob_verify/statemachine.h"
#include "../prob_verify/strategy.h"

#include "lock.h"

typedef map<int, int> IdStatePairs;

class FairStrategy;

class FairStrategyState : public StrategyState {
  friend class FairStrategy;
public:
  FairStrategyState(const map<int, IdStatePairs>& tracked)
      : tracked_states_(tracked) {}
  ~FairStrategyState() {}
  StrategyState *clone() const {
    return new FairStrategyState(tracked_states_);
  }
  string toString();

private:
  map<int, IdStatePairs> tracked_states_;
};


class FairStrategy : public Strategy {
public:
  FairStrategy() {}
  ~FairStrategy() {}
  void initialize(const map<int, IdStatePairs> tracked);
  bool validChoice(int mac_id, int null_input_idx,
                   const vector<StateSnapshot*>& mstate,
                   StrategyState *strategy_state);
  StrategyState *createInitState();
  void finalizeTransition(const vector<StateSnapshot*>& mstate,
                          StrategyState *strategy_state);

private:
  static map<int, IdStatePairs> tracked_;
};

#endif // LOCKPROTOCOL_FAIRSTRATEGY_H
