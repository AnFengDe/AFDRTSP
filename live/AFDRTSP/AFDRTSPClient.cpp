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

/// AFD RTSP client runtime struct
	Boolean StreamUsingTCP;
    MediaSession* session ;
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

// RTSP 'response handlers':
void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString)
{
    //You can put process code here for Option.

    (*(g_client_rt->env))<<"Put code for PLAY command process here please.\n";
	//rtspClient->sendPlayCommand(*session, continueAfterPLAY, start, end, Scale, OurAuthenticator);
 }
void continueAfterPAUSE(RTSPClient* rtspClient, int resultCode, char* resultString)
{
    //You can put process code here for Option.

    //(*(g_client_rt->env))<<"Put code for PAUSE command process here please.\n";

 }
void continueAfterTAERDOWN(RTSPClient* rtspClient, int resultCode, char* resultString)
{
    //You can put process code here for Option.

    //(*(g_client_rt->env))<<"Put code for PAUSE command process here please.\n";

 }
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString)
{
    //You can put process code here for DESCRIBE.

    (*(g_client_rt->env))<<"Put code for SETUP command process here please.\n";
 // 	rtspClient->sendSetupCommand(*subsession, continueAfterSETUP, False, False, forceMulticastOnUnspecified, NULL);

  
}

void continueAfterOPTIONS(RTSPClient* rtspClient, int resultCode, char* resultString)
{
    //You can put process code here for Option.

    (*(g_client_rt->env))<<"Put code for OPTIONS command process here please.\n";
    
    //continue send DESCRIBE command
    rtspClient->sendDescribeCommand(continueAfterDESCRIBE);
}


void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterPAUSE(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterTEARDOWN(RTSPClient* rtspClient, int resultCode, char* resultString);

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
        client = RTSPClient::createNew(*g_client_rt->env, url, verbosity, appname);
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

int pause(const int handle)
{   
    // (*(g_client_rt->env))<<"Put code for PAUSE command process here please.\n";
  	//rtspClient->sendPauseCommand(*session, continueAfterPAUSE, OurAuthenticator);
    return NULL;
}

int resume(const int handle)
{
    return NULL;
}

int skip(const int handle, double percent)
{
    return NULL;
}

int fast(const int handle, double scale)
{
    return NULL;
}

int slow(const int handle, double scale)
{
    return NULL;
}

int stop(const int handle)
{
    return NULL;
}
