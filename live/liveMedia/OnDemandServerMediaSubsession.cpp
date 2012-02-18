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
// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand.
// Implementation

#include "OnDemandServerMediaSubsession.hh"
#include <GroupsockHelper.hh>

OnDemandServerMediaSubsession
::OnDemandServerMediaSubsession(UsageEnvironment& env,
				Boolean reuseFirstSource,
				portNumBits initialPortNum)
  : ServerMediaSubsession(env),
    fSDPLines(NULL), fReuseFirstSource(reuseFirstSource), fInitialPortNum(initialPortNum), fLastStreamToken(NULL) {
  fDestinationsHashTable = HashTable::create(ONE_WORD_HASH_KEYS);
  gethostname(fCNAME, sizeof fCNAME);
  fCNAME[sizeof fCNAME-1] = '\0'; // just in case
}

OnDemandServerMediaSubsession::~OnDemandServerMediaSubsession() {
  delete[] fSDPLines;

  // Clean out the destinations hash table:
  while (1) {
    Destinations* destinations
      = (Destinations*)(fDestinationsHashTable->RemoveNext());
    if (destinations == NULL) break;
    delete destinations;
  }
  delete fDestinationsHashTable;
}

char const*
OnDemandServerMediaSubsession::sdpLines() {
  if (fSDPLines == NULL) {
    // We need to construct a set of SDP lines that describe this
    // subsession (as a unicast stream).  To do so, we first create
    // dummy (unused) source and "RTPSink" objects,
    // whose parameters we use for the SDP lines:
    unsigned estBitrate;
    return NULL; // file not found

    struct in_addr dummyAddr;
    dummyAddr.s_addr = 0;
    Groupsock dummyGroupsock(envir(), dummyAddr, 0, 0);
    unsigned char rtpPayloadType = 96 + trackNumber()-1; // if dynamic
  }

  return fSDPLines;
}

void OnDemandServerMediaSubsession
::getStreamParameters(unsigned clientSessionId,
		      netAddressBits clientAddress,
		      Port const& clientRTPPort,
		      Port const& clientRTCPPort,
		      int tcpSocketNum,
		      unsigned char rtpChannelId,
		      unsigned char rtcpChannelId,
		      netAddressBits& destinationAddress,
		      u_int8_t& /*destinationTTL*/,
		      Boolean& isMulticast,
		      Port& serverRTPPort,
		      Port& serverRTCPPort,
		      void*& streamToken) {
  if (destinationAddress == 0) destinationAddress = clientAddress;
  struct in_addr destinationAddr; destinationAddr.s_addr = destinationAddress;
  isMulticast = False;

  if (fLastStreamToken != NULL && fReuseFirstSource) {
    // Special case: Rather than creating a new 'StreamState',
    // we reuse the one that we've already created:
    serverRTPPort = ((StreamState*)fLastStreamToken)->serverRTPPort();
    serverRTCPPort = ((StreamState*)fLastStreamToken)->serverRTCPPort();
    ++((StreamState*)fLastStreamToken)->referenceCount();
    streamToken = fLastStreamToken;
  } else {
    // Normal case: Create a new media source:
    unsigned streamBitrate;

    // Create 'groupsock' and 'sink' objects for the destination,
    // using previously unused server port numbers:
    RTPSink* rtpSink;
    Groupsock* rtpGroupsock;
    Groupsock* rtcpGroupsock;
    portNumBits serverPortNum;
    if (clientRTCPPort.num() == 0) {
      // We're streaming raw UDP (not RTP). Create a single groupsock:
      NoReuse dummy(envir()); // ensures that we skip over ports that are already in use
      for (serverPortNum = fInitialPortNum; ; ++serverPortNum) {
	struct in_addr dummyAddr; dummyAddr.s_addr = 0;

	serverRTPPort = serverPortNum;
	rtpGroupsock = new Groupsock(envir(), dummyAddr, serverRTPPort, 255);
	if (rtpGroupsock->socketNum() >= 0) break; // success
      }

      rtcpGroupsock = NULL;
      rtpSink = NULL;
    } else {
      // Normal case: We're streaming RTP (over UDP or TCP).  Create a pair of
      // groupsocks (RTP and RTCP), with adjacent port numbers (RTP port number even):
      NoReuse dummy(envir()); // ensures that we skip over ports that are already in use
      for (portNumBits serverPortNum = fInitialPortNum; ; serverPortNum += 2) {
	struct in_addr dummyAddr; dummyAddr.s_addr = 0;

	serverRTPPort = serverPortNum;
	rtpGroupsock = new Groupsock(envir(), dummyAddr, serverRTPPort, 255);
	if (rtpGroupsock->socketNum() < 0) {
	  delete rtpGroupsock;
	  continue; // try again
	}

	serverRTCPPort = serverPortNum+1;
	rtcpGroupsock = new Groupsock(envir(), dummyAddr, serverRTCPPort, 255);
	if (rtcpGroupsock->socketNum() < 0) {
	  delete rtpGroupsock;
	  delete rtcpGroupsock;
	  continue; // try again
	}

	break; // success
      }

      unsigned char rtpPayloadType = 96 + trackNumber()-1; // if dynamic
    }

    // Turn off the destinations for each groupsock.  They'll get set later
    // (unless TCP is used instead):
    if (rtpGroupsock != NULL) rtpGroupsock->removeAllDestinations();
    if (rtcpGroupsock != NULL) rtcpGroupsock->removeAllDestinations();

    if (rtpGroupsock != NULL) {
      // Try to use a big send buffer for RTP -  at least 0.1 second of
      // specified bandwidth and at least 50 KB
      unsigned rtpBufSize = streamBitrate * 25 / 2; // 1 kbps * 0.1 s = 12.5 bytes
      if (rtpBufSize < 50 * 1024) rtpBufSize = 50 * 1024;
      increaseSendBufferTo(envir(), rtpGroupsock->socketNum(), rtpBufSize);
    }

    // Set up the state of the stream.  The stream will get started later:
    streamToken = fLastStreamToken
      = new StreamState(*this, serverRTPPort, serverRTCPPort, rtpSink,
			streamBitrate, 
			rtpGroupsock, rtcpGroupsock);
  }

  // Record these destinations as being for this client session id:
  Destinations* destinations;
  if (tcpSocketNum < 0) { // UDP
    destinations = new Destinations(destinationAddr, clientRTPPort, clientRTCPPort);
  } else { // TCP
    destinations = new Destinations(tcpSocketNum, rtpChannelId, rtcpChannelId);
  }
  fDestinationsHashTable->Add((char const*)clientSessionId, destinations);
}

