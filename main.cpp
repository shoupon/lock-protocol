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

#define SCENARIO 3

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

void setupDeniedState(StoppingState& stop, int m) {
  stop.addAllow(new LockSnapshot(5, m), locks[m].getName());
  for (auto &c : channels[m])
    stop.addAllow(new ChannelSnapshot(), c->getName());
}

int nLocks = 6;

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

    // Create StateMachine objects
    Lock::setNumLocks(nLocks);
    locks.clear();
    locks.push_back(Lock(0));
#if (SCENARIO >= 3)
    locks.push_back(Lock(1, 2, 4));
#else
    locks.push_back(Lock(1));
#endif
#if (SCENARIO >= 1)
    locks.push_back(Lock(2, 0, 1));
#else
    locks.push_back(Lock(2));
#endif
    locks.push_back(Lock(3));
#if (SCENARIO >= 2)
    locks.push_back(Lock(4, 1, 3));
#else
    locks.push_back(Lock(4));
#endif
#if (SCENRAIO >= 4)
    locks.push_back(Lock(5, 0, 1));
#else
    locks.push_back(Lock(5));
#endif
    for (auto& l : locks) {
      pvObj.addMachine(&l);
    }

    channels.clear();
    for (int i = 0; i < nLocks; ++i) {
      channels.resize(i + 1);
      for (int j = 0; j < nLocks; ++j) {
        if (i != j) {
          channels[i].push_back(shared_ptr<Channel>(new Channel(i, j)));
          pvObj.addMachine(channels[i].back().get());
        }
      }
    }

    //LockService *srvc = new LockService(2,0,1);
    Service srvc;
    StateMachine::dumpMachineTable();
    
    // Specify the starting state
    GlobalState::setService(&srvc);
    GlobalState start_point(pvObj.getMachinePtrs());
    
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
#if (SCENARIO >= 1)
    StoppingState stop_group1_locked(&start_point);
    setupLockedState(stop_group1_locked, 2, 0, 1);
    pvObj.addSTOP(&stop_group1_locked);

    StoppingState stop_group1_denied(&start_point);
    setupDeniedState(stop_group1_denied, 2);
    pvObj.addSTOP(&stop_group1_denied);
#endif

#if (SCENARIO >= 2)
    StoppingState stop_group2_locked(&start_point);
    setupLockedState(stop_group2_locked, 4, 1, 3);
    pvObj.addSTOP(&stop_group2_locked);

    StoppingState stop_group2_denied(&start_point);
    setupDeniedState(stop_group2_denied, 4);
    pvObj.addSTOP(&stop_group2_denied);
#endif
#if (SCENARIO >= 3)
    StoppingState stop_group3_locked(&start_point);
    setupLockedState(stop_group3_locked, 1, 2, 4);
    pvObj.addSTOP(&stop_group3_locked);

    StoppingState stop_group3_denied(&start_point);
    setupDeniedState(stop_group3_denied, 1);
    pvObj.addSTOP(&stop_group3_denied);
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
    config.setLowProbBound(0.0001);
    config.setBoundMethod(DFS_BOUND);
    pvObj.configure(config);
    
    // Start the procedure of probabilistic verification.
    // Specify the maximum probability depth to be explored
    pvObj.start(7, startPoint);
    
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
