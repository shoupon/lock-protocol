//
//  main.cpp
//  verification-lock
//
//  Created by Shou-pon Lin on 11/19/13.
//  Copyright (c) 2013 Shou-pon Lin. All rights reserved.
//

#include <iostream>
#include <vector>
#include <exception>
#include <stdexcept>
using namespace std;

#include "parser.h"
#include "pverify.h"
#include "define.h"
#include "sync.h"

#include "lock.h"
#include "channel.h"
#include "lock_utils.h"
#include "controller.h"
#include "lock_service.h"

ProbVerifier pvObj ;
GlobalState* startPoint;

bool printStop(GlobalState* left, GlobalState* right)
{
    return true;
}

int nLocks = 6;
int nParty = nLocks + 2; // Locks + Controller + Channel

int main( int argc, char* argv[] )
{
    try {
        // Declare the names of component machines so as to register these names as id's in
        // the parser
        Parser* psrPtr = new Parser() ;
        // Create StateMachine objects
        // Add the state machines into ProbVerifier
        // Register the machines that are triggered by deadline (sync)
        StateMachine::setLookup(psrPtr->getMsgTable(), psrPtr->getMacTable()) ;
        Sync* sync = new Sync(nParty, psrPtr->getMsgTable(), psrPtr->getMacTable() );
        pvObj.addMachine(sync);
        
        // Create StateMachine objects
        Controller* ctrl = new Controller(psrPtr->getMsgTable(), psrPtr->getMacTable(),
                                          nLocks);
        sync->addMachine(ctrl);
        
        vector<bool> active(nLocks, false) ;
        vector<vector<pair<int,int> > > nbrs(nLocks);
        nbrs[2].push_back(make_pair(0,1)) ;
        nbrs[4].push_back(make_pair(1,3)) ;
        nbrs[1].push_back(make_pair(2,4)) ;
        nbrs[5].push_back(make_pair(0,1)) ;
        ctrl->setActives(2);
        //ctrl->setActives(4);
        //ctrl->setActives(1);
        //ctrl->setActives(5);
        ctrl->setNbrs(nbrs);
        
        vector<Lock*> arrLock ;
        for( size_t i = 0 ; i < nLocks ; ++i ) {
            arrLock.push_back( new Lock((int)i,nLocks,psrPtr->getMsgTable(),
                                        psrPtr->getMacTable() ) );
            sync->addMachine(arrLock[i]);
        }
        Channel* chan = new Channel(nLocks, psrPtr->getMsgTable(),
                                    psrPtr->getMacTable() ) ;
        sync->addMachine(chan);

        // Add the state machines into ProbVerifier
        pvObj.addMachine(ctrl);
        for( size_t i = 0 ; i < arrLock.size() ; ++i )
            pvObj.addMachine(arrLock[i]);
        pvObj.addMachine(chan);
        
        LockService *srvc = new LockService(2,0,1);
        //Service *srvc = new Service();
        srvc->reset();
        
        // Specify the starting state
        GlobalState::setService(srvc);
        GlobalState* startPoint = new GlobalState(pvObj.getMachinePtrs());
        startPoint->setParser(psrPtr);
        
        // Specify the global states in the set RS (stopping states)
        // initial state
        StoppingState stopZero(startPoint);
        stopZero.addAllow(new LockSnapshot(-1,-1,-1,-1,0), 2); // lock 0
        stopZero.addAllow(new LockSnapshot(-1,-1,-1,-1,0), 3); // lock 1
        stopZero.addAllow(new LockSnapshot(-1,-1,-1,-1,0), 4); // lock 2
        stopZero.addAllow(new LockSnapshot(-1,-1,-1,-1,0), 5); // lock 3
        stopZero.addAllow(new LockSnapshot(-1,-1,-1,-1,0), 6); // lock 4
        stopZero.addAllow(new LockSnapshot(-1,-1,-1,-1,0), 7); // lock 5
        //stopZero.addAllow(new ChannelSnapshot(), 7); // channel
        pvObj.addSTOP(&stopZero);
        
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
        lock26.addAllow(new LockSnapshot(10,2,4,-1,4), 3); // lock 1 in state 4
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
        
        
        pvObj.addPrintStop(printStop) ;
        //pvObj.addPrintStop();
        
        // Start the procedure of probabilistic verification.
        // Specify the maximum probability depth to be explored
        pvObj.start(9);
        
        // When complete, deallocate all machines
        delete ctrl ;
        for( size_t i = 0 ; i < arrLock.size() ; ++i )
            delete arrLock[i];
        delete chan ;
        delete sync;
        
        srvc->printTraversed();
        delete srvc;
        
        delete startPoint;
            
        delete psrPtr;
    
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