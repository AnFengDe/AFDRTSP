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

#ifndef _AFDRTSPCLIENT_H_
#define _AFDRTSPCLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif
/**
 * \brief   init rtsp client runtime envrionment
 *
 * \return  return true while success, otherwise is false
 */
bool client_init();

/**
 * \brief   cleanup rtsp client runtime envrionment,close all session automatically
 *
 * \return  return true while success, otherwise is false
 */
bool client_cleanup();

/**
 * \brief   Create RTSP client session
 * \param   url rtsp url , like rtsp://192.168.1.20/record.mp4
 * \param   verbosity 0 or 1, the swither of log output, defalut is 0
 * \param   appname  the application name, default is NULL
 *
 * \return  RTSP client session handle, the handle will be auto release 
 *          when call Stop function. If return NULL, means create fail.
 */
const void* create_new(const char* url, int verbosity = 0, const char* appname = NULL);

/**
 * \brief   Open stream by handle create_new returned, this function will call method below:
 *          OPTIONS, DESCRIBE, SETUP, PLAY
 *          the methode call is asynchronous, if you want to process response 
 *          yourself, you must set callback function for it.
 *
 * \param   handle create_new returned
 *
 * \return  the return value is equal zero means failure, otherwise is success.
 */
unsigned play(const void* handle);

int pause(const int handle);

int resume(const int handle);

int skip(const int handle, double percent);

int fast(const int handle, double scale);

int slow(const int handle, double scale);

int stop(const int handle);

#ifdef __cplusplus
}
#endif
#endif
