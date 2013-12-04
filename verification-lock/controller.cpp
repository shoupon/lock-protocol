#include "controller.h"

vector<vector<Neighbor> > Controller::_nbrs ;
vector<bool> Controller::_actives;

Controller::Controller( Lookup* msg, Lookup* mac, int num )
: StateMachine(msg, mac), _numVehs(num), _time(0)
{ 
    _name = "controller" ;
    setId(machineToInt(_name));
    _actives.resize(num,false);
    reset();
}

int Controller::transit(MessageTuple* inMsg, vector<MessageTuple*>& outMsgs,
                bool& high_prob, int startIdx ) 
{
    outMsgs.clear();
    high_prob = true ;

    if( startIdx != 0 )
        return -1;

    string msg = IntToMessage( inMsg->destMsgId() ) ;
    if( msg == "complete" || msg == "abort") {
        /*
        int veh = inMsg->getParam(0) ;
        if( typeid(*inMsg) == typeid(LockMessage) ) {
            int master = inMsg->getParam(1);
            _busy[master] = -1;
            _time++;
            return 3;
        }*/
        return 3;
    }
    else if (msg == "free") {
        int veh = inMsg->getParam(0);
        _busy[veh] = -1;
        return 3;
    }
    else if (msg == "success")
        return 3;
    else if (msg == "DEADLINE")
        return 3;
    
    return -1;
}

int Controller::nullInputTrans(vector<MessageTuple*>& outMsgs,
                               bool& high_prob, int startIdx ) 
{ 
    outMsgs.clear();
    high_prob = true ;
    if( startIdx < 0 )
        return -1;
    else if( startIdx >= 0 ) {
        // check for initiation
        size_t i = 0 ;
        int count = startIdx;
        while( i < _busy.size() ) {
            if( _busy[i] == -1 && _actives[i] == true ) {
                if (count)
                    count--;
                else {
                    // merge start event
                    int f = _nbrs[i].front().first ;
                    int b = _nbrs[i].front().second ;
                    // create response
                    int dstId = machineToInt(Lock_Utils::getLockName((int)i));
                    int dstMsgId = messageToInt("init");            
                    ControllerMessage* initMsg
                        = new ControllerMessage(0,dstId, 0,dstMsgId,macId());
                    initMsg->addParams(_time, f, b);
                    outMsgs.push_back(initMsg);                    
                    // Change state
                    _busy[i] = _time ;
                    //_time++;
                    return startIdx + 1;
                }               
            }
            ++i ;
        }
        // the startIdx exceeds the number of available vehicles
        return -1;
    }
    return -1;
}

StateSnapshot* Controller::curState() 
{
    StateSnapshot* ret = new ControllerSnapshot(_busy, _time);
    return ret ;
}

void Controller::restore(const StateSnapshot* ss)
{
    if( typeid(*ss) != typeid(ControllerSnapshot) )
        return ;

    const ControllerSnapshot* cs = dynamic_cast<const ControllerSnapshot*>(ss);
    _busy = cs->_ss_busy ;
    _time = cs->_ss_time ;
}

void Controller::reset() 
{
    _busy.clear() ;
    _busy.resize(_numVehs, -1);
    _time = 0 ;
}

void Controller::allNbrs()
{
    for( size_t i = 0 ; i < _numVehs ; ++i ) 
        _nbrs[i].clear();

    for( int i = 0 ; i < _numVehs ; ++i ) {
        for( int j = 0 ; j < _numVehs ; ++j ) {
            if( j == i )
                continue ;
            for( int  k = j+1 ; k < _numVehs ; ++k ) {
                if( k == i ) 
                    continue ;
                _nbrs[i].push_back( Neighbor(i,j) ) ;
                _nbrs[i].push_back( Neighbor(j,i) ) ;
            }
        }
    }
}

void Controller::setActives(int idx)
{
    assert(idx >= 0);
    assert((size_t)idx < _actives.size());
    _actives[idx] = true;
}
 
int ControllerMessage::getParam(size_t arg) 
{
    assert(arg >= 0);
    assert(arg <= 2);
    return _params[arg];
}

void ControllerMessage::addParams(int time, int front, int back)
{
    _timestamp = time;
    _front = front;
    _back = back;
    _nParams = 3;
}

ControllerSnapshot::ControllerSnapshot(vector<int> busy, int time)
:_ss_busy(busy), _ss_time(time)
{
    int sumEng = 0 ;
    for( size_t i = 0 ; i < _ss_busy.size() ; ++i ) {
        sumEng += _ss_busy[i];
        sumEng = sumEng << 1;
    }
    _hash_use  = (sumEng << 4) + time;
}

// Returns the name of current state as specified in the input file
string ControllerSnapshot::toString()
{
    stringstream ss;
#ifdef CONTROLLER_TIME
    ss << _ss_time << "(" ;
#else
    ss << "(" ;
#endif
    if( _ss_busy.empty() )
        ss << ")" ;
    else {
        for( size_t i = 0 ; i < _ss_busy.size() - 1 ; ++i ) {
            ss << _ss_busy[i] << "," ;
        }
        ss << _ss_busy.back() << ")" ;
    }
    return ss.str();
}

ControllerSnapshot* ControllerSnapshot::clone() const 
{
    return new ControllerSnapshot(*this) ;
}