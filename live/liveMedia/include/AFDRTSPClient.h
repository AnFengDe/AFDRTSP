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

const void* CreateRTSPClientSession();

int Play(const void* pSession, const char* sURL);

int Pause(const void* pSession);

int Resume(const void* pSession);

int Play(const void* pSession, double fPercent);

int Fast(const void* pSession, double fScale);

int Slow(const void* pSession, double fScale);

int Stop(const void* pSession);

#endif
