/*!
 * \author chtian
 * \brief 	
 * \file AFDPollThread.h
 * \version 1.0 
 * \date 2012-03-09 14:30
 * */
#ifndef _AFDPOLLTHREAD_H_
#define _AFDPOLLTHREAD_H_
#include "jthread.h"
#include "jmutex.h"

class AFDPollThread : private jthread::JThread  
{
public:
    AFDPollThread();
    ~AFDPollThread();
    int Start();
    void Stop();
private:
    void *Thread();
    
    bool stop;
    jthread::JMutex stopmutex;
    //RTPTransmitter *transmitter;
    
    //RTPSession &rtpsession;
    //RTCPScheduler &rtcpsched;

};
#endif
