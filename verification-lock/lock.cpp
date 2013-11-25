//
//  lock.cpp
//  Prob_verify
//
//  Created by Shou-pon Lin on 8/24/12.
//  Copyright (c) 2012 Shou-pon Lin. All rights reserved.
//
#include <sstream>
#include <string>
using namespace std;

#include "lock.h"

Lock::Lock(int k, int num, Lookup* msg, Lookup* mac)
:_id(k), _range(num), StateMachine(msg,mac)
{
    // The name of the lock is "lock(i)", where i is the id of the machine
    _name = Lock_Utils::getLockName(_id);
    _machineId = machineToInt(_name);
    reset();
}


int Lock::transit(MessageTuple* inMsg, vector<MessageTuple*>& outMsgs,
                  bool& high_prob, int startIdx )
{
    outMsgs.clear();

    if( startIdx != 0 )
        return -1 ;
        
    high_prob = true ;
    
    string msg = IntToMessage(inMsg->destMsgId() ) ;
    switch ( _current ) {
        case 0 :
            if( msg == "REQUEST" ) {        // become slave
                // Assignments
                int i = inMsg->getParam(0) ;
                int t = inMsg->getParam(2) ;
                _ts = t ;
                _m = i;
                // Response
                MessageTuple* response = createResponse("LOCKED", "channel",
                                                        inMsg, _m, _ts);
                outMsgs.push_back(response);
                // Change State
                _current = 1;
                
                return 3;
            }
            else if( msg == "init" ) {      // become master
                assert( inMsg->numParams() == 3 ) ;
                // Assignments
                _ts = inMsg->getParam(0);
                _f = inMsg->getParam(1);
                _b = inMsg->getParam(2);
                // Response
                MessageTuple* rFront = createResponse("REQUEST", "channel", inMsg, _f,_ts);
                MessageTuple* rBack = createResponse("REQUEST", "channel", inMsg, _b,_ts);
                outMsgs.push_back(rFront);
                outMsgs.push_back(rBack);
                // Change state
                _current = 2;
                
                return 3;
            }
            else if( msg == "LOCKED" || msg == "FAILED" || msg == "timeout" ) {
                // Do nothing
                return 3;
            }
            else if( toIgnore(inMsg,outMsgs) ) {
                // Do nothing 
                return 3;
            }
            break;
        
        case 1:
            if( msg == "init" ) {
                return 3 ;
            }
            else if( msg == "REQUEST" ) {
                int j = inMsg->getParam(0) ;
                
                if( j == _m ) {
                    int newTs = inMsg->getParam(2) ;
                    if( newTs < _ts ) {
                        // this is not always satisfied when the messages are not delivered
                        // in the order of their transmission 
                        // throw logic_error("Newly arrival REQUEST has smaller timestamp");
                        // ignore the message instead
                        return 3;
                    }
                    /*
                    // Assignments
                    // update ts
                    _ts = newTs ;
                    
                    // Response
                    if( _m != _id ) {
                        MessageTuple* response = createResponse("LOCKED", "channel",
                                                                inMsg, _m, _ts);
                        outMsgs.push_back(response);
                    } */
                    // ignore if there are more REQ's from the same master
                    return 3 ;
                }
                else {
                    toDeny(inMsg, outMsgs);
                    
                    return 3;
                }
            }
            else if( msg == "timeout" ) {
                assert( inMsg->subjectId() == machineToInt("controller") ) ;
                int time = inMsg->getParam(0);
                if( time == _ts ) {
                    // Change state
                    _current = 0;
                    reset();
                }
                else {
                    // Ignore the timeout message, since this lock is no longer locked
                    // by another master
                }
                return 3;
            }
            
        case 2:
            if( msg == "LOCKED" ) {
                if( inMsg->getParam(2) == _ts ){
                    if( inMsg->getParam(0) == _f) {
                        // Change state
                        _current = 3;
                        
                        return 3;
                    }
                    else if( inMsg->getParam(0) == _b ) {
                        // Change state
                        _current = 13;
                        
                        return 3;
                    }
                }
                else {
#ifdef VERBOSE_ACTIONS
                    cout << "Received LOCKED from earlier transaction (t="
                    << inMsg->getParam(2)
                    << "), current transaction t=" << _t << endl;
#endif
                    // Ignore the outdated LOCKED message
                    return 3;
                }
            }
            else if( toAbort(inMsg, outMsgs) ) {
                return 3;
            }
            else if( toTimeout(inMsg, outMsgs) ) {
                return 3;
            }
            else if( toDeny(inMsg, outMsgs) ) {
                return 3;
            }
            else if( toIgnore(inMsg,outMsgs) ) {
                // Do nothing
                return 3;
            }
            
            break;
            
        case 3: // State s_3
            if( msg == "LOCKED" ) {
                if( inMsg->getParam(2) == _ts ) {
                    if( inMsg->getParam(0) == _b ) {
                        // Change state
                        _current = 4;
                        
                        return 3;
                    }
                }
                else {
                    // outdated LOCKED, ignore
                    return 3;
                }
            }
            else if( toAbort(inMsg, outMsgs) ) {
                return 3;
            }
            else if( toTimeout(inMsg, outMsgs) ) {
                return 3;
            }
            else if( toDeny(inMsg, outMsgs) ) {
                return 3;
            }
            else if( toIgnore(inMsg,outMsgs) ) {
                // Do nothing
                return 3;
            }
            
            break;
            
        case 13: // State s_3'
            if( msg == "LOCKED" ) {
                if( inMsg->getParam(2) == _ts ) {
                    if( inMsg->getParam(0) == _f ) {
                        // Change state
                        _current = 4;
                        
                        return 3;
                    }
                }
                else {
                    // outdated LOCKED, ignore
                    return 3;
                }
            }
            else if( toAbort(inMsg, outMsgs) ) {
                return 3;
            }
            else if( toTimeout(inMsg, outMsgs) ) {
                return 3;
            }
            else if( toDeny(inMsg, outMsgs) ) {
                return 3;
            }
            else if( toIgnore(inMsg,outMsgs) ) {
                // Do nothing
                return 3;
            }
            
            break;
            
        case 4:
            if( toTimeout(inMsg, outMsgs) ) {
                return 3;
            }
            else if( toDeny(inMsg, outMsgs) ) {
                return 3;
            }
            else if( toIgnore(inMsg,outMsgs) ) {
                // Do nothing
                return 3;
            }
            else if( msg == "LOCKED" ) {
                // Simply ignore this message
                return 3;
            }

            break;
            
        default:
            return -1;
            break;
    }
    
    return -1;
}

