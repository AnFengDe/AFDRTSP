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
#include "BasicUsageEnvironment.hh"

class AFDPollThread : private jthread::JThread  
{
public:
    AFDPollThread(UsageEnvironment*& rtspenv);
    ~AFDPollThread();
    int Start();
    void Stop();
private:
    void *Thread();
    UsageEnvironment* env;    
    char stop;
    jthread::JMutex stopmutex;
};
#endif
