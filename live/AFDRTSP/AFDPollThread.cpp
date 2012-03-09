/*!
 * \author chtian
 * \brief 	
 * \file AFDPollThread.cpp
 * \version 1.0 
 * \date 2012-03-09 14:39
 * */

#include "AFDPollThread.h"

#ifndef _WIN32_WCE
    #include <iostream>
#endif // _WIN32_WCE

AFDPollThread::AFDPollThread(UsageEnvironment*& rtspenv)
    : env(rtspenv)
{
}

AFDPollThread::~AFDPollThread()
{
    Stop();
}
 
int AFDPollThread::Start()
{
    if (JThread::IsRunning())
        return ERR_JTHREAD_ALREADYRUNNING;
          
    if (!stopmutex.IsInitialized())
    {
        if (stopmutex.Init() < 0)
            return ERR_JTHREAD_CANTINITMUTEX;
    }

    stop = 0x00;

    if (JThread::Start() < 0)
        return ERR_JTHREAD_CANTSTARTTHREAD;

    return 0;
}

void AFDPollThread::Stop()
{  
    if (!IsRunning()) return;
            
    stopmutex.Lock();
    stop = 0x01;
    stopmutex.Unlock();
                        
    bool done = false;

    while (JThread::IsRunning() && !done)
    {
        // wait max 5 sec
        done = true;
        sleep(5);
    }
                                                        
    if (JThread::IsRunning())
    {
    #ifndef _WIN32_WCE
        std::cerr << "AFDPollThread: Warning! Having to kill thread!" << std::endl;
    #endif // _WIN32_WCE
        JThread::Kill();
    }

    stop = 0x00;
}

void *AFDPollThread::Thread()
{
    JThread::ThreadStarted();
    
    //never break here
    if (NULL != env) env->taskScheduler().doEventLoop(&stop);

    return 0;
}


