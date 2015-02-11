//
//  lock_service.h
//  Prob_verify
//
//  Created by Shou-pon Lin on 8/27/12.
//  Copyright (c) 2012 Shou-pon Lin. All rights reserved.
//

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <string>
#include <vector>
#include <queue>
#include <bitset>
#include <iostream>
using namespace std;

#include "../prob_verify/statemachine.h"
#include "lock_utils.h"
#include "competitor.h"
#include "lock.h"

class ControllerMessage;
class ControllerSnapshot;

typedef pair<int,int> Neighbor;

class Controller: public StateMachine
{
public:
    Controller( Lookup* msg, Lookup* mac, int num) ;
    int transit(MessageTuple* inMsg, vector<MessageTuple*>& outMsgs,
                bool& high_prob, int startIdx ) ;   
    int nullInputTrans(vector<MessageTuple*>& outMsgs,
                       bool& high_prob, int startIdx ) ;   
    void restore(const StateSnapshot* snapshot) ;
    StateSnapshot* curState() ;   
    void reset() ;

    // Generate all possible sets of neighbors
    void allNbrs();
    // Specify the possible neighbors that are being tested
    static void setNbrs(vector<vector<Neighbor> >& nbrs) { _nbrs = nbrs; }
    // Specify the competitors that will be tested; The default is true for all vehicles
    void setActives(int idx) ;
private:
    const int _numVehs ;
    string _name;
    
    // _nbrs[i] contains the possible neighbors set of veh[i]
    static vector<vector<Neighbor> > _nbrs ;
    // when _actives[i] = true, vehicle i is able to initiate a merge
    //                   false, vehicle i cannot start a merge
    vector<bool> _actSetting;
    static vector<bool> _actives;

    // State variables
    // Indicates whether the competitor with the vector subscript is busy
    // negative value means idle. positive values record the starting timestamp of the lock
    vector<int> _busy;
    // The relative but not absolute time stamp for each event
    int _time ;
};


class ControllerMessage: public MessageTuple
{
public:
    ControllerMessage(int src, int dest, int srcMsg, int destMsg, int subject)
    :MessageTuple(src,dest,srcMsg,destMsg,subject) {};
    
    ControllerMessage(int src, int dest, int srcMsg, int destMsg, int subject,
                      int master)
    :MessageTuple(src,dest,srcMsg,destMsg,subject) {_timestamp = master;}

    ControllerMessage(const ControllerMessage& item)
    :MessageTuple(item)
    , _nParams(item._nParams), _front(item._front), _back(item._back), _timestamp(item._timestamp) {}

    ~ControllerMessage() {}
    
    size_t numParams() { return _nParams; }
    int getParam(size_t arg) ;        
    ControllerMessage* clone() const { return new ControllerMessage(*this); }
    
    void addParams(int time, int front, int back);   
protected:
    int _nParams;
    int _params[3];
    int& _timestamp = _params[0];
    int& _front = _params[1];
    int& _back = _params[2];
};

class ControllerSnapshot: public StateSnapshot
{
    friend class Controller;
public:
    ControllerSnapshot(vector<int> busy, int time);
    ControllerSnapshot(const ControllerSnapshot& item)
    :_ss_busy(item._ss_busy), _ss_time(item._ss_time) {}
    ~ControllerSnapshot() {}
    int curStateId() const { return 0; }
    // Returns the name of current state as specified in the input file
    string toString();
    // Cast the Snapshot into a integer. Used in HashTable
    int toInt() { return _hash_use ; }
    ControllerSnapshot* clone() const ;

private:
    vector<int> _ss_busy;
    int _ss_time ;
    int _hash_use ;
};


#endif
