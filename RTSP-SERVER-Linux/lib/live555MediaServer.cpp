/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2011, Live Networks, Inc.  All rights reserved
// LIVE555 Media Server
// main program
#include <BasicUsageEnvironment.hh>
#include "DynamicRTSPServer.hh"
#include "version.hh"
 static RTSPServer* rtspServer_=NULL;
int runRTSPsever(unsigned short serverPortNum) {
	
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);
//用于事件的调度，实现异步读取事件的句柄的设置以及错误信息的输出
  UserAuthenticationDatabase* authDB = NULL;
#ifdef ACCESS_CONTROL
  // To implement client access control to the RTSP server, do the following:
  authDB = new UserAuthenticationDatabase;
  authDB->addUserRecord("username1", "password1"); // replace these with real strings
  // Repeat the above with each <username>, <password> that you wish to allow
  // access to the server.
#endif

  // Create the RTSP server.  Try first with the default port number (554),
  // and then with the alternative port number (8554):

  portNumBits rtspServerPortNum = serverPortNum;
  RTSPServer* rtspServer;
  rtspServer = DynamicRTSPServer::createNew(*env, rtspServerPortNum, authDB,rtspServerPortNum);//首先创建RTSP服务器(具体实现类是DynamicRTSPServer)
  //在创建过程中，先建立Socket(ourSocket)在TCP的554端口进行监听，然后把连接处理函数句柄(RTSPServer::incomingConnectionHandler)和socket句柄传给任务调度器(taskScheduler)。
  if (rtspServer == NULL) {
    rtspServerPortNum = 8554;
    rtspServer = DynamicRTSPServer::createNew(*env, rtspServerPortNum, authDB,rtspServerPortNum);
  }
   rtspServer_=rtspServer;
  if (rtspServer == NULL) {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    //exit(1);
	 return 0;
  }

  *env << "LIVE555 Media Server\n";
  *env << "\tversion " << MEDIA_SERVER_VERSION_STRING
       << " (LIVE555 Streaming Media library version "
       << LIVEMEDIA_LIBRARY_VERSION_STRING << ").\n";

  char* urlPrefix = rtspServer->rtspURLPrefix();
  *env << "Play streams from this server using the URL\n\t"
       << urlPrefix << "<filename>\nwhere <filename> is a file present in the current directory.\n";
  *env << "Each file's type is inferred from its name suffix:\n";
  *env << "\t\".aac\" => an AAC Audio (ADTS format) file\n";
  *env << "\t\".amr\" => an AMR Audio file\n";
  *env << "\t\".m4e\" => a MPEG-4 Video Elementary Stream file\n";
  *env << "\t\".264\" => a H.264 Video Elementary Stream file\n";
  *env << "\t\".dv\" => a DV Video file\n";
  *env << "\t\".mp3\" => a MPEG-1 or 2 Audio file\n";
  *env << "\t\".mpg\" => a MPEG-1 or 2 Program Stream (audio+video) file\n";
  *env << "\t\".ts\" => a MPEG Transport Stream file\n";
  *env << "\t\t(a \".tsx\" index file - if present - provides server 'trick play' support)\n";
  *env << "\t\".wav\" => a WAV Audio file\n";
  *env << "See http://www.live555.com/mediaServer/ for additional documentation.\n";

  delete urlPrefix;

  env->taskScheduler().doEventLoop(); // does not return

  return 0; // only to prevent compiler warning
}
