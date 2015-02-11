//
//  lock_service.cpp
//  verification-lock
//
//  Created by Shou-pon Lin on 11/19/13.
//  Copyright (c) 2013 Shou-pon Lin. All rights reserved.
//

#include "lock_service.h"
#include "lock_utils.h"

#include "lock.h"
#include "controller.h"

LockService::LockService(int m, int f, int b)
: Service(), _m(m), _f(f), _b(b)
{
    setId(machineToInt("lockservice"));
    reset();
    
    _mName = Lock_Utils::getLockName(m);
    _fName = Lock_Utils::getLockName(f);
    _bName = Lock_Utils::getLockName(b);
    _mid = machineToInt(_mName);
    _fid = machineToInt(_fName);
    _bid = machineToInt(_bName);
    _cid = machineToInt("channel");
    _mFreed = _fFreed = _bFreed = 1;
    _stateFreed = -1;
    
    ControllerMessage* initMsg = new ControllerMessage(0,machineToInt(_mName),
                                                       0,messageToInt("init"),
                                                       machineToInt("controller"));
    _interface.insert(initMsg);
    
    int time = 0;   // time is not revelant here
    LockMessage* fGranted
        = new LockMessage(_cid, machineToInt("channel"),
                          messageToInt("REQUEST"), messageToInt("LOCKED"),
                          _fid, f, m, time);
    _interface.insert(fGranted);
    
    LockMessage* bGranted
        = new LockMessage(_cid, machineToInt("channel"),
                          messageToInt("REQUEST"), messageToInt("LOCKED"),
                          _bid, b, m, time);
    _interface.insert(bGranted);
    
    LockMessage* fFree
        = new LockMessage(machineToInt("sync"), machineToInt("controller"),
                          messageToInt("DEADLINE"), messageToInt("free"),
                          _fid, f, m, time);
    _interface.insert(fFree);
    
    LockMessage* bFree
        = new LockMessage(machineToInt("sync"), machineToInt("controller"),
                          messageToInt("DEADLINE"), messageToInt("free"),
                          _bid, b, m, time);
    _interface.insert(bFree);
    
    LockMessage* mFree
        = new LockMessage(machineToInt("sync"), machineToInt("controller"),
                          messageToInt("DEADLINE"), messageToInt("free"),
                          _mid, m, m, time);
    _interface.insert(mFree);
    
    LockMessage* fsuccess
        = new LockMessage(machineToInt("channel"), machineToInt("controller"),
                          messageToInt("LOCKED"), messageToInt("success"),
                          _mid, f, m, time);
    _interface.insert(fsuccess);
    
    LockMessage* bsuccess
        = new LockMessage(machineToInt("channel"), machineToInt("controller"),
                          messageToInt("LOCKED"), messageToInt("success"),
                          _mid, b, m, time);
    _interface.insert(bsuccess);
}

int LockService::transit(MessageTuple* inMsg, vector<MessageTuple*>& outMsgs,
                         bool& high_prob, int startIdx)
{
    if( !isMonitored(inMsg) )
        return 3;
    
    string msg = IntToMessage(inMsg->destMsgId());
    
    switch ( _state ) {
        case 0 :
            if( msg == "init" ) {
                assert( inMsg->numParams() == 3 ) ;
                // Assignments
                _d = inMsg->getParam(0);
                // Change state
                _state = 1;
                _mFreed = 0;
                assert(_stateFreed<0);
                _traversed.insert("01");
                return 3;
            }
            else {
                assert(false);
            }
            break;
        case 1:
            if( msg == "LOCKED" ) {
                // intercept the messages that are put into channel
                if( machineToInt("channel") == inMsg->destId() ) {
                    int from = inMsg->getParam(0);
                    if (from == _f) {
                        _state = 2;
                        _fFreed = 0;
                        assert(_stateFreed<0);
                        _traversed.insert("12");
                        return 3;
                    }
                    else if(from == _b) {
                        _state = 3;
                        _bFreed = 0;
                        assert(_stateFreed<0);
                        _traversed.insert("13");
                        return 3;
                    }
                    else
                        assert(false);
                }
            }
            else if( msg == "free" ) {
                if (free(inMsg->getParam(1), 1)) {
                    _state = 0;
                    _traversed.insert("10");
                }
                return 3;
            }
            else
                assert(false);
            break;
        case 2:
            if( msg == "LOCKED" ) {
                if( machineToInt("channel") == inMsg->destId() ) {
                    if( inMsg->getParam(0) == _b ) {
                        _state = 4;
                        _bFreed = 0;
                        assert(_stateFreed<0);
                        _traversed.insert("24");
                        return 3;
                    }
                    else
                        assert(false);
                }
                else
                    assert(false);
            }
            else if( msg == "free" ) {
                if (free(inMsg->getParam(1), 2)) {
                    _state = 0;
                    _traversed.insert("20");
                }
                return 3;
            }
            else
                assert(false);
            break;
        case 3:
            if( msg == "LOCKED" ) {
                if( machineToInt("channel") == inMsg->destId() ) {
                    if( inMsg->getParam(0) == _f ) {
                        _state = 4;
                        _fFreed = 0;
                        assert(_stateFreed<0);
                        _traversed.insert("34");
                        return 3;
                    }
                    else
                        assert(false);
                }
                else
                    assert(false);
            }
            else if( msg == "free") {
                if (free(inMsg->getParam(1), 3)) {
                    _state = 0;
                    _traversed.insert("30");
                }
                return 3;
            }
            else
                assert(false);
            break;
        case 4:
            if( msg == "success" ) {
                _state = 5;
                _traversed.insert("45");
                return 3;
            }
            else if( msg == "free" ) {
                if (free(inMsg->getParam(1), 4)) {
                    _state = 0;
                    _traversed.insert("40");
                }
                return 3;
            }
            else
                assert(false);
        case 5:
            if( msg == "free" ){
                if (free(inMsg->getParam(1), 5)) {
                    _state = 0;
                    _traversed.insert("50");
                }
                return 3;
            }
            else
                assert(false);
            break;
        default:
            assert(false);
            break;
    }
    return -1;
}

