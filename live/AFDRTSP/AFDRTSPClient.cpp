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

/// library initliaze flag
bool                g_init_flag     = false;
/// global task scheduler instance
TaskScheduler*      g_scheduler     = NULL;
/// usage environment
UsageEnvironment*   g_env           = NULL;
/// count how many rtsp clients are current use
int                 g_client_count  = 0;
/// work thread for asynchronous rtsp message exchange
AFDPollThread*      g_pollthread    = 0;

// RTSP 'response handlers':
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString)
{
    //You can put process code here for DESCRIBE.

    (*g_env)<<"Put code for DESCRIBE command process here please.\n";
    
}

void continueAfterOPTIONS(RTSPClient* rtspClient, int resultCode, char* resultString)
{
    //You can put process code here for Option.

    (*g_env)<<"Put code for OPTIONS command process here please.\n";
    
    //continue send DESCRIBE command
    rtspClient->sendDescribeCommand(continueAfterDESCRIBE);
}

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString);
void continueAfterPLAY(RTSPClient* rtspClient, int resultCode, char* resultString);

// Other event handler functions:
void subsessionAfterPlaying(void* clientData); // called when a stream's subsession (e.g., audio or video substream) ends
void subsessionByeHandler(void* clientData); // called when a RTCP "BYE" is received for a subsession
void streamTimerHandler(void* clientData);
// called at the end of a stream's expected duration (if the stream has not already signaled its end using a RTCP "BYE")

extern "C" bool init()
{
    if (true == g_init_flag) return g_init_flag;

    g_scheduler = g_scheduler ? g_scheduler : BasicTaskScheduler::createNew();
    if (NULL == g_scheduler) return g_init_flag;

    g_env = g_env ? g_env : BasicUsageEnvironment::createNew(*g_scheduler);
    if (NULL == g_env) return g_init_flag;
    
    g_client_count = 0;

    //startup thread for process eventloop
    g_pollthread = new AFDPollThread(g_env);
    if ( NULL == g_pollthread || g_pollthread->Start() < 0)
       return g_init_flag;

    g_init_flag = true;
   
    return g_init_flag;
}

extern "C" bool cleanup()
{
    if (false == g_init_flag) return true;
    
    //todo:shutdown rtsp session one by one

    //clean up thread, close all client session
    if (g_pollthread)
    {
        g_pollthread->Stop();
        delete g_pollthread;
        g_pollthread = NULL;
    }
    
    g_client_count = 0;

    g_init_flag = false;
    
    return true;
}

extern "C" const void* create_new(const char* url, int verbosity, const char* appname)
{
    RTSPClient* client = NULL;
    
    if ( NULL != g_scheduler &&
         NULL != g_env &&
         NULL != g_pollthread)
    {
        client = RTSPClient::createNew(*g_env, url, verbosity, appname);
        if ( NULL == client)
        {
            *g_env << "Failed to create a RTSP client for URL \"" 
                  << url << "\": " 
                  << g_env->getResultMsg() << "\n";
        }
        else
            g_client_count++;
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
