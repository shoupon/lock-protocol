//
//  main.cpp
//  verification-lock
//
//  Created by Shou-pon Lin on 11/19/13.
//  Copyright (c) 2013 Shou-pon Lin. All rights reserved.
//

#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>
using namespace std;

#include "../prob_verify/pverify.h"
#include "../prob_verify/sync.h"

#include "lock.h"
#include "channel.h"
#include "lock_utils.h"
#include "fair-strategy.h"

#define SCENARIO 2

ProbVerifier pvObj ;
GlobalState* startPoint;

vector<Lock> locks;
vector<vector<shared_ptr<Channel>>> channels;

bool printStop(GlobalState* left, GlobalState* right)
{
    return true;
}

void setupLockedState(StoppingState& stop, int m, int f, int b) {
  stop.addAllow(new LockSnapshot(4, m), locks[m].getName());
  stop.addAllow(new LockSnapshot(1, m), locks[f].getName());
  stop.addAllow(new LockSnapshot(1, m), locks[b].getName());
  for (auto &c : channels[m])
    stop.addAllow(new ChannelSnapshot(), c->getName());
  for (auto &c : channels[f])
    stop.addAllow(new ChannelSnapshot(), c->getName());
  for (auto &c : channels[b])
    stop.addAllow(new ChannelSnapshot(), c->getName());
}

void setupResetState(StoppingState& stop, int m, int f, int b) {
  stop.addAllow(new LockSnapshot(0, -1), locks[m].getName());
  for (auto &c : channels[m])
    stop.addAllow(new ChannelSnapshot(), c->getName());
}

void setupSuccessState(StoppingState& stop, int m, int f, int b) {
  stop.addAllow(new LockSnapshot(10, -1), locks[m].getName());
  stop.addAllow(new LockSnapshot(0, -1), locks[f].getName());
  stop.addAllow(new LockSnapshot(0, -1), locks[b].getName());
}

void setupFailureState(StoppingState& stop, int m, int f, int b) {
  stop.addAllow(new LockSnapshot(9, -1), locks[m].getName());
}

void setupDeniedState(StoppingState& stop, int m) {
  stop.addAllow(new LockSnapshot(5, m), locks[m].getName());
  for (auto &c : channels[m])
    stop.addAllow(new ChannelSnapshot(), c->getName());
}

int nLocks = 7;

