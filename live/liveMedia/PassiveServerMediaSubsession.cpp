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
// A 'ServerMediaSubsession' object that represents an existing
// 'TPSink', rather than one that creates new 'TPSink's on demand.
// Implementation

#include "PassiveServerMediaSubsession.hh"
#include <GroupsockHelper.hh>

////////// PassiveServerMediaSubsession //////////

PassiveServerMediaSubsession*
PassiveServerMediaSubsession::createNew(
                                        ) {
  UsageEnvironment* env = NULL; //only for modify to across compile by chtian
  return new PassiveServerMediaSubsession(*env);
}

PassiveServerMediaSubsession
::PassiveServerMediaSubsession(UsageEnvironment& env)
  : ServerMediaSubsession(env),
    fSDPLines(NULL) {
  fClientRTCPSourceRecords = HashTable::create(ONE_WORD_HASH_KEYS);
}

class RTCPSourceRecord {
public:
  RTCPSourceRecord(netAddressBits addr, Port const& port)
    : addr(addr), port(port) {
  }

  netAddressBits addr;
  Port port;
};

PassiveServerMediaSubsession::~PassiveServerMediaSubsession() {
  delete[] fSDPLines;

  // Clean out the RTCPSourceRecord table:
  while (1) {
    RTCPSourceRecord* source = (RTCPSourceRecord*)(fClientRTCPSourceRecords->RemoveNext());
    if (source == NULL) break;
    delete source;
  }

  delete fClientRTCPSourceRecords;
}

char const*
PassiveServerMediaSubsession::sdpLines() {
  if (fSDPLines == NULL ) {
    // Construct a set of SDP lines that describe this subsession:
    // Use the components from "rtpSink":
#if 0
    Groupsock const& gs = fTPSink.groupsockBeingUsed();
    AddressString groupAddressStr(gs.groupAddress());
    unsigned short portNum = ntohs(gs.port().num());
    unsigned char ttl = gs.ttl();
    unsigned char rtpPayloadType = fTPSink.rtpPayloadType();
    char const* mediaType = fTPSink.sdpMediaType();
    unsigned estBitrate
      = fRTCPInstance == NULL ? 50 : fRTCPInstance->totSessionBW();
    char* rtpmapLine = fTPSink.rtpmapLine();
    char const* rangeLine = rangeSDPLine();
    char const* auxSDPLine = fTPSink.auxSDPLine();
    if (auxSDPLine == NULL) auxSDPLine = "";

    char const* const sdpFmt =
      "m=%s %d RTP/AVP %d\r\n"
      "c=IN IP4 %s/%d\r\n"
      "b=AS:%u\r\n"
      "%s"
      "%s"
      "%s"
      "a=control:%s\r\n";
    unsigned sdpFmtSize = strlen(sdpFmt)
      + strlen(mediaType) + 5 /* max short len */ + 3 /* max char len */
      + strlen(groupAddressStr.val()) + 3 /* max char len */
      + 20 /* max int len */
      + strlen(rtpmapLine)
      + strlen(rangeLine)
      + strlen(auxSDPLine)
      + strlen(trackId());
    char* sdpLines = new char[sdpFmtSize];
    sprintf(sdpLines, sdpFmt,
            mediaType, // m= <media>
            portNum, // m= <port>
            rtpPayloadType, // m= <fmt list>
            groupAddressStr.val(), // c= <connection address>
            ttl, // c= TTL
            estBitrate, // b=AS:<bandwidth>
            rtpmapLine, // a=rtpmap:... (if present)
            rangeLine, // a=range:... (if present)
            auxSDPLine, // optional extra SDP line
            trackId()); // a=control:<track-id>
    delete[] (char*)rangeLine; delete[] rtpmapLine;

    fSDPLines = strDup(sdpLines);
    delete[] sdpLines;
#endif
  }

  return fSDPLines;
}

void PassiveServerMediaSubsession
::getStreamParameters(unsigned clientSessionId,
                      netAddressBits clientAddress,
                      Port const& /*clientRTPPort*/,
                      int /*tcpSocketNum*/,
                      unsigned char /*rtpChannelId*/,
                      unsigned char /*rtcpChannelId*/,
                      netAddressBits& destinationAddress,
                      u_int8_t& destinationTTL,
                      Boolean& isMulticast,
                      Port& serverRTPPort,
                      void*& streamToken) {
  isMulticast = True;
#if 0
  Groupsock& gs = fTPSink.groupsockBeingUsed();
  if (destinationTTL == 255) destinationTTL = gs.ttl();
  if (destinationAddress == 0) { // normal case
    destinationAddress = gs.groupAddress().s_addr;
  } else { // use the client-specified destination address instead:
    struct in_addr destinationAddr; destinationAddr.s_addr = destinationAddress;
    gs.changeDestinationParameters(destinationAddr, 0, destinationTTL);
    if (fRTCPInstance != NULL) {
      Groupsock* rtcpGS = fRTCPInstance->RTCPgs();
      rtcpGS->changeDestinationParameters(destinationAddr, 0, destinationTTL);
    }
  }
  serverRTPPort = gs.port();
  if (fRTCPInstance != NULL) {
    Groupsock* rtcpGS = fRTCPInstance->RTCPgs();
    serverRTCPPort = rtcpGS->port();
  }
  streamToken = NULL; // not used

  // Make a record of this client's source - for RTCP RR handling:
  RTCPSourceRecord* source = new RTCPSourceRecord(clientAddress, clientRTCPPort);
  fClientRTCPSourceRecords->Add((char const*)clientSessionId, source);
#endif
}

void PassiveServerMediaSubsession::startStream(unsigned clientSessionId,
                                               void* /*streamToken*/,
                                               TaskFunc* rtcpRRHandler,
                                               void* rtcpRRHandlerClientData,
                                               unsigned short& rtpSeqNum,
                                               unsigned& rtpTimestamp,
                                               void* /*serverRequestAlternativeByteHandlerClientData*/) {

  // Try to use a big send buffer for RTP -  at least 0.1 second of
  // specified bandwidth and at least 50 KB
  unsigned streamBitrate = 50; // in kbps
  unsigned rtpBufSize = streamBitrate * 25 / 2; // 1 kbps * 0.1 s = 12.5 bytes
  if (rtpBufSize < 50 * 1024) rtpBufSize = 50 * 1024;
#if 0
  increaseSendBufferTo(envir(), fTPSink.groupsockBeingUsed().socketNum(), rtpBufSize);
#endif

  // Set up the handler for incoming RTCP "RR" packets from this client:
  if (NULL) {
    RTCPSourceRecord* source = (RTCPSourceRecord*)(fClientRTCPSourceRecords->Lookup((char const*)clientSessionId));
    if (source != NULL) {
    }
  }
}

void PassiveServerMediaSubsession::deleteStream(unsigned clientSessionId, void*& /*streamToken*/) {
  // Lookup and remove the 'RTCPSourceRecord' for this client.  Also turn off RTCP "RR" handling:
  RTCPSourceRecord* source = (RTCPSourceRecord*)(fClientRTCPSourceRecords->Lookup((char const*)clientSessionId));
  if (source != NULL) {
    }

    fClientRTCPSourceRecords->Remove((char const*)clientSessionId);
    delete source;
}
