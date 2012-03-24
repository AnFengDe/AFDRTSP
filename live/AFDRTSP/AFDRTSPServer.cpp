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
    g_pstCallback = pstCallback;
#if 0
  // Create the RTSP server.  Try first with the default port number (554),
  // and then with the alternative port number (8554):
  portNumBits rtspServerPortNum = 554;
  rtspServer = DynamicRTSPServer::createNew(*env, rtspServerPortNum, authDB);
  if (rtspServer == NULL) {
    rtspServerPortNum = 8554;
    rtspServer = DynamicRTSPServer::createNew(*env, rtspServerPortNum, authDB);
  }
  if (rtspServer == NULL) {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    exit(1);
  }

  *env << "RTSP Media Server\n";
 /* *env << "\tversion " << MEDIA_SERVER_VERSION_STRING
       << " (LIVE555 Streaming Media library version "
       << LIVEMEDIA_LIBRARY_VERSION_STRING << ").\n";

  char* urlPrefix = rtspServer->rtspURLPrefix();
  *env << "Play streams from this server using the URL\n\t"
       << urlPrefix << "<filename>\nwhere <filename> is a file present in the current directory.\n";
  *env << "Each file's type is inferred from its name suffix:\n";
  *env << "\t\".264\" => a H.264 Video Elementary Stream file\n";
  *env << "\t\".aac\" => an AAC Audio (ADTS format) file\n";
  *env << "\t\".ac3\" => an AC-3 Audio file\n";
  *env << "\t\".amr\" => an AMR Audio file\n";
  *env << "\t\".dv\" => a DV Video file\n";
  *env << "\t\".m4e\" => a MPEG-4 Video Elementary Stream file\n";
  *env << "\t\".mkv\" => a Matroska audio+video+(optional)subtitles file\n";
  *env << "\t\".mp3\" => a MPEG-1 or 2 Audio file\n";
  *env << "\t\".mpg\" => a MPEG-1 or 2 Program Stream (audio+video) file\n";
  *env << "\t\".ts\" => a MPEG Transport Stream file\n";
  *env << "\t\t(a \".tsx\" index file - if present - provides server 'trick play' support)\n";
  *env << "\t\".wav\" => a WAV Audio file\n";
  *env << "\t\".webm\" => a WebM audio(Vorbis)+video(VP8) file\n";
  *env << "See http://www.live555.com/mediaServer/ for additional documentation.\n";*/

  // Also, attempt to create a HTTP server for RTSP-over-HTTP tunneling.
  // Try first with the default HTTP port (80), and then with the alternative HTTP
  // port numbers (8000 and 8080).

  if (rtspServer->setUpTunnelingOverHTTP(80) || rtspServer->setUpTunnelingOverHTTP(8000) || rtspServer->setUpTunnelingOverHTTP(8080)) {
    *env << "(We use port " << rtspServer->httpServerPortNum() << " for optional RTSP-over-HTTP tunneling, or for HTTP live streaming (for indexed Transport Stream files only).)\n";
  } else {
    *env << "(RTSP-over-HTTP tunneling is not available.)\n";
  }

  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
#endif
    return false;
}
