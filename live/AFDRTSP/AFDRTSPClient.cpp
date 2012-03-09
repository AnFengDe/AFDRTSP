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

extern "C" bool init()
{
    if (true == g_init_flag) return g_init_flag;

    g_scheduler = BasicTaskScheduler::createNew();
    if (NULL == g_scheduler) return g_init_flag;

    g_env = BasicUsageEnvironment::createNew(*g_scheduler);
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
    
    //todo:clean up thread, global variable

    g_init_flag = false;
    return true;
}

extern "C" const void* create_new(const char* url, int verbosity, const char* appname)
{
    RTSPClient* client = NULL;
    // Create enviroment when first call
    if ( NULL == g_scheduler )
    {
        g_scheduler = BasicTaskScheduler::createNew();
        g_env = BasicUsageEnvironment::createNew(*g_scheduler);

        //openURL(env, program, rtspurl)
        client = RTSPClient::createNew(*g_env, url, verbosity, appname);
        // todo: env scheduler must in standalone thread
        char s = 0x00;
        g_env->taskScheduler().doEventLoop(&s);
    }
    else
    {
        client = RTSPClient::createNew(*g_env, url, verbosity, appname);
    }
   
    return (client);
}

int play(const int handle, const char* url)
{
    return NULL;
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
