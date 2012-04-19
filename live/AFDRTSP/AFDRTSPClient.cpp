/**********
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 * **********/
// Copyright (c) 2012, AnFengDe Info Ltd.  All rights reserved
//#include <stdio.h>
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "AFDPollThread.h"

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);

// Implementation of "StreamClientState":
class StreamClientState 
{
public:
  StreamClientState();
  virtual ~StreamClientState();

public:
  MediaSubsessionIterator* iter;
  MediaSession* session;
  MediaSubsession* subsession;
  TaskToken streamTimerTask;
  double duration;
};

StreamClientState::StreamClientState()
  : iter(NULL), session(NULL), subsession(NULL), streamTimerTask(NULL), duration(0.0) 
{
}

StreamClientState::~StreamClientState() 
{
    delete iter;
    if (session != NULL) 
    {
        // We also need to delete "session", and unschedule "streamTimerTask" (if set)
        UsageEnvironment& env = session->envir(); // alias

        env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
        Medium::close(session);
    }
}

class ourRTSPClient: public RTSPClient 
{
public:
  static ourRTSPClient* createNew(  UsageEnvironment& env, 
                                    char const* rtspURL,
                                    int verbosityLevel = 0,
                                    char const* applicationName = NULL,
                                    portNumBits tunnelOverHTTPPortNum = 0);

protected:
  ourRTSPClient(UsageEnvironment& env, 
                char const* rtspURL,
                int verbosityLevel, 
                char const* applicationName, 
                portNumBits tunnelOverHTTPPortNum);
    // called only by createNew();
  virtual ~ourRTSPClient();

public:
  StreamClientState scs;
};

ourRTSPClient* ourRTSPClient::createNew(UsageEnvironment& env, 
                                        char const* rtspURL,
                                        int verbosityLevel, 
                                        char const* applicationName, 
                                        portNumBits tunnelOverHTTPPortNum) 
{
    return new ourRTSPClient(env, rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

ourRTSPClient::ourRTSPClient(UsageEnvironment& env, 
                             char const* rtspURL,
                             int verbosityLevel, 
                             char const* applicationName, 
                             portNumBits tunnelOverHTTPPortNum)
  : RTSPClient(env,rtspURL, verbosityLevel, applicationName, tunnelOverHTTPPortNum) 
{
}

ourRTSPClient::~ourRTSPClient() 
{
}

/// AFD RTSP client runtime struct
typedef struct _st_AFD_RTSP_Client_Runtime{
    /// library initliaze flag
    bool                init_flag;
    /// global task scheduler instance
    TaskScheduler*      scheduler;
    /// usage environment
    UsageEnvironment*   env;
    /// count how many rtsp clients are current use
    int                 client_count;
    /// work thread for asynchronous rtsp message exchange
    AFDPollThread*      pollthread;
}st_AFD_RTSP_Client_Runtime;

st_AFD_RTSP_Client_Runtime* g_client_rt = NULL;

void continueAfterPAUSE(RTSPClient* rtspClient, int resultCode, char* resultString)
{
    UsageEnvironment& env = rtspClient->envir(); // alias

    if (resultCode != 0) 
    {
        env << rtspClient->url() << ": Failed to start playing session: " << resultString << "\n";
    }
}

void continueAfterTAERDOWN(RTSPClient* rtspClient, int resultCode, char* resultString)
{
    //You can put process code here for Option.

    //(*(g_client_rt->env))<<"Put code for PAUSE command process here please.\n";

}

void shutdownStream(RTSPClient* rtspClient, int exitCode = 1) 
{
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

    // First, check whether any subsessions have still to be closed:
    if (scs.session != NULL) 
    { 
        rtspClient->sendTeardownCommand(*scs.session, NULL);
    }

    env << rtspClient->url() << ": Closing the stream.\n";
    Medium::close(rtspClient);
}

void streamTimerHandler(void* clientData) 
{
    ourRTSPClient* rtspClient = (ourRTSPClient*)clientData;
    StreamClientState& scs = rtspClient->scs; // alias

    scs.streamTimerTask = NULL;

    // Shut down the stream:
    shutdownStream(rtspClient);
}

void setupNextSubsession(RTSPClient* rtspClient) 
{
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias
  
    scs.subsession = scs.iter->next();
    if (scs.subsession != NULL) 
    {
        if (!scs.subsession->initiate()) 
        {
            env << rtspClient->url() << ": Failed to initiate the \"" << scs.subsession->mediumName() << "\" subsession: " << env.getResultMsg() << "\n";
            setupNextSubsession(rtspClient); // give up on this subsession; go to the next one
        } 
        else 
        {
            env << rtspClient->url() << ": Initiated the \"" << scs.subsession->mediumName()
	            << "\" subsession (client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1 << ")\n";

            // Continue setting up this subsession, by sending a RTSP "SETUP" command:
            rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP);
        }
        return;
    }

    // We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
    scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
    rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
}

