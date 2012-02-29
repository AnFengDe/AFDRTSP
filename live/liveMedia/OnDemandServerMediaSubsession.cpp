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
// A 'ServerMediaSubsession' object that creates new, unicast, "TPSink"s
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
    // dummy (unused) source and "TPSink" objects,
    // whose parameters we use for the SDP lines:
    return NULL; // file not found

    struct in_addr dummyAddr;
    dummyAddr.s_addr = 0;
    Groupsock dummyGroupsock(envir(), dummyAddr, 0, 0);
  }

  return fSDPLines;
}

void OnDemandServerMediaSubsession
::getStreamParameters(unsigned clientSessionId,
                      netAddressBits clientAddress,
                      Port const& clientRTPPort,
                      int tcpSocketNum,
                      unsigned char rtpChannelId,
                      unsigned char rtcpChannelId,
                      netAddressBits& destinationAddress,
                      u_int8_t& /*destinationTTL*/,
                      Boolean& isMulticast,
                      Port& serverRTPPort,
                      void*& streamToken) {
  if (destinationAddress == 0) destinationAddress = clientAddress;
  struct in_addr destinationAddr; destinationAddr.s_addr = destinationAddress;
  isMulticast = False;

  if (fLastStreamToken != NULL && fReuseFirstSource) {
    // Special case: Rather than creating a new 'treamState',
    // we reuse the one that we've already created:
    streamToken = fLastStreamToken;
  } else {
    // Normal case: Create a new media source:

    // Create 'groupsock' and 'sink' objects for the destination,
    // using previously unused server port numbers:
    Groupsock* rtpGroupsock;
    portNumBits serverPortNum;
    if (0) {
      // We're streaming raw UDP (not RTP). Create a single groupsock:
      NoReuse dummy(envir()); // ensures that we skip over ports that are already in use
      for (serverPortNum = fInitialPortNum; ; ++serverPortNum) {
	struct in_addr dummyAddr; dummyAddr.s_addr = 0;

	serverRTPPort = serverPortNum;
	rtpGroupsock = new Groupsock(envir(), dummyAddr, serverRTPPort, 255);
	if (rtpGroupsock->socketNum() >= 0) break; // success
      }

    } else {
      // Normal case: We're streaming RTP (over UDP or TCP).  Create a pair of
      // groupsocks (RTP and TCP), with adjacent port numbers (RTP port number even):
      NoReuse dummy(envir()); // ensures that we skip over ports that are already in use
      for (portNumBits serverPortNum = fInitialPortNum; ; serverPortNum += 2) {
	struct in_addr dummyAddr; dummyAddr.s_addr = 0;

	serverRTPPort = serverPortNum;
	rtpGroupsock = new Groupsock(envir(), dummyAddr, serverRTPPort, 255);
	if (rtpGroupsock->socketNum() < 0) {
          delete rtpGroupsock;
          continue; // try again
        }

	if ( 0) {
	  delete rtpGroupsock;
	  continue; // try again
	}

	break; // success
      }
    }

    // Turn off the destinations for each groupsock.  They'll get set later
    // (unless TCP is used instead):
    if (rtpGroupsock != NULL) rtpGroupsock->removeAllDestinations();

    if (rtpGroupsock != NULL) {
      // Try to use a big send buffer for RTP -  at least 0.1 second of
      // specified bandwidth and at least 50 KB
      unsigned rtpBufSize = 25 / 2; // 1 kbps * 0.1 s = 12.5 bytes
      if (rtpBufSize < 50 * 1024) rtpBufSize = 50 * 1024;
      increaseSendBufferTo(envir(), rtpGroupsock->socketNum(), rtpBufSize);
    }

    // Set up the state of the stream.  The stream will get started later:
  }

  // Record these destinations as being for this client session id:
  Destinations* destinations;
  if (tcpSocketNum < 0) { // UDP
    //destinations = new Destinations(destinationAddr, clientRTPPort );
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
}

void OnDemandServerMediaSubsession::pauseStream(unsigned /*clientSessionId*/,
                                        	void* streamToken) {
  // Pausing isn't allowed if multiple clients are receiving data from
  // the same source:
  if (fReuseFirstSource) return;

}

void OnDemandServerMediaSubsession::seekStream(unsigned /*clientSessionId*/,
                                               void* streamToken, double& seekNPT, double streamDuration, u_int64_t& numBytes) {
  numBytes = 0; // by default: unknown

  // Seeking isn't allowed if multiple clients are receiving data from
  // the same source:
  if (fReuseFirstSource) return;

}

void OnDemandServerMediaSubsession::setStreamScale(unsigned /*clientSessionId*/,
                                                   void* streamToken, float scale) {
  // Changing the scale factor isn't allowed if multiple clients are receiving data
  // from the same source:
  if (fReuseFirstSource) return;

}

void OnDemandServerMediaSubsession::deleteStream(unsigned clientSessionId,
                                                 void*& streamToken) {
  // Look up (and remove) the destinations for this client session:
  Destinations* destinations
    = (Destinations*)(fDestinationsHashTable->Lookup((char const*)clientSessionId));
  if (destinations != NULL) {
    fDestinationsHashTable->Remove((char const*)clientSessionId);

    // Stop streaming to these destinations:
  }

  // Delete the "treamState" structure if it's no longer being used:

  // Finally, delete the destinations themselves:
  delete destinations;
}


////////// treamState implementation //////////

static void afterPlayingStreamState(void* clientData) {
  // Otherwise, keep the stream alive, in case a client wants to
  // subsequently re-play the stream starting from somewhere other than the end.
  // (This can be done only on streams that have a known duration.)
}

