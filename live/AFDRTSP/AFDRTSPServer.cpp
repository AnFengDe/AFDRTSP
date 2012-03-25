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

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "AFDPollThread.h"

extern st_Handle_Cmd_Callback* g_pstCallback;

/// AFD RTSP server runtime struct
typedef struct _st_AFD_RTSP_Server_Runtime{
    /// library initliaze flag
    bool                init_flag;
    /// global task scheduler instance
    TaskScheduler*      scheduler;
    /// usage environment
    UsageEnvironment*   env;
    /// work thread for asynchronous rtsp message exchange
    AFDPollThread*      pollthread;
    /// rtsp server
    RTSPServer*         server;
}st_AFD_RTSP_Server_Runtime;

st_AFD_RTSP_Server_Runtime* g_server_rt = NULL;

void create_server_runtime()
{
    g_server_rt = new st_AFD_RTSP_Server_Runtime;
    g_server_rt->init_flag = false;
    g_server_rt->scheduler = NULL;
    g_server_rt->env = NULL;
    g_server_rt->server = NULL;
    g_server_rt->pollthread = NULL;
}

void delete_server_runtime()
{
    delete g_server_rt; 
    g_server_rt = NULL;
}
extern "C" bool server_init()
{
    if ( NULL == g_server_rt ) create_server_runtime();
        
    if (true == g_server_rt->init_flag) return g_server_rt->init_flag;

    g_server_rt->scheduler = g_server_rt->scheduler ? 
                             g_server_rt->scheduler : 
                             BasicTaskScheduler::createNew();
    if (NULL == g_server_rt->scheduler) return g_server_rt->init_flag;

    g_server_rt->env = g_server_rt->env ? 
                       g_server_rt->env : 
                       BasicUsageEnvironment::createNew(*g_server_rt->scheduler);
    if (NULL == g_server_rt->env) return g_server_rt->init_flag;
    
    //startup thread for process eventloop
    g_server_rt->pollthread = new AFDPollThread(g_server_rt->env);
    if ( NULL == g_server_rt->pollthread || g_server_rt->pollthread->Start() < 0)
        return g_server_rt->init_flag;

    g_server_rt->init_flag = true;
   
    return g_server_rt->init_flag;
}

extern "C" bool server_cleanup()
{
    if (false == g_server_rt->init_flag) return true;
    
    //clean up thread, close all client session
    if (g_server_rt->pollthread)
    {
        g_server_rt->pollthread->Stop();
        delete g_server_rt->pollthread;
        g_server_rt->pollthread = NULL;
    }
    
    delete g_server_rt->server;
    delete_server_runtime();
        
    return true;
}

extern "C" bool run_rtsp_srv(st_Handle_Cmd_Callback* pstCallback, unsigned short listen_port)
{
    if (NULL == pstCallback ) return false;
        
    g_pstCallback = pstCallback;
  
    portNumBits server_port = listen_port;
    g_server_rt->server = RTSPServer::createNew(*g_server_rt->env, server_port);
    if ( NULL == g_server_rt->server)
    {
        *g_server_rt->env << "Failed to create RTSP server: " << *(g_server_rt->env)->getResultMsg() << "\n";
        return false;
    }

    *g_server_rt->env << "RTSP Media Server\n";

    return true;
}
