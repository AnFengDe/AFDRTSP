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
 *        the cmd_names is all or subset of "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER".
 *        normally, the handler of command not be neccessary, keep it NULL
 */
typedef void (STD_CALLBACK *AFD_RTSP_Handle_Cmd_OPTIONS)(char* cmd_names);

/**
 * \brief RTSP DESCRIBE command handler define
 * \param ret the DESCRIBE command process return value, if less equal zero means failure.
 *            the reason maybe :
 *            1. the media file in url is not found or invalid
 *            2. can't surpport sevice by customize check
 * \param url the media file url, you can process by rule yourself, for example, you can crypt your media url yourself
 * \param sdp_desc sdp description, your must complete yourself
 * \param duration the media file total time length, your must get/set it yourself
 */
typedef void (STD_CALLBACK *AFD_RTSP_Handle_Cmd_DESCRIBE)(int* ret, const char* url, char* sdp_desc, float* duration);

/**
 * \brief RTSP SETUP command handler define
   \param sessionid the server rtsp session id, the unique number for session
   \param url the media file url 
   \param rtp_client_port the rtp client port, this value for rtp server,
          and as defalut, the rtcp port is follow this
   \param rtp_server_port the rtcp server port, this value must be set in callback function
 */
typedef void (STD_CALLBACK *AFD_RTSP_Handle_Cmd_SETUP)(int* ret, const unsigned sessionid, const char* url, const sockaddr_in rtp_client_addr, const unsigned short rtp_client_port, unsigned short *rtp_server_port);

/** 
 * \brief RTSP PLAY command handler define TODO:fix comment for PAUSE / RESUME 
   \param sessionid the server rtsp session id, the unique number for session
   \param scale the play speed ratio 
   \param start meida play start npt
   \param end meida play end npt
 */
typedef void (STD_CALLBACK *AFD_RTSP_Handle_Cmd_PLAY)(const unsigned sessionid, const float scale, const double start, const double end);

/** 
 * \brief RTSP PAUSE command handler define
   \param sessionid the server rtsp session id, the unique number for session
 *
 */
typedef void (STD_CALLBACK *AFD_RTSP_Handle_Cmd_PAUSE)(const unsigned sessionid);

/** 
 * \brief RTSP TEARDOWN command handler define
   \param sessionid the server rtsp session id, the unique number for session
*/
typedef void (STD_CALLBACK *AFD_RTSP_Handle_Cmd_TEARDOWN)(const unsigned sessionid);

///handle cmd callback struct define
typedef struct __st_Handle_Cmd_Callback
{
    ///the options callback function 
    AFD_RTSP_Handle_Cmd_OPTIONS     options;
    ///the describe callback function
    AFD_RTSP_Handle_Cmd_DESCRIBE    describe;
    ///the setup callback function
    AFD_RTSP_Handle_Cmd_SETUP       setup;
    ///the play callback function
    AFD_RTSP_Handle_Cmd_PLAY        play;
    ///the pause callback function
    AFD_RTSP_Handle_Cmd_PAUSE       pause;
    ///the teardown callback function
    AFD_RTSP_Handle_Cmd_TEARDOWN    teardown;
}st_Handle_Cmd_Callback;

#ifdef __cplusplus
}
#endif

#endif //end _AFDRTSPSERVERCALLBACK_H_