void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString) 
{
    do 
    {
        UsageEnvironment& env = rtspClient->envir(); // alias
        StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

        if (resultCode != 0) 
        {
            env << rtspClient->url() << ": Failed to start playing session: " << resultString << "\n";
            break;
        }

        // Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its end
        // using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can later
        // 'seek' back within it and do another RTSP "PLAY" - then you can omit this code.
        // (Alternatively, if you don't want to receive the entire stream, you could set this timer for some shorter value.)
        if (scs.duration > 0) 
        {
            unsigned const delaySlop = 2; // number of seconds extra to delay, after the stream's expected duration.  (This is optional.)
            scs.duration += delaySlop;
            unsigned uSecsToDelay = (unsigned)(scs.duration*1000000);
            scs.streamTimerTask = env.taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc*)streamTimerHandler, rtspClient);
        }

        env << rtspClient->url() << ": Started playing session";
        if (scs.duration > 0) 
        {
            env << " (for up to " << scs.duration << " seconds)";
        }
        env << "...\n";

        return;
    } while (0);

    // An unrecoverable error occurred with this stream.
    shutdownStream(rtspClient);
}

// RTSP 'response handlers':
void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) 
{
    do 
    {
        UsageEnvironment& env = rtspClient->envir(); // alias
        StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

        if (resultCode != 0) 
        {
            env << rtspClient->url() << "Failed to set up the \"" << scs.subsession->mediumName() << "\" subsession: " << env.getResultMsg() << "\n";
            break;
        }

        env << rtspClient->url() << ":Set up the \"" << scs.subsession->mediumName()
	        << "\" subsession (client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1 << ")\n";

        // Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
        // (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening until later,
        // after we've sent a RTSP "PLAY" command.)

        scs.subsession->miscPtr = rtspClient; // a hack to let subsession handle functions get the "RTSPClient" from the subsession 

    } while (0);

    // Set up the next subsession, if any:
    setupNextSubsession(rtspClient);
}

void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString)
{
    //You can put process code here for DESCRIBE.

    //(*(g_client_rt->env))<<"Put code for SETUP command process here please.\n";
    do
    {
        UsageEnvironment& env = rtspClient->envir(); // alias
        StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias

        if (resultCode != 0) 
        {
            env << rtspClient->url() << ": Failed to get a SDP description: " << resultString << "\n";
            break;
        }

        char* sdpDescription = resultString;
        env << rtspClient->url() << ": Got a SDP description:\n" << sdpDescription << "\n";

        // Create a media session object from this SDP description:
        scs.session = MediaSession::createNew(env, sdpDescription);
        delete[] sdpDescription; // because we don't need it anymore
        if (scs.session == NULL) 
        {
            env << rtspClient->url() << ": Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
            break;
        } 
        else if (!scs.session->hasSubsessions()) 
        {
            env << rtspClient->url() << ":This session has no media subsessions (i.e., no \"m=\" lines)\n";
            break;
        }

        // Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
        // calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
        // (Each 'subsession' will have its own data source.)
        scs.iter = new MediaSubsessionIterator(*scs.session);
        setupNextSubsession(rtspClient);
        return;
    } while (0);

    // An unrecoverable error occurred with this stream.
    shutdownStream(rtspClient);
}

void continueAfterOPTIONS(RTSPClient* rtspClient, int resultCode, char* resultString)
{
    //You can put process code here for Option.

    //(*(g_client_rt->env))<<"Put code for OPTIONS command process here please.\n";
    
    //continue send DESCRIBE command
    rtspClient->sendDescribeCommand(continueAfterDESCRIBE);
}


void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);
//void continueAfterPAUSE(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterTEARDOWN(RTSPClient* rtspClient, int resultCode, char* resultString)
{
}

