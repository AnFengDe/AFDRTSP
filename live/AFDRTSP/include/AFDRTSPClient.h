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

/**
 * \brief   Create RTSP client session
 *
 * \return  RTSP client session handle, the handle will be auto release 
 *          when call Stop function. If return NULL, means create fail.
 */
const int create_new();

/**
 * \brief   Open stream by url, this function will call method below:
 *          OPTIONS, DESCRIBE, SETUP, PLAY
 *          the methode call is asynchronous, if you want to process response 
 *          yourself, you must set callback function for it.
 *
 * \param   handle
 * \param   url like rtsp://60.168.1.20/record.mp4
 *
 * \return  less than zero means failure, otherwise is success.
 */
int play(const int handle, const char* url);

int pause(const int handle);

int resume(const int handle);

int play(const int handle, double percent);

int fast(const int handle, double scale);

int slow(const int handle, double scale);

int stop(const int handle);

#endif