bool LockService::isMonitored(MessageTuple *inMsg)
{
    if (Service::isMonitored(inMsg)) {
        // Check m, f, b, deadline here
        string msg = IntToMessage(inMsg->destMsgId() ) ;
        int to = inMsg->getParam(1);
        int from = inMsg->getParam(0);
        if (msg == "init") {
            _d = inMsg->getParam(0);
            return true;
        }
        else if (msg == "free") {
            if(from != _m && from != _f && from != _b)
                return false;
            else if (to != from)
                return false;
            else
                return true;
        }
        else {
            if (_d != inMsg->getParam(2))
                return false;
            
            if (to != _m)
                return false;
            else if (msg == "success")
                return true;
            else if (msg == "LOCKED") {
                if (from != _f && from != _b)
                    return false;
                else
                    return true;
            }
            else
                return false;
        }
        return true;
    }
    else
        return false;
}

void LockService::restore(const StateSnapshot* snapshot)
{
    assert(typeid(*snapshot) == typeid(LockServiceSnapshot));
    const LockServiceSnapshot *ss
        = dynamic_cast<const LockServiceSnapshot*>(snapshot);

    _d = ss->_ss_d;
    _state = snapshot->curStateId();
    _mFreed = ss->_ss_mFreed;
    _fFreed = ss->_ss_fFreed;
    _bFreed = ss->_ss_bFreed;
    _stateFreed = ss->_ss_stateFreed;
}

bool LockService::free(int i, int state)
{
    if (_stateFreed != -1)
        assert(state == _stateFreed);
    else
        _stateFreed = state;
    
    if (i == _m)
        _mFreed = 1;
    else if (i == _f)
        _fFreed = 1;
    else if (i == _b)
        _bFreed = 1;
    
    if (_mFreed & _fFreed & _bFreed) {
        _stateFreed = -1;
        return true;
    }
    else
        return false;
}

ServiceSnapshot* LockService::curState()
{
    return new LockServiceSnapshot(_m, _f, _b, _d, _state,
                                   _mFreed, _fFreed, _bFreed, _stateFreed);
}

LockServiceSnapshot::LockServiceSnapshot(int m, int f, int b, int d, int state,
                                         int mf, int ff, int bf, int sf)
:ServiceSnapshot(state), _ss_m(m), _ss_f(f), _ss_b(b), _ss_d(d),
 _ss_mFreed(mf), _ss_fFreed(ff), _ss_bFreed(bf), _ss_stateFreed(sf)
{
    ;
}

string LockServiceSnapshot::toString()
{
    stringstream ss;
    ss << "(" << _ss_m << "," << _ss_f << "," << _ss_b << "," << _ss_d
       << "," << _ss_state
       << "," << _ss_mFreed << "," << _ss_fFreed << "," << _ss_bFreed << ")" ;
    return ss.str();
}

ServiceSnapshot* LockServiceSnapshot::clone() const
{
    return new LockServiceSnapshot(_ss_m, _ss_f, _ss_b, _ss_d, _ss_state,
                                   _ss_mFreed, _ss_fFreed, _ss_bFreed,
                                   _ss_stateFreed);
}