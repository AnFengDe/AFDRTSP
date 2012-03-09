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

AFDPollThread::AFDPollThread()
{
}

AFDPollThread::~AFDPollThread()
{
    Stop();
}
 
int AFDPollThread::Start(AFDTransmitter *trans)
{
#if 0
        if (JThread::IsRunning())
                    return ERR_AFD_POLLTHREAD_ALREADYRUNNING;
            
            transmitter = trans;
                if (!stopmutex.IsInitialized())
                        {
                                    if (stopmutex.Init() < 0)
                                                    return ERR_AFD_POLLTHREAD_CANTINITMUTEX;
                                        }
                    stop = false;
                        if (JThread::Start() < 0)
                                    return ERR_AFD_POLLTHREAD_CANTSTARTTHREAD;
#endif
    return 0;
}

void AFDPollThread::Stop()
{  
#if 0 
    if (!IsRunning())
        return;
            
    stopmutex.Lock();
    stop = true;
    stopmutex.Unlock();
                        
    if (transmitter)
        transmitter->AbortWait();
                           
    AFDTime thetime = AFDTime::CurrentTime();
    bool done = false;

    while (JThread::IsRunning() && !done)
    {
        // wait max 5 sec
        AFDTime curtime = AFDTime::CurrentTime();
        if ((curtime.GetDouble()-thetime.GetDouble()) > 5.0)
            done = true;
         AFDTime::Wait(AFDTime(0,10000));
    }
                                                        
    if (JThread::IsRunning())
    {
    #ifndef _WIN32_WCE
        std::cerr << "AFDPollThread: Warning! Having to kill thread!" << std::endl;
    #endif // _WIN32_WCE
        JThread::Kill();
    }

    stop = false;
    transmitter = 0;
#endif
}

void *AFDPollThread::Thread()
{
#if 0
    JThread::ThreadStarted();
    
    bool stopthread;

    stopmutex.Lock();
    stopthread = stop;
    stopmutex.Unlock();

    rtpsession.OnPollThreadStart(stopthread);

    while (!stopthread)
    {
    int status;

    rtpsession.schedmutex.Lock();
    rtpsession.sourcesmutex.Lock();
    
    AFDTime rtcpdelay = rtcpsched.GetTransmissionDelay();
    
    rtpsession.sourcesmutex.Unlock();
    rtpsession.schedmutex.Unlock();
    
    if ((status = transmitter->WaitForIncomingData(rtcpdelay)) < 0)
    {
    stopthread = true;
    rtpsession.OnPollThreadError(status);
    }
    else
    {
    if ((status = transmitter->Poll()) < 0)
    {
    stopthread = true;
    rtpsession.OnPollThreadError(status);
    }
    else
    {
    if ((status = rtpsession.ProcessPolledData()) < 0)
    {
    stopthread = true;
    rtpsession.OnPollThreadError(status);
    }
    else
    {
    rtpsession.OnPollThreadStep();
    stopmutex.Lock();
    stopthread = stop;
    stopmutex.Unlock();
    }
    }
    }
    }

    rtpsession.OnPollThreadStop();

    return 0;
#endif
}


