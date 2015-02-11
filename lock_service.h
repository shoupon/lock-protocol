//
//  lock_service.h
//  verification-lock
//
//  Created by Shou-pon Lin on 11/19/13.
//  Copyright (c) 2013 Shou-pon Lin. All rights reserved.
//

#ifndef LOCK_SERVICE_H
#define LOCK_SERVICE_H

#include <iostream>
#include <string>
#include <sstream>
using namespace std;

#include "../prob_verify/service.h"
#include "../prob_verify/statemachine.h"

class LockService: public Service {
    const int _m;
    const int _f;
    const int _b;
    int _d; // the deadline number being monitored
    
    string _mName;
    string _fName;
    string _bName;
    
    int _mid;
    int _fid;
    int _bid;
    int _cid;
    
    int _mFreed;
    int _fFreed;
    int _bFreed;
    
    int _stateFreed;
public:
    LockService(int m, int f, int b);
    int transit(MessageTuple* inMsg, vector<MessageTuple*>& outMsgs,
                bool& high_prob, int startIdx = 0);
    void restore(const StateSnapshot* snapshot);
    ServiceSnapshot* curState();
    
    bool isMonitored(MessageTuple *inMsg);
private:
    bool free(int i, int state);
};

class LockServiceSnapshot: public ServiceSnapshot {
    friend LockService;
    
    int _ss_m;
    int _ss_f;
    int _ss_b;
    int _ss_d;
    
    int _ss_mFreed;
    int _ss_fFreed;
    int _ss_bFreed;
    
    int _ss_stateFreed;
public:
    LockServiceSnapshot(int m, int f, int b, int d, int state,
                        int mf, int ff, int bf, int sf);
    string toString();
    ServiceSnapshot* clone() const ;
};

#endif /* defined(LOCK_SERVICE_H) */