void OnDemandServerMediaSubsession::startStream(unsigned clientSessionId,
						void* streamToken,
						TaskFunc* rtcpRRHandler,
						void* rtcpRRHandlerClientData,
						unsigned short& rtpSeqNum,
						unsigned& rtpTimestamp,
						void* serverRequestAlternativeByteHandlerClientData) {
  StreamState* streamState = (StreamState*)streamToken;
  Destinations* destinations
    = (Destinations*)(fDestinationsHashTable->Lookup((char const*)clientSessionId));
  if (streamState != NULL) {
    streamState->startPlaying(destinations,
			      rtcpRRHandler, rtcpRRHandlerClientData,
			      serverRequestAlternativeByteHandlerClientData);
    if (streamState->rtpSink() != NULL) {
      rtpSeqNum = streamState->rtpSink()->currentSeqNo();
      rtpTimestamp = streamState->rtpSink()->presetNextTimestamp();
    }
  }
}

void OnDemandServerMediaSubsession::pauseStream(unsigned /*clientSessionId*/,
						void* streamToken) {
  // Pausing isn't allowed if multiple clients are receiving data from
  // the same source:
  if (fReuseFirstSource) return;

  StreamState* streamState = (StreamState*)streamToken;
  if (streamState != NULL) streamState->pause();
}

void OnDemandServerMediaSubsession::seekStream(unsigned /*clientSessionId*/,
					       void* streamToken, double& seekNPT, double streamDuration, u_int64_t& numBytes) {
  numBytes = 0; // by default: unknown

  // Seeking isn't allowed if multiple clients are receiving data from
  // the same source:
  if (fReuseFirstSource) return;

  StreamState* streamState = (StreamState*)streamToken;
}

void OnDemandServerMediaSubsession::setStreamScale(unsigned /*clientSessionId*/,
						   void* streamToken, float scale) {
  // Changing the scale factor isn't allowed if multiple clients are receiving data
  // from the same source:
  if (fReuseFirstSource) return;

  StreamState* streamState = (StreamState*)streamToken;
}

void OnDemandServerMediaSubsession::deleteStream(unsigned clientSessionId,
						 void*& streamToken) {
  StreamState* streamState = (StreamState*)streamToken;

  // Look up (and remove) the destinations for this client session:
  Destinations* destinations
    = (Destinations*)(fDestinationsHashTable->Lookup((char const*)clientSessionId));
  if (destinations != NULL) {
    fDestinationsHashTable->Remove((char const*)clientSessionId);

    // Stop streaming to these destinations:
    if (streamState != NULL) streamState->endPlaying(destinations);
  }

  // Delete the "StreamState" structure if it's no longer being used:
  if (streamState != NULL) {
    if (streamState->referenceCount() > 0) --streamState->referenceCount();
    if (streamState->referenceCount() == 0) {
      delete streamState;
      streamToken = NULL;
    }
  }

  // Finally, delete the destinations themselves:
  delete destinations;
}


////////// StreamState implementation //////////

