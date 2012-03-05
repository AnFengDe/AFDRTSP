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

TaskScheduler* g_scheduler = NULL;
UsageEnvironment* g_env = NULL;

const int create_new(const char* url, int verbosity = 0, const char* appname = NULL)
{
    // Create enviroment when first call
    if ( NULL == g_scheduler )
    {
        g_scheduler = BasicTaskScheduler::createNew();
        g_env = BasicUsageEnvironment::createNew(*g_scheduler);
    }

    //openURL(env, program, rtspurl)
    RTSPClient* client = RTSPClient::createNew(*g_env, url, verbosity, appname);
    // todo: env scheduler must in standalone thread
    //env->taskScheduler().doEventLoop();

    return NULL;
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

int play(const int handle, double percent)
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