int main( int argc, char* argv[] )
{
  try {
    // Create StateMachine objects
    // Add the state machines into ProbVerifier
    // Register the machines that are triggered by deadline (sync)
    unique_ptr<Lookup> message_lookup(new Lookup());
    unique_ptr<Lookup> machine_lookup(new Lookup());
    StateMachine::setLookup(message_lookup.get(), machine_lookup.get());

    Clock clock;
    pvObj.addMachine(&clock);

    // Register the names of the locks
    StateMachine::machineToInt("lock(0)");
    StateMachine::machineToInt("lock(1)");
    StateMachine::machineToInt("lock(2)");
    StateMachine::machineToInt("lock(3)");
    StateMachine::machineToInt("lock(4)");
    StateMachine::machineToInt("lock(5)");
    StateMachine::machineToInt("lock(6)");

    channels.clear();
    channels.resize(nLocks);
#if (SCENARIO >= 1)
    channels[0].push_back(shared_ptr<Channel>(new Channel(0, 2)));
    channels[1].push_back(shared_ptr<Channel>(new Channel(1, 2)));
    channels[2].push_back(shared_ptr<Channel>(new Channel(2, 0)));
    channels[2].push_back(shared_ptr<Channel>(new Channel(2, 1)));
#endif
#if (SCENARIO >= 2)
    channels[1].push_back(shared_ptr<Channel>(new Channel(1, 4)));
    channels[3].push_back(shared_ptr<Channel>(new Channel(3, 4)));
    channels[4].push_back(shared_ptr<Channel>(new Channel(4, 1)));
    channels[4].push_back(shared_ptr<Channel>(new Channel(4, 3)));
#endif
#if (SCENARIO >= 4)
    channels[0].push_back(shared_ptr<Channel>(new Channel(0, 5)));
    channels[1].push_back(shared_ptr<Channel>(new Channel(1, 5)));
    channels[5].push_back(shared_ptr<Channel>(new Channel(5, 0)));
    channels[5].push_back(shared_ptr<Channel>(new Channel(5, 1)));
#endif
#if (SCENARIO >= 5)
    channels[1].push_back(shared_ptr<Channel>(new Channel(1, 6)));
    channels[6].push_back(shared_ptr<Channel>(new Channel(6, 1)));
#endif
    for (auto& cs : channels) {
      for (auto& c : cs) {
        pvObj.addMachine(c.get());
        clock.registerChannel(c->macId());
      }
    }

    // Create StateMachine objects
    Lock::setNumLocks(nLocks);
    locks.clear();
    locks.push_back(Lock(0));
#if (SCENARIO == 3 || SCENARIO == 4)
    locks.push_back(Lock(1, 2, 4, true));
#elif (SCENARIO == 5)
    locks.push_back(Lock(1, 2, 4, 5, 6, true));
#else
    locks.push_back(Lock(1));
#endif
#if (SCENARIO == 1)
    locks.push_back(Lock(2, 0, 1, true));
#elif (SCENARIO > 1)
    locks.push_back(Lock(2, 0, 1));
#else
    locks.push_back(Lock(2));
#endif
    locks.push_back(Lock(3));
#if (SCENARIO == 2)
    locks.push_back(Lock(4, 1, 3, true));
#elif (SCENARIO > 2)
    locks.push_back(Lock(4, 1, 3));
#else
    locks.push_back(Lock(4));
#endif
#if (SCENARIO >= 4)
    locks.push_back(Lock(5, 0, 1));
#else
    locks.push_back(Lock(5));
#endif
    locks.push_back(Lock(6));

    for (auto& l : locks) {
      pvObj.addMachine(&l);
    }

    FairStrategy fair_strategy;
    map<int, IdStatePairs> strategy_config;

#if (SCENARIO == 2)
    strategy_config[locks[2].macId()] = IdStatePairs();
    strategy_config[locks[2].macId()][locks[4].macId()] = -1;
#endif
#if (SCENARIO >= 3)
    strategy_config[locks[2].macId()] = IdStatePairs();
    strategy_config[locks[2].macId()][locks[1].macId()] = 0;
    strategy_config[locks[4].macId()] = IdStatePairs();
    strategy_config[locks[4].macId()][locks[1].macId()] = 0;
#endif
#if (SCENARIO >= 4)
    strategy_config[locks[5].macId()] = IdStatePairs();
    strategy_config[locks[5].macId()][locks[1].macId()] = 0;
#endif
#if (SCENARIO >= 2)
    fair_strategy.initialize(strategy_config);
#endif

    //LockService *srvc = new LockService(2,0,1);
    Service srvc;
    StateMachine::dumpMachineTable();

    // Configure GlobalState
#if (SCENARIO < 2)
    Strategy strategy;
    GlobalState::setStrategy(&strategy);
#endif
#if (SCENARIO >= 2)
    GlobalState::setStrategy(&fair_strategy);
#endif

    // Specify the starting state
    GlobalState::setService(&srvc);
    GlobalState start_point(pvObj.getMachinePtrs());
#if (SCENARIO < 2)
    start_point.setStrategyState(new StrategyState());
#endif
#if (SCENARIO >= 2)
    start_point.setStrategyState(new FairStrategyState(strategy_config));
#endif

    // Specify the global states in the set RS (stopping states)
    // initial state
    StoppingState stop_zero(&start_point);
    for (auto& l : locks)
      stop_zero.addAllow(new LockSnapshot(), l.getName());
    for (auto& cs : channels) {
      for (auto& c : cs)
        stop_zero.addAllow(new ChannelSnapshot(), c->getName());
    }
    pvObj.addSTOP(&stop_zero);
#if (SCENARIO == 1)
    StoppingState stop_group1_reset(&start_point);
    setupResetState(stop_group1_reset, 2, 0, 1);
    pvObj.addSTOP(&stop_group1_reset);

    StoppingState stop_group1_success(&start_point);
    setupSuccessState(stop_group1_success, 2, 0, 1);
    pvObj.addEND(&stop_group1_success);

    StoppingState stop_group1_failure(&start_point);
    setupFailureState(stop_group1_failure, 2, 0, 1);
    pvObj.addEND(&stop_group1_failure);
#endif
#if (SCENARIO == 2)
    StoppingState stop_group2_reset(&start_point);
    setupResetState(stop_group2_reset, 4, 1, 3);
    pvObj.addSTOP(&stop_group2_reset);

    StoppingState stop_group2_success(&start_point);
    setupSuccessState(stop_group2_success, 4, 1, 3);
    pvObj.addEND(&stop_group2_success);

    StoppingState stop_group2_failure(&start_point);
    setupFailureState(stop_group2_failure, 4, 1, 3);
    pvObj.addEND(&stop_group2_failure);
#endif
#if (SCENARIO >= 3)
    StoppingState stop_group3_reset(&start_point);
    setupResetState(stop_group3_reset, 1, 2, 4);
    pvObj.addSTOP(&stop_group3_reset);

    StoppingState stop_group3_success(&start_point);
    setupSuccessState(stop_group3_success, 1, 2, 4);
    pvObj.addEND(&stop_group3_success);

    StoppingState stop_group3_failure(&start_point);
    setupFailureState(stop_group3_failure, 1, 2, 4);
    pvObj.addEND(&stop_group3_failure);
#endif
#if (SCENARIO >= 5)
    StoppingState stop_group5_success(&start_point);
    setupSuccessState(stop_group5_success, 1, 5, 6);
    pvObj.addEND(&stop_group5_success);
#endif
    /*
    // state LF
    StoppingState stopLF(startPoint);
    stopLF.addAllow(new LockSnapshot(10,-1,-1,2,0), 2); // lock 0
    stopLF.addAllow(new LockSnapshot(10,-1,-1,2,0), 3); // lock 1
    stopLF.addAllow(new LockSnapshot(10,0,1,-1,4), 4); // lock 2
    pvObj.addSTOP(&stopLF);
    
    // state FL
    StoppingState stopFL(startPoint);
    stopFL.addAllow(new LockSnapshot(10,-1,-1,4,1), 3); // lock 1
    stopFL.addAllow(new LockSnapshot(10,-1,-1,4,1), 5); // lock 3
    stopFL.addAllow(new LockSnapshot(10,1,3,-1,4), 6); // lock 4
    pvObj.addSTOP(&stopFL);
    
    // state 2L: master car2, slaves car3 car5
    StoppingState stop2L(startPoint);
    stop2L.addAllow(new LockSnapshot(10,2,4,-1,4), 3); // lock 1
    stop2L.addAllow(new LockSnapshot(10,-1,-1,1,1), 4); // lock 2
    stop2L.addAllow(new LockSnapshot(10,-1,-1,1,1), 6); // lcok 4
    pvObj.addSTOP(&stop2L);
    
    // state 6L: master car6, slaves car1 car2
    StoppingState stop6L(startPoint);
    stop6L.addAllow(new LockSnapshot(10,-1,-1,5,1), 2); // lock 0
    stop6L.addAllow(new LockSnapshot(10,-1,-1,5,1), 3); // lock 1
    stop6L.addAllow(new LockSnapshot(10,0,1,-1,4), 7); // lock 5
    pvObj.addSTOP(&stop6L);

    
    // Specify the error states
    // One of the slaves is not locked
    StoppingState lock3FFree(startPoint) ;
    lock3FFree.addAllow(new LockSnapshot(-1,-1,-1,-1,0), 2); // lock 0 in state 0
    lock3FFree.addAllow(new LockSnapshot(10,0,1,-1,4), 4); // lock 2 in state 4
    pvObj.addError(&lock3FFree);
    
    StoppingState lock3BFree(startPoint) ;
    lock3BFree.addAllow(new LockSnapshot(-1,-1,-1,-1,0), 3); // lock 1 in state 0
    lock3BFree.addAllow(new LockSnapshot(10,0,1,-1,4), 4); // lock 2 in state 4
    pvObj.addError(&lock3BFree);
    
    StoppingState lock5FFree(startPoint) ;
    lock5FFree.addAllow(new LockSnapshot(-1,-1,-1,-1,0), 3); // lock 1 in state 0
    lock5FFree.addAllow(new LockSnapshot(10,1,3,-1,4), 6); // lock 4 in state 4
    pvObj.addError(&lock5FFree);
    
    StoppingState lock5BFree(startPoint) ;
    lock5BFree.addAllow(new LockSnapshot(-1,-1,-1,-1,0), 5); // lock 3 in state 0
    lock5BFree.addAllow(new LockSnapshot(10,1,3,-1,4), 6); // lock 4 in state 4
    pvObj.addError(&lock5BFree);
    
    StoppingState lock2FFree(startPoint);
    lock2FFree.addAllow(new LockSnapshot(10,2,4,-1,4), 3); // lock 1 in state 4
    lock2FFree.addAllow(new LockSnapshot(10,-1,-1,-1,0), 4); // lock 2 in state 0
    pvObj.addError(&lock2FFree);
    
    StoppingState lock2BFree(startPoint);
    lock2BFree.addAllow(new LockSnapshot(10,2,4,-1,4), 3); // lock 1 in state 4
    lock2BFree.addAllow(new LockSnapshot(10,-1,-1,-1,0), 6); // lock 4 in state 0
    pvObj.addError(&lock2BFree);
    
    StoppingState lock6FFree(startPoint) ;
    lock6FFree.addAllow(new LockSnapshot(-1,-1,-1,-1,0), 2); // lock 0 in state 0
    lock6FFree.addAllow(new LockSnapshot(10,0,1,-1,4), 7); // lock 5 in state 4
    pvObj.addError(&lock6FFree);
    
    StoppingState lock6BFree(startPoint) ;
    lock6BFree.addAllow(new LockSnapshot(-1,-1,-1,-1,0), 3); // lock 1 in state 0
    lock6BFree.addAllow(new LockSnapshot(10,0,1,-1,4), 7); // lock 5 in state 4
    pvObj.addError(&lock6BFree);
    
    // ================
    // Two masters enter state 4: cannot happen
    StoppingState lock35(startPoint) ;
    lock35.addAllow(new LockSnapshot(10,0,1,-1,4), 4); // lock 2 in state 4
    lock35.addAllow(new LockSnapshot(10,1,3,-1,4), 6); // lock 4 in state 4
    pvObj.addError(&lock35) ;
    
    StoppingState lock23(startPoint) ;
    lock23.addAllow(new LockSnapshot(10,2,4,-1,4), 3); // lock 1 in state 4
    lock23.addAllow(new LockSnapshot(10,0,1,-1,4), 4); // lock 2 in state 4
    pvObj.addError(&lock23) ;
    
    StoppingState lock25(startPoint) ;
    lock25.addAllow(new LockSnapshot(10,2,4,-1,4), 3); // lock 1 in state 4
    lock25.addAllow(new LockSnapshot(10,1,3,-1,4), 6); // lock 4 in state 4
    pvObj.addError(&lock25) ;
    
    StoppingState lock26(startPoint) ;
    lock26.addAllow(new LocnLockskSnapshot(10,2,4,-1,4), 3); // lock 1 in state 4
    lock26.addAllow(new LockSnapshot(10,0,1,-1,4), 7); // lock 5 in state 4
    pvObj.addError(&lock26) ;
    
    StoppingState lock36(startPoint) ;
    lock36.addAllow(new LockSnapshot(10,0,1,-1,4), 4); // lock 2 in state 4
    lock36.addAllow(new LockSnapshot(10,0,1,-1,4), 7); // lock 5 in state 4
    pvObj.addError(&lock36) ;
    
    StoppingState lock56(startPoint) ;
    lock56.addAllow(new LockSnapshot(10,1,3,-1,4), 6); // lock 4 in state 4
    lock56.addAllow(new LockSnapshot(10,0,1,-1,4), 7); // lock 5 in state 4
    pvObj.addError(&lock56) ;
    */
    
    pvObj.addPrintStop(printStop) ;
    ProbVerifierConfig config;
    //config.disableTraceback();
    config.setLowProbBound(0.0001);
    config.setBoundMethod(DFS_TWO_STEP);
    config.setMaxClass(2);
    pvObj.configure(config);
    
    // Start the procedure of probabilistic verification.
    // Specify the maximum probability depth to be explored
    pvObj.start(2, new GlobalState(&start_point));
    
    //srvc->printTraversed();
      
  } catch( runtime_error& re ) {
      cerr << "Runtime error:" << endl
      << re.what() << endl ;
  } catch (exception e) {
      cerr << e.what() << endl;
  } catch (...) {
      cerr << "Something wrong." << endl;
  }
  return 0;
}
