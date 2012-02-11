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
// "liveMedia"
// Copyright (c) 1996-2012 Live Networks, Inc.  All rights reserved.
// Inclusion of header files representing the interface
// for the entire library
//
// Programs that use the library can include this header file,
// instead of each of the individual media header files

#ifndef _LIVEMEDIA_HH
#define _LIVEMEDIA_HH

#include "MPEG1or2VideoRTPSink.hh"
#include "MPEG4ESVideoRTPSink.hh"
#include "BasicUDPSink.hh"
#include "JPEGVideoRTPSink.hh"
#include "SimpleRTPSink.hh"
#include "MPEG2IndexFromTransportStream.hh"
#include "MPEG2TransportStreamTrickModeFilter.hh"
#include "ByteStreamMultiFileSource.hh"
#include "ByteStreamMemoryBufferSource.hh"
#include "BasicUDPSource.hh"
#include "SimpleRTPSource.hh"
#include "MPEG4ESVideoRTPSource.hh"
#include "MPEG4GenericRTPSource.hh"
#include "MP3ADURTPSource.hh"
#include "JPEGVideoRTPSource.hh"
#include "JPEGVideoSource.hh"
#include "MPEG1or2VideoRTPSource.hh"
#include "VP8VideoRTPSource.hh"
#include "MPEG2TransportStreamFromPESSource.hh"
#include "MPEG2TransportStreamFromESSource.hh"
#include "MPEG2TransportStreamFramer.hh"
#include "MP3HTTPSource.hh"
#include "MP3ADU.hh"
#include "MP3ADUinterleaving.hh"
#include "MP3Transcoder.hh"
#include "MPEG1or2DemuxedElementaryStream.hh"
#include "VP8VideoRTPSink.hh"
#include "MPEG4GenericRTPSink.hh"
#include "MPEG1or2VideoStreamDiscreteFramer.hh"
#include "MPEG4VideoStreamDiscreteFramer.hh"
#include "DeviceSource.hh"
#include "StreamReplicator.hh"
#include "RTSPServerSupportingHTTPStreaming.hh"
#include "RTSPClient.hh"
#include "QuickTimeGenericRTPSource.hh"
#include "PassiveServerMediaSubsession.hh"
#include "MPEG4VideoFileServerMediaSubsession.hh"
#include "T140TextRTPSink.hh"
#include "TCPStreamSink.hh"
#include "MPEG1or2VideoFileServerMediaSubsession.hh"
#include "MPEG1or2FileServerDemux.hh"
#include "MPEG2TransportFileServerMediaSubsession.hh"
#include "MPEG2TransportUDPServerMediaSubsession.hh"
#include "MatroskaFileServerDemux.hh"
#include "DarwinInjector.hh"

#endif
