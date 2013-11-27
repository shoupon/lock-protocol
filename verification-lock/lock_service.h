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
    void restore(const StateSnapshot* snapshot);
    ServiceSnapshot* curState();
    
    bool isMonitored(MessageTuple *inMsg);
};

class LockServiceSnapshot: public ServiceSnapshot {
    friend LockService;
    
    int _ss_m;
    int _ss_f;
    int _ss_b;
    int _ss_d;
public:
    LockServiceSnapshot(int m, int f, int b, int d, int state);
    string toString();
    ServiceSnapshot* clone() const ;
};

#endif /* defined(LOCK_SERVICE_H) */
