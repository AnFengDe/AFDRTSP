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

typedef int (__stdcall* AFD_RTSP_Callback)(char* cmd_name);

bool run_rtsp_srv(/*AFD_RTSP_Callback* pf, */unsigned short listen_port);

#ifdef __cplusplus
}
#endif

#endif //end _AFDRTSPSERVER_H_
