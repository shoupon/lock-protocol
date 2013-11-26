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
using namespace std;

#include "service.h"
#include "statemachine.h"

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
public:
    LockService(int m, int f, int b);
    int transit(MessageTuple* inMsg, vector<MessageTuple*>& outMsgs,
                bool& high_prob, int startIdx = 0);
    bool isMonitored(MessageTuple *inMsg);
};

class LockServiceSnapshot: public StateSnapshot {
    friend LockService;
    
    int _ss_m;
    int _ss_f;
    int _ss_b;
    int _ss_d;
    int _ss_state;
public:
    LockServiceSnapshot(int m, int f, int b, int d, int state);
};

#endif /* defined(LOCK_SERVICE_H) */
