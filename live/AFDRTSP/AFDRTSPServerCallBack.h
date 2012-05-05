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

#ifndef _AFDRTSPSERVERCALLBACK_H_
#define _AFDRTSPSERVERCALLBACK_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#define STD_CALLBACK    __stdcall
#else
#define STD_CALLBACK    
#endif

/** 
 * \brief RTSP OPTIONS command handler define
 * \param cmd_names the rtsp cmds that server supported
 */
typedef void (STD_CALLBACK *AFD_RTSP_Handle_Cmd_OPTIONS)(char* cmd_names);
/// RTSP DESCRIBE command handler define
typedef void (STD_CALLBACK *AFD_RTSP_Handle_Cmd_DESCRIBE)(int* ret, const char* url, char* sdp_desc, float* duration);
/// RTSP SETUP command handler define
typedef void (STD_CALLBACK *AFD_RTSP_Handle_Cmd_SETUP)(const unsigned sessionid, const char* url, const unsigned short rtp_client_port, unsigned short *rtp_server_port);
/// RTSP PLAY command handler define
typedef void (STD_CALLBACK *AFD_RTSP_Handle_Cmd_PLAY)(const unsigned sessionid, const float scale, const double start, const double end);
/// RTSP PAUSE command handler define
typedef void (STD_CALLBACK *AFD_RTSP_Handle_Cmd_PAUSE)(const unsigned sessionid);
/// RTSP TEARDOWN command handler define
typedef void (STD_CALLBACK *AFD_RTSP_Handle_Cmd_TEARDOWN)(const unsigned sessionid);

///handle cmd callback struct define
typedef struct __st_Handle_Cmd_Callback
{
    ///the options callback function 
    AFD_RTSP_Handle_Cmd_OPTIONS     options;
    AFD_RTSP_Handle_Cmd_DESCRIBE    describe;
    AFD_RTSP_Handle_Cmd_SETUP       setup;
    AFD_RTSP_Handle_Cmd_PLAY        play;
    AFD_RTSP_Handle_Cmd_PAUSE       pause;
    AFD_RTSP_Handle_Cmd_TEARDOWN    teardown;
}st_Handle_Cmd_Callback;

#ifdef __cplusplus
}
#endif

#endif //end _AFDRTSPSERVERCALLBACK_H_