// Other event handler functions:
void subsessionAfterPlaying(void* clientData); // called when a stream's subsession (e.g., audio or video substream) ends
void subsessionByeHandler(void* clientData); // called when a RTCP "BYE" is received for a subsession
void streamTimerHandler(void* clientData);
// called at the end of a stream's expected duration (if the stream has not already signaled its end using a RTCP "BYE")

void create_client_runtime()
{
    g_client_rt = new st_AFD_RTSP_Client_Runtime;
    g_client_rt->init_flag = false;
    g_client_rt->scheduler = NULL;
    g_client_rt->env = NULL;
    g_client_rt->client_count = 0;
    g_client_rt->pollthread = NULL;
}

void delete_client_runtime()
{
    delete g_client_rt; 
    g_client_rt = NULL;
}

extern "C" bool client_init()
{
    if (NULL == g_client_rt) create_client_runtime();
        
    if (true == g_client_rt->init_flag) return g_client_rt->init_flag;

    g_client_rt->scheduler = g_client_rt->scheduler ? 
                             g_client_rt->scheduler : 
                             BasicTaskScheduler::createNew();
    if (NULL == g_client_rt->scheduler) return g_client_rt->init_flag;

    g_client_rt->env = g_client_rt->env ? 
                       g_client_rt->env : 
                       BasicUsageEnvironment::createNew(*g_client_rt->scheduler);
    if (NULL == g_client_rt->env) return g_client_rt->init_flag;
    
    g_client_rt->client_count = 0;

    //startup thread for process eventloop
    g_client_rt->pollthread = new AFDPollThread(g_client_rt->env);
    if ( NULL == g_client_rt->pollthread || g_client_rt->pollthread->Start() < 0)
        return g_client_rt->init_flag;

    g_client_rt->init_flag = true;
   
    return g_client_rt->init_flag;
}

extern "C" bool client_cleanup()
{
    if (NULL == g_client_rt) return true;
    
    //todo:shutdown rtsp session one by one

    //clean up thread, close all client session
    if (g_client_rt->pollthread)
    {
        g_client_rt->pollthread->Stop();
        delete g_client_rt->pollthread;
        g_client_rt->pollthread = NULL;
    }
    
    delete_client_runtime();

    return true;
}

extern "C" const void* create_new(const char* url, int verbosity, const char* appname)
{
    RTSPClient* client = NULL;
    
    if ( NULL != g_client_rt->scheduler &&
         NULL != g_client_rt->env &&
         NULL != g_client_rt->pollthread)
    {
        client = ourRTSPClient::createNew(*g_client_rt->env, url, verbosity, appname);
        if ( NULL == client)
        {
            *g_client_rt->env << "Failed to create a RTSP client for URL \"" 
                              << url << "\": " 
                              << g_client_rt->env->getResultMsg() << "\n";
        }
        else
            g_client_rt->client_count++;
    }
   
    return client;
}

extern "C" unsigned play(const void* handle)
{
    RTSPClient* client = (RTSPClient*)handle;
    return client->sendOptionsCommand(continueAfterOPTIONS);  
}

extern "C" unsigned pause(const void* handle)
{   
    RTSPClient* client = (RTSPClient*)handle;
    StreamClientState& scs = ((ourRTSPClient*)client)->scs; // alias
    
    return client->sendPauseCommand(*scs.session, continueAfterPAUSE);
}

extern "C" unsigned resume(const void* handle, double npt)
{
    RTSPClient* client = (RTSPClient*)handle;
    StreamClientState& scs = ((ourRTSPClient*)client)->scs; // alias
    
    return client->sendPlayCommand(*scs.session, continueAfterPLAY, npt);
}

extern "C" unsigned seek(const void* handle, double npt)
{
    return resume(handle, npt);
}

extern "C" unsigned fast(const void* handle, double scale)
{
    RTSPClient* client = (RTSPClient*)handle;
    StreamClientState& scs = ((ourRTSPClient*)client)->scs; // alias
    
    return client->sendPlayCommand(*scs.session, continueAfterPLAY, 0.0f, -1.0f, scale);
}

extern "C" unsigned slow(const void* handle, double scale)
{
    return fast(handle, scale);
}

extern "C" unsigned stop(const void* handle)
{
    RTSPClient* client = (RTSPClient*)handle;
    StreamClientState& scs = ((ourRTSPClient*)client)->scs; // alias
    
    return client->sendTeardownCommand(*scs.session, continueAfterTEARDOWN);
}
