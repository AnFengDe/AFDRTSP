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

#ifndef _AFDRTSPSERVER_H_
#define _AFDRTSPSERVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#define STD_CALLBACK    __stdcall
#else
#define STD_CALLBACK    
#endif

/*! \brief the cmd_names is all or subset of 
 "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER".
 normally, the handler of command not be neccessary, keep it NULL 
*/
typedef void (STD_CALLBACK *AFD_RTSP_Handle_Cmd_OPTIONS)(char* cmd_names);
typedef void (STD_CALLBACK *AFD_RTSP_Handle_Cmd_DESCRIBE)(int* ret, char* url, char* sdp_desc);

///handle cmd callback struct define
typedef struct __st_Handle_Cmd_Callback
{
    ///the options callback function 
    AFD_RTSP_Handle_Cmd_OPTIONS options;
    AFD_RTSP_Handle_Cmd_DESCRIBE describe;
}st_Handle_Cmd_Callback;

/**
 * \brief   init rtsp server runtime envrionment
 *
 * \return  return true while success, otherwise is false
 */
bool server_init();

/**
 * \brief   cleanup rtsp server runtime envrionment,shutdown service automatically
 *
 * \return  return true while success, otherwise is false
 */
bool server_cleanup();

bool run_rtsp_srv(st_Handle_Cmd_Callback* pstCallback, unsigned short listen_port);

bool add_server_media_session();
bool remove_server_media_session();

#ifdef __cplusplus
}
#endif

#endif //end _AFDRTSPSERVER_H_
