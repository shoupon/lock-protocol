//
//  lock_service.cpp
//  verification-lock
//
//  Created by Shou-pon Lin on 11/19/13.
//  Copyright (c) 2013 Shou-pon Lin. All rights reserved.
//

#include "lock_service.h"

LockService::LockService(int m, int f, int b)
: Service(), _m(m), _f(f), _b(b)
{
    setId(machineToInt("lockservice"));
    reset();
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
                        return 3;
                    }
                    else if(from == _b) {
                        _state = 3;
                        return 3;
                    }
                    else
                        assert(false);
                }
            }
            else if( msg == "DEADLINE" ) {
                reset();
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
                        return 3;
                    }
                    else
                        assert(false);
                }
                else
                    assert(false);
            }
            else if( msg == "DEADLINE" ) {
                reset();
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
                        return 3;
                    }
                    else
                        assert(false);
                }
                else
                    assert(false);
            }
            else if( msg == "DEADLINE") {
                reset();
                return 3;
            }
            else
                assert(false);
            break;
        case 4:
            if( msg == "SUCCESS" ) {
                _state = 5;
                return 3;
            }
            else if( msg == "DEADLINE" ) {
                reset();
                return 3;
            }
            else
                assert(false);
        case 5:
            if( msg == "DEADLINE" ){
                reset() ;
                return 3;
            }
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
        return true;
    }
    else
        return false;
}