int Lock::nullInputTrans(vector<MessageTuple*>& outMsgs,
                   bool& high_prob, int startIdx)
{
    outMsgs.clear();
    if( startIdx == 0 ) {
        if( _current == 4 ) {
            MessageTuple* ctrlRes = new LockMessage(0, machineToInt("controller"),
                                                    0, messageToInt("complete"),
                                                    _machineId, _id, -1, 0);
            outMsgs.push_back(ctrlRes);
            // Change State
            _current = 0;
            reset();
            
            return 3;
            
        }
    }
    return -1;
}

void Lock::restore(const StateSnapshot* snapshot)
{
    assert( typeid(*snapshot) == typeid(LockSnapshot) ) ;
    
    const LockSnapshot* sslock = dynamic_cast<const LockSnapshot*>(snapshot);
    _ts = sslock->_ss_ts;
    _f = sslock->_ss_f ;
    _b = sslock->_ss_b ;
    _m = sslock->_ss_m ;
    _current = sslock->_stateId;
}

StateSnapshot* Lock::curState()
{
    return new LockSnapshot(_ts, _f, _b, _m, _current);
}


void Lock::reset()
{
    _ts = -1 ;
    _f = -1 ;
    _b = -1 ;
    _m = -1 ;
    _current = 0 ;
}

MessageTuple* Lock::createResponse(string msg, string dst, MessageTuple* inMsg,
                                   int toward, int time )
{
    int outMsgId = messageToInt(msg);
    int dstId = machineToInt(dst);
    
    assert(inMsg->destId() == _machineId);
    assert(toward < _range) ;
    
    MessageTuple* ret = new LockMessage(inMsg->subjectId(), dstId,
                                        inMsg->destMsgId(), outMsgId,
                                        _machineId, _id, toward, time);
    return ret;
}

bool Lock::toAbort(MessageTuple *inMsg, vector<MessageTuple *> &outMsgs)
{
    string msg = IntToMessage(inMsg->destMsgId() ) ;
    assert( outMsgs.size() == 0 );
    
    int from = inMsg->getParam(0);
    if( msg == "FAILED" ) {
        if(inMsg->getParam(2) == _ts ) {
            if( from == _f ) {
                outMsgs.push_back(abortMsg());
            }
            else if( from == _b ) {
                outMsgs.push_back(abortMsg());
            }
        }
        else {
            // ignore the message with wrong timestamp
            return true ;
        }
    }
    
    if( outMsgs.size() != 0 ) {
        _current = 0;
        reset();
        return true ;
    }
    else
        return false;
}

bool Lock::toDeny(MessageTuple* inMsg, vector<MessageTuple*>& outMsgs)
{
    string msg = IntToMessage(inMsg->destMsgId() ) ;
    assert( outMsgs.size() == 0 );
    
    if( msg == "REQUEST" ) {
        // Assignments
        int newt = inMsg->getParam(2); // extract timestamp
        int j = inMsg->getParam(0) ; // extract id of the source lock
        
        // False
        // Response
        MessageTuple* response = createResponse("FAILED", "channel",
                                                inMsg, j, newt );
        outMsgs.push_back(response);
        // Change state
        // no state change
        
        return true;
    }
    return false;
}

bool Lock::toIgnore(MessageTuple* inMsg, vector<MessageTuple*>& outMsgs)
{
    string msg = IntToMessage(inMsg->destMsgId() ) ;
    assert( outMsgs.size() == 0 );
    
    if( msg == "RELEASE" ) {
        return true;
    }
    
    return false;
}

bool Lock::toTimeout(MessageTuple *inMsg, vector<MessageTuple *> &outMsgs)
{
    string msg = IntToMessage(inMsg->destMsgId() ) ;
    assert( outMsgs.size() == 0 );
    
    if( msg == "timeout" ) {
        if( inMsg->getParam(0) == _ts ) {
            _current = 0;
            reset();
            return true;
        }
        else {
            return true ;
        }
    }
    
    return false ;
}

LockMessage* Lock::abortMsg()
{
    return new LockMessage(0, machineToInt("controller"),
                           0, messageToInt("abort"),
                           _machineId, _id, -1, _ts);
}

LockMessage* LockMessage::clone() const
{
    return new LockMessage(*this) ;
}

string LockMessage::toString()
{
    stringstream ss ;
    ss << MessageTuple::toString() << "(k=" << _k << ")" ;
    return ss.str() ;
}

string LockSnapshot::toString()
{
    stringstream ss;
    ss << _stateId << "(" ;
    
    ss << _ss_f << "," << _ss_b << "," << _ss_m << ")" ;
    return ss.str() ;
}