static void afterPlayingStreamState(void* clientData) {
  StreamState* streamState = (StreamState*)clientData;
  if (streamState->streamDuration() == 0.0) {
    // When the input stream ends, tear it down.  This will cause a RTCP "BYE"
    // to be sent to each client, teling it that the stream has ended.
    // (Because the stream didn't have a known duration, there was no other
    //  way for clients to know when the stream ended.)
    streamState->reclaim();
  }
  // Otherwise, keep the stream alive, in case a client wants to
  // subsequently re-play the stream starting from somewhere other than the end.
  // (This can be done only on streams that have a known duration.)
}

StreamState::StreamState(OnDemandServerMediaSubsession& master,
                         Port const& serverRTPPort, Port const& serverRTCPPort,
			 RTPSink* rtpSink, 
			 unsigned totalBW, 
			 Groupsock* rtpGS, Groupsock* rtcpGS)
  : fMaster(master), fAreCurrentlyPlaying(False), fReferenceCount(1),
    fServerRTPPort(serverRTPPort), fServerRTCPPort(serverRTCPPort),
    fRTPSink(rtpSink), fStreamDuration(master.duration()),
    fTotalBW(totalBW), fRTCPInstance(NULL) /* created later */,
    fRTPgs(rtpGS), fRTCPgs(rtcpGS) {
}

StreamState::~StreamState() {
  reclaim();
}

void StreamState
::startPlaying(Destinations* dests,
	       TaskFunc* rtcpRRHandler, void* rtcpRRHandlerClientData,
	       void* serverRequestAlternativeByteHandlerClientData) {
  if (dests == NULL) return;

  if (fRTCPInstance == NULL && fRTPSink != NULL) {
    // Create (and start) a 'RTCP instance' for this RTP sink:
    fRTCPInstance
      = RTCPInstance::createNew(fRTPSink->envir(), fRTCPgs,
				fTotalBW, (unsigned char*)fMaster.fCNAME,
				fRTPSink, NULL /* we're a server */);
        // Note: This starts RTCP running automatically
  }

  if (dests->isTCP) {
    // Change RTP and RTCP to use the TCP socket instead of UDP:
    if (fRTPSink != NULL) {
      fRTPSink->addStreamSocket(dests->tcpSocketNum, dests->rtpChannelId);
    }
    if (fRTCPInstance != NULL) {
      fRTCPInstance->addStreamSocket(dests->tcpSocketNum, dests->rtcpChannelId);
      fRTCPInstance->setSpecificRRHandler(dests->tcpSocketNum, dests->rtcpChannelId,
					  rtcpRRHandler, rtcpRRHandlerClientData);
    }
  } else {
    // Tell the RTP and RTCP 'groupsocks' about this destination
    // (in case they don't already have it):
    if (fRTPgs != NULL) fRTPgs->addDestination(dests->addr, dests->rtpPort);
    if (fRTCPgs != NULL) fRTCPgs->addDestination(dests->addr, dests->rtcpPort);
    if (fRTCPInstance != NULL) {
      fRTCPInstance->setSpecificRRHandler(dests->addr.s_addr, dests->rtcpPort,
					  rtcpRRHandler, rtcpRRHandlerClientData);
    }
  }

  if (!fAreCurrentlyPlaying ) {
    if (fRTPSink != NULL) {
      fAreCurrentlyPlaying = True;
    }
  }
}

void StreamState::pause() {
  if (fRTPSink != NULL) fRTPSink->stopPlaying();
  fAreCurrentlyPlaying = False;
}

void StreamState::endPlaying(Destinations* dests) {
  if (dests->isTCP) {
    if (fRTPSink != NULL) {
      fRTPSink->removeStreamSocket(dests->tcpSocketNum, dests->rtpChannelId);
    }
    if (fRTCPInstance != NULL) {
      fRTCPInstance->removeStreamSocket(dests->tcpSocketNum, dests->rtcpChannelId);
      fRTCPInstance->unsetSpecificRRHandler(dests->tcpSocketNum, dests->rtcpChannelId);
    }
  } else {
    // Tell the RTP and RTCP 'groupsocks' to stop using these destinations:
    if (fRTPgs != NULL) fRTPgs->removeDestination(dests->addr, dests->rtpPort);
    if (fRTCPgs != NULL) fRTCPgs->removeDestination(dests->addr, dests->rtcpPort);
    if (fRTCPInstance != NULL) {
      fRTCPInstance->unsetSpecificRRHandler(dests->addr.s_addr, dests->rtcpPort);
    }
  }
}

void StreamState::reclaim() {
  // Delete allocated media objects
  Medium::close(fRTCPInstance) /* will send a RTCP BYE */; fRTCPInstance = NULL;
  Medium::close(fRTPSink); fRTPSink = NULL;

  if (fMaster.fLastStreamToken == this) fMaster.fLastStreamToken = NULL;

  delete fRTPgs; fRTPgs = NULL;
  delete fRTCPgs; fRTCPgs = NULL;
}
