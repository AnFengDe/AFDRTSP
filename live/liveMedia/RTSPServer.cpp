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
// A RTSP server
// Implementation
#include "RTSPServer.hh"
#include "RTSPCommon.hh"
#include "Base64.hh"
#include "GroupsockHelper.hh"
#include "../../AFDRTSP/AFDRTSPServerCallBack.h"

#if defined(__WIN32__) || defined(_WIN32) || defined(_QNX4)
#else
#include <signal.h>
#define USE_SIGNALS 1
#endif

/// callback struct for process rtsp command customize 
st_Handle_Cmd_Callback* g_pstCallback = NULL;

////////// RTSPServer implementation //////////

RTSPServer*
RTSPServer::createNew(UsageEnvironment& env, Port ourPort,
                      UserAuthenticationDatabase* authDatabase,
                      unsigned reclamationTestSeconds) 
{
    int ourSocket = setUpOurSocket(env, ourPort);
    if (ourSocket == -1) return NULL;

    return new RTSPServer(env, ourSocket, ourPort, authDatabase, reclamationTestSeconds);
}

Boolean RTSPServer::lookupByName(UsageEnvironment& env,
                                 char const* name,
                                 RTSPServer*& resultServer) 
{
    resultServer = NULL; // unless we succeed

    Medium* medium;
    if (!Medium::lookupByName(env, name, medium)) return False;

    if (!medium->isRTSPServer()) 
    {
        env.setResultMsg(name, " is not a RTSP server");
        return False;
    }

    resultServer = (RTSPServer*)medium;
    return True;
}

void RTSPServer::addServerMediaSession(ServerMediaSession* serverMediaSession) 
{
    if (serverMediaSession == NULL) return;

    char const* sessionName = serverMediaSession->streamName();
    if (sessionName == NULL) sessionName = "";
    ServerMediaSession* existingSession
            = (ServerMediaSession*)(fServerMediaSessions->Add(sessionName, (void*)serverMediaSession));
    removeServerMediaSession(existingSession); // if any
}

ServerMediaSession* RTSPServer::lookupServerMediaSession(char const* streamName) 
{
    return (ServerMediaSession*)(fServerMediaSessions->Lookup(streamName));
}

void RTSPServer::removeServerMediaSession(ServerMediaSession* serverMediaSession) 
{
    if (serverMediaSession == NULL) return;

    fServerMediaSessions->Remove(serverMediaSession->streamName());
    if (serverMediaSession->referenceCount() == 0) 
    {
        Medium::close(serverMediaSession);
    } 
}

void RTSPServer::removeServerMediaSession(char const* streamName) 
{
    removeServerMediaSession(lookupServerMediaSession(streamName));
}

char* RTSPServer::rtspURL(ServerMediaSession const* serverMediaSession, 
                          int clientSocket) const 
{
    char* urlPrefix = rtspURLPrefix(clientSocket);
    char const* sessionName = serverMediaSession->streamName();

    char* resultURL = new char[strlen(urlPrefix) + strlen(sessionName) + 1];
    sprintf(resultURL, "%s%s", urlPrefix, sessionName);

    delete[] urlPrefix;
    return resultURL;
}

char* RTSPServer::rtspURLPrefix(int clientSocket) const 
{
    struct sockaddr_in ourAddress;
    if (clientSocket < 0) 
    {
        // Use our default IP address in the URL:
        ourAddress.sin_addr.s_addr = ReceivingInterfaceAddr != 0
                                        ? ReceivingInterfaceAddr
                                        : ourIPAddress(envir()); // hack
    } 
    else 
    {
        SOCKLEN_T namelen = sizeof ourAddress;
        getsockname(clientSocket, (struct sockaddr*)&ourAddress, &namelen);
    }

    char urlBuffer[100]; // more than big enough for "rtsp://<ip-address>:<port>/"

    portNumBits portNumHostOrder = ntohs(fRTSPServerPort.num());
    if (portNumHostOrder == 554 /* the default port number */) 
    {
        sprintf(urlBuffer, "rtsp://%s/", AddressString(ourAddress).val());
    } 
    else 
    {
        sprintf(urlBuffer, "rtsp://%s:%hu/",
                AddressString(ourAddress).val(), portNumHostOrder);
    }

    return strDup(urlBuffer);
}

UserAuthenticationDatabase* RTSPServer::setAuthenticationDatabase(UserAuthenticationDatabase* newDB) 
{
    UserAuthenticationDatabase* oldDB = fAuthDB;
    fAuthDB = newDB;

    return oldDB;
}

#define LISTEN_BACKLOG_SIZE 20

int RTSPServer::setUpOurSocket(UsageEnvironment& env, Port& ourPort) 
{
    int ourSocket = -1;

    do 
    {
        // The following statement is enabled by default.
        // Don't disable it (by defining ALLOW_RTSP_SERVER_PORT_REUSE) unless you know what you're doing.
#ifndef ALLOW_RTSP_SERVER_PORT_REUSE
        NoReuse dummy(env); // Don't use this socket if there's already a local server using it
#endif

        ourSocket = setupStreamSocket(env, ourPort);
        if (ourSocket < 0) break;

        // Make sure we have a big send buffer:
        if (!increaseSendBufferTo(env, ourSocket, 50*1024)) break;

        // Allow multiple simultaneous connections:
        if (listen(ourSocket, LISTEN_BACKLOG_SIZE) < 0) 
        {
            env.setResultErrMsg("listen() failed: ");
            break;
        }

        if (ourPort.num() == 0) 
        {
            // bind() will have chosen a port for us; return it also:
            if (!getSourcePort(env, ourSocket, ourPort)) break;
        }

        return ourSocket;
    } while (0);

    if (ourSocket != -1) ::closeSocket(ourSocket);
    return -1;
}

RTSPServer::RTSPServer(UsageEnvironment& env,
                       int ourSocket, Port ourPort,
                       UserAuthenticationDatabase* authDatabase,
                       unsigned reclamationTestSeconds)
  : Medium(env),
    fRTSPServerSocket(ourSocket), fRTSPServerPort(ourPort),
    fAuthDB(authDatabase), fReclamationTestSeconds(reclamationTestSeconds),
    fServerMediaSessions(HashTable::create(STRING_HASH_KEYS)) {
#ifdef USE_SIGNALS
  // Ignore the SIGPIPE signal, so that clients on the same host that are killed
  // don't also kill us:
  signal(SIGPIPE, SIG_IGN);
#endif

  // Arrange to handle connections from others:
  env.taskScheduler().turnOnBackgroundReadHandling(fRTSPServerSocket,
                                                   (TaskScheduler::BackgroundHandlerProc*)&incomingConnectionHandlerRTSP, this);
}

RTSPServer::~RTSPServer() 
{
    // Turn off background read handling:
    envir().taskScheduler().turnOffBackgroundReadHandling(fRTSPServerSocket);
    ::closeSocket(fRTSPServerSocket);

    // Remove all server media sessions (they'll get deleted when they're finished):
    while (1) 
    {
        ServerMediaSession* serverMediaSession
            = (ServerMediaSession*)fServerMediaSessions->RemoveNext();
        if (serverMediaSession == NULL) break;
        removeServerMediaSession(serverMediaSession);
    }

    // Finally, delete the session table itself:
    delete fServerMediaSessions;
}

Boolean RTSPServer::isRTSPServer() const 
{
    return True;
}

void RTSPServer::incomingConnectionHandlerRTSP(void* instance, int /*mask*/) {
  RTSPServer* server = (RTSPServer*)instance;
  server->incomingConnectionHandlerRTSP1();
}
void RTSPServer::incomingConnectionHandlerRTSP1() {
  incomingConnectionHandler(fRTSPServerSocket);
}

void RTSPServer::incomingConnectionHandler(int serverSocket) {
  struct sockaddr_in clientAddr;
  SOCKLEN_T clientAddrLen = sizeof clientAddr;
  int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
  if (clientSocket < 0) {
    int err = envir().getErrno();
    if (err != EWOULDBLOCK) {
        envir().setResultErrMsg("accept() failed: ");
    }
    return;
  }
  makeSocketNonBlocking(clientSocket);
  increaseSendBufferTo(envir(), clientSocket, 50*1024);

#ifdef DEBUG
  envir() << "accept()ed connection from " << AddressString(clientAddr).val() << "\n";
#endif

  // Create a new object for this RTSP session.
  // (Choose a random 32-bit integer for the session id (it will be encoded as a 8-digit hex number).  We don't bother checking for
  //  a collision; the probability of two concurrent sessions getting the same session id is very low.)
  // (We do, however, avoid choosing session id 0, because that has a special use (by "OnDemandServerMediaSubsession").)
  unsigned sessionId;
  do { sessionId = (unsigned)our_random32(); } while (sessionId == 0);
  (void)createNewClientSession(sessionId, clientSocket, clientAddr);
}


////////// RTSPServer::RTSPClientSession implementation //////////

RTSPServer::RTSPClientSession
::RTSPClientSession(RTSPServer& ourServer, unsigned sessionId, int clientSocket, struct sockaddr_in clientAddr)
  : fOurServer(ourServer), fOurSessionId(sessionId),
    fOurServerMediaSession(NULL),
    fClientInputSocket(clientSocket), fClientOutputSocket(clientSocket), fClientAddr(clientAddr),
    fLivenessCheckTask(NULL),
    fSessionIsActive(True), fStreamAfterSETUP(False),
    fTCPStreamIdCount(0), fNumStreamStates(0), fStreamStates(NULL), fRecursionCount(0) {
  // Arrange to handle incoming requests:
  resetRequestBuffer();
  envir().taskScheduler().turnOnBackgroundReadHandling(fClientInputSocket,
     (TaskScheduler::BackgroundHandlerProc*)&incomingRequestHandler, this);
  noteLiveness();
}

RTSPServer::RTSPClientSession::~RTSPClientSession() 
{
    closeSockets();

    reclaimStreamStates();

    if (fOurServerMediaSession != NULL) 
    {
        fOurServerMediaSession->decrementReferenceCount();
        if (fOurServerMediaSession->referenceCount() == 0) 
        {
            fOurServer.removeServerMediaSession(fOurServerMediaSession);
            fOurServerMediaSession = NULL;
        }
    }
}

void RTSPServer::RTSPClientSession::closeSockets() {
  // Turn off any liveness checking:
  envir().taskScheduler().unscheduleDelayedTask(fLivenessCheckTask);

  // Turn off background read handling:
  envir().taskScheduler().turnOffBackgroundReadHandling(fClientInputSocket);

  if (fClientOutputSocket != fClientInputSocket) ::closeSocket(fClientOutputSocket);
  ::closeSocket(fClientInputSocket);

  fClientInputSocket = fClientOutputSocket = -1;
}

void RTSPServer::RTSPClientSession::reclaimStreamStates() 
{
    delete[] fStreamStates; fStreamStates = NULL;
    fNumStreamStates = 0;
}

void RTSPServer::RTSPClientSession::resetRequestBuffer() 
{
    fRequestBytesAlreadySeen = 0;
    fRequestBufferBytesLeft = sizeof fRequestBuffer;
    fLastCRLF = &fRequestBuffer[-3]; // hack: Ensures that we don't think we have end-of-msg if the data starts with <CR><LF>
}

void RTSPServer::RTSPClientSession::incomingRequestHandler(void* instance, int /*mask*/) 
{
    RTSPClientSession* session = (RTSPClientSession*)instance;
    session->incomingRequestHandler1();
}

void RTSPServer::RTSPClientSession::incomingRequestHandler1() 
{
    struct sockaddr_in dummy; // 'from' address, meaningless in this case

    int bytesRead = readSocket(envir(), fClientInputSocket, &fRequestBuffer[fRequestBytesAlreadySeen], fRequestBufferBytesLeft, dummy);
    handleRequestBytes(bytesRead);
}

void RTSPServer::RTSPClientSession::handleAlternativeRequestByte(void* instance, u_int8_t requestByte) 
{
    RTSPClientSession* session = (RTSPClientSession*)instance;
    session->handleAlternativeRequestByte1(requestByte);
}

void RTSPServer::RTSPClientSession::handleAlternativeRequestByte1(u_int8_t requestByte) 
{
    // Add this character to our buffer; then try to handle the data that we have buffered so far:
    if (fRequestBufferBytesLeft == 0|| fRequestBytesAlreadySeen >= RTSP_BUFFER_SIZE) return;
    fRequestBuffer[fRequestBytesAlreadySeen] = requestByte;
    handleRequestBytes(1);
}

void RTSPServer::RTSPClientSession::handleRequestBytes(int newBytesRead) 
{
    ++fRecursionCount;

    do 
    {
        noteLiveness();
    
        if (newBytesRead <= 0 || (unsigned)newBytesRead >= fRequestBufferBytesLeft) 
        {
            // Either the client socket has died, or the request was too big for us.
            // Terminate this connection:
#ifdef DEBUG
            fprintf(stderr, 
                    "RTSPClientSession[%p]::handleRequestBytes() read %d new bytes (of %d); terminating connection!\n", 
                    this, newBytesRead, fRequestBufferBytesLeft);
#endif
            fSessionIsActive = False;
            break;
        }
    
        Boolean endOfMsg = False;
        unsigned char* ptr = &fRequestBuffer[fRequestBytesAlreadySeen];
#ifdef DEBUG
        ptr[newBytesRead] = '\0';
        fprintf(stderr, 
                "RTSPClientSession[%p]::handleRequestBytes() read %d new bytes:%s\n", 
                this, newBytesRead, ptr);
#endif
    
        if (fClientOutputSocket != fClientInputSocket) 
        {
            // We're doing RTSP-over-HTTP tunneling, and input commands are assumed to have been Base64-encoded.
            // We therefore Base64-decode as much of this new data as we can (i.e., up to a multiple of 4 bytes):
        }
    
        // Look for the end of the message: <CR><LF><CR><LF>
        unsigned char *tmpPtr = fLastCRLF + 2;
        if (tmpPtr < fRequestBuffer) tmpPtr = fRequestBuffer;
        while (tmpPtr < &ptr[newBytesRead-1]) 
        {
            if (*tmpPtr == '\r' && *(tmpPtr+1) == '\n') 
            {
                if (tmpPtr - fLastCRLF == 2) 
                { // This is it:
                    endOfMsg = True;
                    break;
                }
                fLastCRLF = tmpPtr;
            }
            ++tmpPtr;
        }
    
        fRequestBufferBytesLeft -= newBytesRead;
        fRequestBytesAlreadySeen += newBytesRead;
    
        if (!endOfMsg) break; // subsequent reads will be needed to complete the request
    
        // Parse the request string into command name and 'CSeq', then handle the command:
        fRequestBuffer[fRequestBytesAlreadySeen] = '\0';
        char cmdName[RTSP_PARAM_STRING_MAX];
        char urlPreSuffix[RTSP_PARAM_STRING_MAX];
        char urlSuffix[RTSP_PARAM_STRING_MAX];
        char cseq[RTSP_PARAM_STRING_MAX];
        unsigned contentLength;
        if (parseRTSPRequestString((char*)fRequestBuffer, fRequestBytesAlreadySeen,
                                    cmdName, sizeof cmdName,
                                    urlPreSuffix, sizeof urlPreSuffix,
                                    urlSuffix, sizeof urlSuffix,
                                    cseq, sizeof cseq,
                                    contentLength)) 
        {
#ifdef DEBUG
            fprintf(stderr, 
                    "parseRTSPRequestString() succeeded, returning cmdName \"%s\", urlPreSuffix \"%s\", urlSuffix \"%s\", CSeq \"%s\", Content-Length %u, with %ld bytes following the message.\n", 
                    cmdName, urlPreSuffix, urlSuffix, cseq, contentLength, ptr + newBytesRead - (tmpPtr + 2));
#endif
            // If there was a "Content-Length:" header, then make sure we've received all of the data that it specified:
            if (ptr + newBytesRead < tmpPtr + 2 + contentLength) break; // we still need more data; subsequent reads will give it to us 
      
            if (strcmp(cmdName, "OPTIONS") == 0) 
            {
                handleCmd_OPTIONS(cseq);
            } 
            else if (strcmp(cmdName, "DESCRIBE") == 0) 
            {
                handleCmd_DESCRIBE(cseq, urlPreSuffix, urlSuffix, (char const*)fRequestBuffer);
            } 
            else if (strcmp(cmdName, "SETUP") == 0) 
            {
                //unsigned s=0;
                handleCmd_SETUP(cseq, urlPreSuffix, urlSuffix, (char const*)fRequestBuffer);
            } 
            else if (strcmp(cmdName, "TEARDOWN") == 0
                    || strcmp(cmdName, "PLAY") == 0
                    || strcmp(cmdName, "PAUSE") == 0
                    || strcmp(cmdName, "GET_PARAMETER") == 0
                    || strcmp(cmdName, "SET_PARAMETER") == 0) 
            {
                handleCmd_withinSession(cmdName, urlPreSuffix, urlSuffix, cseq, (char const*)fRequestBuffer);
            } 
            else 
            {
                handleCmd_notSupported(cseq);
            }
        } 
        else 
        {
#ifdef DEBUG
            fprintf(stderr, "parseRTSPRequestString() failed\n");
#endif
            handleCmd_bad(cseq);
        }
    
#ifdef DEBUG
        fprintf(stderr, "sending response: %s", fResponseBuffer);
#endif
        send(fClientOutputSocket, (char const*)fResponseBuffer, strlen((char*)fResponseBuffer), 0);
    
        if (strcmp(cmdName, "SETUP") == 0 && fStreamAfterSETUP) 
        {
            // The client has asked for streaming to commence now, rather than after a
            // subsequent "PLAY" command.  So, simulate the effect of a "PLAY" command:
            handleCmd_withinSession("PLAY", urlPreSuffix, urlSuffix, cseq,
                                    (char const*)fRequestBuffer);
        }
    
        resetRequestBuffer(); // to prepare for any subsequent request
    } while (0);

    --fRecursionCount;
    if (!fSessionIsActive) 
    {
        if (fRecursionCount > 0) closeSockets(); else delete this;
        // Note: The "fRecursionCount" test is for a pathological situation where we got called recursively while handling a command.
        // In such a case we don't want to actually delete ourself until we leave the outermost call.
    }
}

// Handler routines for specific RTSP commands:

static char const* allowedCommandNames
= "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER";

void RTSPServer::RTSPClientSession::handleCmd_bad(char const* /*cseq*/) 
{
    // Don't do anything with "cseq", because it might be nonsense
    snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
            "RTSP/1.0 400 Bad Request\r\n%sAllow: %s\r\n\r\n",
            dateHeader(), allowedCommandNames);
}

void RTSPServer::RTSPClientSession::handleCmd_notSupported(char const* cseq) 
{
    snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
           "RTSP/1.0 405 Method Not Allowed\r\nCSeq: %s\r\n%sAllow: %s\r\n\r\n",
           cseq, dateHeader(), allowedCommandNames);
}

void RTSPServer::RTSPClientSession::handleCmd_notFound(char const* cseq) 
{
    snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
           "RTSP/1.0 404 Stream Not Found\r\nCSeq: %s\r\n%s\r\n",
           cseq, dateHeader());
    fSessionIsActive = False; // triggers deletion of ourself after responding
}

void RTSPServer::RTSPClientSession::handleCmd_unsupportedTransport(char const* cseq) 
{
    snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
           "RTSP/1.0 461 Unsupported Transport\r\nCSeq: %s\r\n%s\r\n",
           cseq, dateHeader());
    fSessionIsActive = False; // triggers deletion of ourself after responding
}

void RTSPServer::RTSPClientSession::handleCmd_OPTIONS(char const* cseq) 
{
    if (NULL == g_pstCallback || NULL == g_pstCallback->options)
    {
        snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
                "RTSP/1.0 200 OK\r\nCSeq: %s\r\n%sPublic: %s\r\n\r\n",
                cseq, dateHeader(), allowedCommandNames);
    }
    else
    {
        char buf[128];
        g_pstCallback->options(buf);
        snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
                "RTSP/1.0 200 OK\r\nCSeq: %s\r\n%sPublic: %s\r\n\r\n",
                cseq, dateHeader(), buf);
    }
}

void RTSPServer::RTSPClientSession::handleCmd_DESCRIBE( char const* cseq,
                                                        char const* urlPreSuffix, 
                                                        char const* urlSuffix,
                                                        char const* fullRequestStr) 
{
    char* sdpDescription = NULL;
    char* miscSDP = NULL;
    char* rtspURL = NULL;

    int   iCallbackRet = 0;
    float fDuration = 0.0;
    bool  bCallback = (NULL == g_pstCallback || NULL == g_pstCallback->describe) ? false : true;

    miscSDP = new char[1024];
    ::memset(miscSDP, 0x00, 1024);

    do 
    {
        char urlTotalSuffix[RTSP_PARAM_STRING_MAX];
        if (strlen(urlPreSuffix) + strlen(urlSuffix) + 2 > sizeof urlTotalSuffix) 
        {
            handleCmd_bad(cseq);
            break;
        }
        urlTotalSuffix[0] = '\0';
        if (urlPreSuffix[0] != '\0') 
        {
            strcat(urlTotalSuffix, urlPreSuffix);
            strcat(urlTotalSuffix, "/");
        }
        strcat(urlTotalSuffix, urlSuffix);
      
        if (!authenticationOK("DESCRIBE", cseq, urlTotalSuffix, fullRequestStr)) break;
    
        // We should really check that the request contains an "Accept:" #####
        // for "application/sdp", because that's what we're sending back #####

        // Begin by looking up the "ServerMediaSession" object for the specified "urlTotalSuffix":
        
        //1. let callback confirm the url is exist or not
        //2. let callback return sdp description, we can set a large buf for callback
        if (true == bCallback)
        {
            g_pstCallback->describe(&iCallbackRet, urlTotalSuffix, miscSDP, &fDuration);
            if ( 0 == iCallbackRet)
            {
                handleCmd_notFound(cseq);
                break;
            }
        }
        // if no exist session , then create it 
        ServerMediaSession* session = fOurServer.lookupServerMediaSession(urlTotalSuffix);
        if (session == NULL && false == bCallback) 
        {
            handleCmd_notFound(cseq);
            break;
        }
        else if (session == NULL)
        {
            session = ServerMediaSession::createNew(envir(), 
                                                    urlSuffix, 
                                                    "AnFengDe Info", 
                                                    "AnFengDe Description", 
                                                    True);
            if (session == NULL) 
            {
                handleCmd_notFound(cseq);
                break;
            }
            ServerMediaSubsession* psmss = new ServerMediaSubsession(envir());
            session->addSubsession(psmss);
            fOurServer.addServerMediaSession(session);
        }
    
        // Then, assemble a SDP description for this session:
        sdpDescription = session->generateSDPDescription(miscSDP, fDuration);
        if (::strlen(sdpDescription) == 0) 
        {
            // This usually means that a file name that was specified for a
            // "ServerMediaSubsession" does not exist.
            snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
                    "RTSP/1.0 404 File Not Found, Or In Incorrect Format\r\n"
                    "CSeq: %s\r\n"
                    "%s\r\n",
                    cseq,
                    dateHeader());
            break;
        }
        unsigned sdpDescriptionSize = strlen(sdpDescription);
    
        // Also, generate our RTSP URL, for the "Content-Base:" header
        // (which is necessary to ensure that the correct URL gets used in
        // subsequent "SETUP" requests).
        rtspURL = fOurServer.rtspURL(session, fClientInputSocket);
    
        snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
                 "RTSP/1.0 200 OK\r\nCSeq: %s\r\n"
                 "%s"
                 "Content-Base: %s/\r\n"
                 "Content-Type: application/sdp\r\n"
                 "Content-Length: %d\r\n\r\n"
                 "%s",
                 cseq,
                 dateHeader(),
                 rtspURL,
                 sdpDescriptionSize,
                 sdpDescription);
    } while (0);
    
    if (miscSDP)        delete[] miscSDP;
    if (sdpDescription) delete[] sdpDescription;
    if (rtspURL)        delete[] rtspURL;
}

typedef enum StreamingMode {
  RTP_UDP,
  RTP_TCP,
  RAW_UDP
} StreamingMode;

static void parseTransportHeader(char const* buf,
                                 StreamingMode& streamingMode,
                                 char*& streamingModeString,
                                 char*& destinationAddressStr,
                                 u_int8_t& destinationTTL,
                                 portNumBits& clientRTPPortNum, // if UDP
                                 portNumBits& clientRTCPPortNum, // if UDP
                                 unsigned char& rtpChannelId, // if TCP
                                 unsigned char& rtcpChannelId // if TCP
                                 ) 
{
    // Initialize the result parameters to default values:
    streamingMode = RTP_UDP;
    streamingModeString = NULL;
    destinationAddressStr = NULL;
    destinationTTL = 255;
    clientRTPPortNum = 0;
    clientRTCPPortNum = 1;
    rtpChannelId = rtcpChannelId = 0xFF;

    portNumBits p1, p2;
    unsigned ttl, rtpCid, rtcpCid;

    // First, find "Transport:"
    while (1) 
    {
        if (*buf == '\0') return; // not found
        if (_strncasecmp(buf, "Transport: ", 11) == 0) break;
        ++buf;
    }

    // Then, run through each of the fields, looking for ones we handle:
    char const* fields = buf + 11;
    char* field = strDupSize(fields);
    while (sscanf(fields, "%[^;]", field) == 1) 
    {
        if (strcmp(field, "RTP/AVP/TCP") == 0) 
        {
            streamingMode = RTP_TCP;
        } 
        else if (strcmp(field, "RAW/RAW/UDP") == 0 
                 || strcmp(field, "MP2T/H2221/UDP") == 0) 
        {
            streamingMode = RAW_UDP;
            streamingModeString = strDup(field);
        } 
        else if (_strncasecmp(field, "destination=", 12) == 0) 
        {
            delete[] destinationAddressStr;
            destinationAddressStr = strDup(field + 12);
        } 
        else if (sscanf(field, "ttl%u", &ttl) == 1) 
        {
            destinationTTL = (u_int8_t)ttl;
        } 
        else if (sscanf(field, "client_port=%hu-%hu", &p1, &p2) == 2) 
        {
            clientRTPPortNum = p1;
            clientRTCPPortNum = streamingMode == RAW_UDP ? 0 : p2; // ignore the second port number if the client asked for raw UDP
        } 
        else if (sscanf(field, "client_port=%hu", &p1) == 1) 
        {
            clientRTPPortNum = p1;
            clientRTCPPortNum = streamingMode == RAW_UDP ? 0 : p1 + 1;
        } 
        else if (sscanf(field, "interleaved=%u-%u", &rtpCid, &rtcpCid) == 2) 
        {
            rtpChannelId = (unsigned char)rtpCid;
            rtcpChannelId = (unsigned char)rtcpCid;
        }

        fields += strlen(field);
        while (*fields == ';') ++fields; // skip over separating ';' chars
        if (*fields == '\0' || *fields == '\r' || *fields == '\n') break;
    }
    delete[] field;
}

static Boolean parsePlayNowHeader(char const* buf) 
{
    // Find "x-playNow:" header, if present
    while (1) 
    {
        if (*buf == '\0') return False; // not found
        if (_strncasecmp(buf, "x-playNow:", 10) == 0) break;
        ++buf;
    }

    return True;
}

void RTSPServer::RTSPClientSession::handleCmd_SETUP(char const* cseq,
                                                    char const* urlPreSuffix, 
                                                    char const* urlSuffix,
                                                    char const* fullRequestStr) 
{
    // "urlPreSuffix" should be the session (stream) name, and
    // "urlSuffix" should be the subsession (track) name.
    char const* streamName = urlPreSuffix;
    char const* trackId = urlSuffix;
    bool  bCallback = (NULL == g_pstCallback || NULL == g_pstCallback->setup) ? false : true;

    // Check whether we have existing session state, and, if so, whether it's
    // for the session that's named in "streamName".  (Note that we don't
    // support more than one concurrent session on the same client connection.) #####
    if (fOurServerMediaSession != NULL
        && strcmp(streamName, fOurServerMediaSession->streamName()) != 0) 
    {
        fOurServerMediaSession = NULL;
    }

    if (fOurServerMediaSession == NULL) 
    {
        // Look up the "ServerMediaSession" object for the specified stream:
        if (streamName[0] != '\0' ||
            fOurServer.lookupServerMediaSession("") != NULL) 
        { // normal case
        } 
        else 
        { // weird case: there was no track id in the URL
            streamName = urlSuffix;
            trackId = NULL;
        }
        fOurServerMediaSession = fOurServer.lookupServerMediaSession(streamName);
        if (fOurServerMediaSession == NULL) 
        {
            handleCmd_notFound(cseq);
            return;
        }

        fOurServerMediaSession->incrementReferenceCount();

        // Set up our array of states for this session's subsessions (tracks):
        reclaimStreamStates();
        ServerMediaSubsessionIterator iter(*fOurServerMediaSession);
        for (fNumStreamStates = 0; iter.next() != NULL; ++fNumStreamStates) 
        {
            //counting number of subsession
        }
        fStreamStates = new struct streamState[fNumStreamStates];
        iter.reset();
        ServerMediaSubsession* subsession;
        for (unsigned i = 0; i < fNumStreamStates; ++i) 
        {
            subsession = iter.next();
            fStreamStates[i].subsession = subsession;
            fStreamStates[i].streamToken = NULL; // for now; reset by SETUP later
        }
    } //fOurServerMediaSession == NULL

    // Look up information for the specified subsession (track):
    ServerMediaSubsession* subsession = NULL;
    unsigned streamNum;
    if (trackId != NULL && trackId[0] != '\0') 
    { // normal case
        for (streamNum = 0; streamNum < fNumStreamStates; ++streamNum) 
        {
            subsession = fStreamStates[streamNum].subsession;
            if (subsession != NULL && strcmp(trackId, subsession->trackId()) == 0) break;
        }
        if (streamNum >= fNumStreamStates) 
        {
            // The specified track id doesn't exist, so this request fails:
            handleCmd_notFound(cseq);
            return;
        }
    } 
    else 
    {
        // Weird case: there was no track id in the URL.
        // This works only if we have only one subsession:
        if (fNumStreamStates != 1) 
        {
            handleCmd_bad(cseq);
            return;
        }
        streamNum = 0;
        subsession = fStreamStates[streamNum].subsession;
    }
    // ASSERT: subsession != NULL

    // Look for a "Transport:" header in the request string,
    // to extract client parameters:
    StreamingMode streamingMode;
    char* streamingModeString = NULL; // set when RAW_UDP streaming is specified
    char* clientsDestinationAddressStr;
    u_int8_t clientsDestinationTTL;
    portNumBits clientRTPPortNum, clientRTCPPortNum;
    unsigned char rtpChannelId, rtcpChannelId;
    
    parseTransportHeader(fullRequestStr, 
                         streamingMode, 
                         streamingModeString,
                         clientsDestinationAddressStr, 
                         clientsDestinationTTL,
                         clientRTPPortNum, 
                         clientRTCPPortNum,
                         rtpChannelId, 
                         rtcpChannelId);

    if ((streamingMode == RTP_TCP && rtpChannelId == 0xFF) 
        || (streamingMode != RTP_TCP && fClientOutputSocket != fClientInputSocket)) 
    {
        // An anomolous situation, caused by a buggy client.  Either:
        //     1/ TCP streaming was requested, but with no "interleaving=" fields.  (QuickTime Player sometimes does this.), or
        //     2/ TCP streaming was not requested, but we're doing RTSP-over-HTTP tunneling (which implies TCP streaming).
        // In either case, we assume TCP streaming, and set the RTP and RTCP channel ids to proper values:
        streamingMode = RTP_TCP;
        rtpChannelId = fTCPStreamIdCount; rtcpChannelId = fTCPStreamIdCount+1;
    }
    fTCPStreamIdCount += 2;

    Port clientRTPPort(clientRTPPortNum);
    Port clientRTCPPort(clientRTCPPortNum);

    // Next, check whether a "Range:" header is present in the request.
    // This isn't legal, but some clients do this to combine "SETUP" and "PLAY":
    double rangeStart = 0.0, rangeEnd = 0.0;
    fStreamAfterSETUP = parseRangeHeader(fullRequestStr, rangeStart, rangeEnd) 
                        || parsePlayNowHeader(fullRequestStr);

    // Then, get server parameters from the 'subsession':
    int tcpSocketNum = streamingMode == RTP_TCP ? fClientOutputSocket : -1;
    netAddressBits destinationAddress = 0;
    u_int8_t destinationTTL = 255;
#ifdef RTSP_ALLOW_CLIENT_DESTINATION_SETTING
    if (clientsDestinationAddressStr != NULL) 
    {
        // Use the client-provided "destination" address.
        // Note: This potentially allows the server to be used in denial-of-service
        // attacks, so don't enable this code unless you're sure that clients are
        // trusted.
        destinationAddress = our_inet_addr(clientsDestinationAddressStr);
    }
    // Also use the client-provided TTL.
    destinationTTL = clientsDestinationTTL;
#endif
    delete[] clientsDestinationAddressStr;
    unsigned short rtpServerPort = 0;
    if ( bCallback )
    {
        g_pstCallback->setup(fOurSessionId, urlSuffix, clientRTPPort.num(), &rtpServerPort);
    }

    Port serverRTPPort(rtpServerPort);
    Port serverRTCPPort(rtpServerPort+1);

    // Make sure that we transmit on the same interface that's used by the client (in case we're a multi-homed server):
    struct sockaddr_in sourceAddr; SOCKLEN_T namelen = sizeof sourceAddr;
    getsockname(fClientInputSocket, (struct sockaddr*)&sourceAddr, &namelen);
    netAddressBits origSendingInterfaceAddr = SendingInterfaceAddr;
    netAddressBits origReceivingInterfaceAddr = ReceivingInterfaceAddr;
    // NOTE: The following might not work properly, so we ifdef it out for now:
#ifdef HACK_FOR_MULTIHOMED_SERVERS
    ReceivingInterfaceAddr = SendingInterfaceAddr = sourceAddr.sin_addr.s_addr;
#endif

    subsession->getStreamParameters(fOurSessionId, 
                                    fClientAddr.sin_addr.s_addr,
                                    clientRTPPort,
                                    tcpSocketNum, 
                                    rtpChannelId, 
                                    rtcpChannelId,
                                    destinationAddress, 
                                    destinationTTL, 
                                    serverRTPPort,
                                    fStreamStates[streamNum].streamToken);
    SendingInterfaceAddr = origSendingInterfaceAddr;
    ReceivingInterfaceAddr = origReceivingInterfaceAddr;

    struct in_addr destinationAddr; destinationAddr.s_addr = destinationAddress;
    char* destAddrStr = strDup(our_inet_ntoa(destinationAddr));
    char* sourceAddrStr = strDup(our_inet_ntoa(sourceAddr.sin_addr));

    switch (streamingMode) 
    {
    case RTP_UDP: 
        {
            snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
               "RTSP/1.0 200 OK\r\n"
               "CSeq: %s\r\n"
               "%s"
               "Transport: RTP/AVP;unicast;destination=%s;source=%s;client_port=%d-%d;server_port=%d-%d\r\n"
               "Session: %08X\r\n\r\n",
               cseq,
               dateHeader(),
               destAddrStr, sourceAddrStr, ntohs(clientRTPPort.num()), ntohs(clientRTCPPort.num()), ntohs(serverRTPPort.num()), ntohs(serverRTCPPort.num()),
               fOurSessionId);
            break;
        }
    case RTP_TCP: 
        {
            snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
               "RTSP/1.0 200 OK\r\n"
               "CSeq: %s\r\n"
               "%s"
               "Transport: RTP/AVP/TCP;unicast;destination=%s;source=%s;interleaved=%d-%d\r\n"
               "Session: %08X\r\n\r\n",
               cseq,
               dateHeader(),
               destAddrStr, sourceAddrStr, rtpChannelId, rtcpChannelId,
               fOurSessionId);
            break;
        }
    case RAW_UDP: 
        {
            snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
               "RTSP/1.0 200 OK\r\n"
               "CSeq: %s\r\n"
               "%s"
               "Transport: %s;unicast;destination=%s;source=%s;client_port=%d;server_port=%d\r\n"
               "Session: %08X\r\n\r\n",
               cseq,
               dateHeader(),
               streamingModeString, destAddrStr, sourceAddrStr, ntohs(clientRTPPort.num()), ntohs(serverRTPPort.num()),
               fOurSessionId);
            break;
        }
    }
    delete[] destAddrStr; delete[] sourceAddrStr; delete[] streamingModeString;
}

void RTSPServer::RTSPClientSession::handleCmd_withinSession(char const* cmdName,
                                                            char const* urlPreSuffix, 
                                                            char const* urlSuffix,
                                                            char const* cseq, 
                                                            char const* fullRequestStr) 
{
    // This will either be:
    // - an operation on the entire server, if "urlPreSuffix" is "", and "urlSuffix" is "*" (i.e., the special "*" URL), or
    // - a non-aggregated operation, if "urlPreSuffix" is the session (stream)
    //   name and "urlSuffix" is the subsession (track) name, or
    // - an aggregated operation, if "urlSuffix" is the session (stream) name,
    //   or "urlPreSuffix" is the session (stream) name, and "urlSuffix" is empty,
    //   or "urlPreSuffix" and "urlSuffix" are both nonempty, but when concatenated, (with "/") form the session (stream) name.
    // Begin by figuring out which of these it is:
    ServerMediaSubsession* subsession;
    if (urlPreSuffix[0] == '\0' && urlSuffix[0] == '*' && urlSuffix[1] == '\0') 
    {
        // An operation on the entire server.  This works only for GET_PARAMETER and SET_PARAMETER:
        if (strcmp(cmdName, "GET_PARAMETER") == 0) 
        {
            handleCmd_GET_PARAMETER(NULL, cseq, fullRequestStr);
        } 
        else if (strcmp(cmdName, "SET_PARAMETER") == 0) 
        {
            handleCmd_SET_PARAMETER(NULL, cseq, fullRequestStr);
        } 
        else 
        {
            handleCmd_notSupported(cseq);
        }
        return;
    } 
    else if (fOurServerMediaSession == NULL) 
    { // There wasn't a previous SETUP!
        handleCmd_notSupported(cseq);
        return;
    } 
    else if (urlSuffix[0] != '\0' && strcmp(fOurServerMediaSession->streamName(), urlPreSuffix) == 0) 
    {
        // Non-aggregated operation.
        // Look up the media subsession whose track id is "urlSuffix":
        ServerMediaSubsessionIterator iter(*fOurServerMediaSession);
        while ((subsession = iter.next()) != NULL) 
        {
            if (strcmp(subsession->trackId(), urlSuffix) == 0) break; // success
        }
        if (subsession == NULL) 
        { // no such track!
            handleCmd_notFound(cseq);
            return;
        }
    } 
    else if (strcmp(fOurServerMediaSession->streamName(), urlSuffix) == 0 ||
             (urlSuffix[0] == '\0' && strcmp(fOurServerMediaSession->streamName(), urlPreSuffix) == 0)) 
    {
        // Aggregated operation
        subsession = NULL;
    } 
    else if (urlPreSuffix[0] != '\0' && urlSuffix[0] != '\0') 
    {
        // Aggregated operation, if <urlPreSuffix>/<urlSuffix> is the session (stream) name:
        unsigned const urlPreSuffixLen = strlen(urlPreSuffix);
        if (strncmp(fOurServerMediaSession->streamName(), urlPreSuffix, urlPreSuffixLen) == 0 
            && fOurServerMediaSession->streamName()[urlPreSuffixLen] == '/' 
            && strcmp(&(fOurServerMediaSession->streamName())[urlPreSuffixLen+1], urlSuffix) == 0) 
        {
            subsession = NULL;
        } 
        else 
        {
            handleCmd_notFound(cseq);
            return;
        }
    } 
    else 
    { // the request doesn't match a known stream and/or track at all!
        handleCmd_notFound(cseq);
        return;
    }

    if (strcmp(cmdName, "TEARDOWN") == 0) 
    {
        handleCmd_TEARDOWN(subsession, cseq);
    } 
    else if (strcmp(cmdName, "PLAY") == 0) 
    {
        handleCmd_PLAY(subsession, cseq, fullRequestStr);
    } else if (strcmp(cmdName, "PAUSE") == 0) 
    {
        handleCmd_PAUSE(subsession, cseq);
    } 
    else if (strcmp(cmdName, "GET_PARAMETER") == 0) 
    {
        handleCmd_GET_PARAMETER(subsession, cseq, fullRequestStr);
    } 
    else if (strcmp(cmdName, "SET_PARAMETER") == 0) 
    {
        handleCmd_SET_PARAMETER(subsession, cseq, fullRequestStr);
    }
}

void RTSPServer::RTSPClientSession
::handleCmd_TEARDOWN(ServerMediaSubsession* subsession, char const* cseq) 
{
    bool bCallback = (NULL == g_pstCallback || NULL == g_pstCallback->teardown) ? false : true;
    if (bCallback)
    {
        g_pstCallback->teardown(fOurSessionId);
    }
    snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
           "RTSP/1.0 200 OK\r\nCSeq: %s\r\n%s\r\n",
           cseq, dateHeader());
    fSessionIsActive = False; // triggers deletion of ourself after responding
}

static Boolean parseScaleHeader(char const* buf, float& scale) 
{
    // Initialize the result parameter to a default value:
    scale = 1.0;

    // First, find "Scale:"
    while (1) 
    {
        if (*buf == '\0') return False; // not found
        if (_strncasecmp(buf, "Scale: ", 7) == 0) break;
        ++buf;
    }

    // Then, run through each of the fields, looking for ones we handle:
    char const* fields = buf + 7;
    while (*fields == ' ') ++fields;
    float sc;
    if (sscanf(fields, "%f", &sc) == 1) 
    {
        scale = sc;
    } 
    else 
    {
        return False; // The header is malformed
    }

    return True;
}

void RTSPServer::RTSPClientSession::handleCmd_PLAY(ServerMediaSubsession* subsession, 
                                                   char const* cseq,
                                                   char const* fullRequestStr) 
{
    bool  bCallback = (NULL == g_pstCallback || NULL == g_pstCallback->play) ? false : true;

    // Parse the client's "Scale:" header, if any:
    float scale;
    Boolean sawScaleHeader = parseScaleHeader(fullRequestStr, scale);

    // Try to set the stream's scale factor to this value:
    if (subsession == NULL /*aggregate op*/) 
    {
        fOurServerMediaSession->testScaleFactor(scale);
    } 
    else 
    {
        subsession->testScaleFactor(scale);
    }

    char buf[100];
    char* scaleHeader;
    if (!sawScaleHeader) 
    {
        buf[0] = '\0'; // Because we didn't see a Scale: header, don't send one back
    } 
    else 
    {
        sprintf(buf, "Scale: %f\r\n", scale);
    }
    scaleHeader = strDup(buf);

    // Parse the client's "Range:" header, if any:
    double rangeStart = 0.0, rangeEnd = 0.0;
    Boolean sawRangeHeader = parseRangeHeader(fullRequestStr, rangeStart, rangeEnd);

    // Make sure that "rangeStart" and "rangeEnd" (from the client's "Range:" header) have sane values
    // before we send back our own "Range:" header in our response:
    if (rangeStart < 0.0) rangeStart = 0.0;
    
    if (rangeEnd < 0.0) rangeEnd = 0.0;
    
    if ((scale > 0.0 && rangeStart > rangeEnd && rangeEnd > 0.0) 
        || (scale < 0.0 && rangeStart < rangeEnd)) 
    {
        // "rangeStart" and "rangeEnd" were the wrong way around; swap them:
        double tmp = rangeStart;
        rangeStart = rangeEnd;
        rangeEnd = tmp;
    }

    // Create the "Range:" header that we'll send back in our response.
    // (Note that we do this after seeking, in case the seeking operation changed the range start time.)
    char* rangeHeader;
    if (!sawRangeHeader) 
    {
        buf[0] = '\0'; // Because we didn't see a Range: header, don't send one back
    } 
    else if (rangeEnd == 0.0 && scale >= 0.0) 
    {
        sprintf(buf, "Range: npt=%.3f-\r\n", rangeStart);
    } 
    else 
    {
        sprintf(buf, "Range: npt=%.3f-%.3f\r\n", rangeStart, rangeEnd);
    }
    rangeHeader = strDup(buf);
 

    if (bCallback)
        g_pstCallback->play(fOurSessionId, scale, rangeStart, rangeEnd);
  
    // Fill in the response:
    snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
               "RTSP/1.0 200 OK\r\n"
               "CSeq: %s\r\n"
               "%s"
               "%s"
               "%s"
               "Session: %08X\r\n",
               cseq,
               dateHeader(),
               scaleHeader,
               rangeHeader,
               fOurSessionId);

    delete[] rangeHeader;
    delete[] scaleHeader;
}

void RTSPServer::RTSPClientSession::handleCmd_PAUSE(ServerMediaSubsession* subsession, 
                                                    char const* cseq) 
{
    bool  bCallback = (NULL == g_pstCallback || NULL == g_pstCallback->pause) ? false : true;
    if (bCallback)
    {
        //callback for pause
        g_pstCallback->pause(fOurSessionId);
    }
    snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
                "RTSP/1.0 200 OK\r\nCSeq: %s\r\n%sSession: %08X\r\n\r\n",
                cseq, dateHeader(), fOurSessionId);
}

void RTSPServer::RTSPClientSession::handleCmd_GET_PARAMETER(ServerMediaSubsession* /*subsession*/, 
                                                            char const* cseq,
                                                            char const* /*fullRequestStr*/) 
{
    // By default, we implement "GET_PARAMETER" just as a 'keep alive', and send back an empty response.
    // (If you want to handle "GET_PARAMETER" properly, you can do so by defining a subclass of "RTSPServer"
    // and "RTSPServer::RTSPClientSession", and then reimplement this virtual function in your subclass.)
    snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
           "RTSP/1.0 200 OK\r\nCSeq: %s\r\n%sSession: %08X\r\n\r\n",
           cseq, dateHeader(), fOurSessionId);
}

void RTSPServer::RTSPClientSession::handleCmd_SET_PARAMETER(ServerMediaSubsession* /*subsession*/, 
                                                            char const* cseq,
                                                            char const* /*fullRequestStr*/) 
{
    // By default, we implement "SET_PARAMETER" just as a 'keep alive', and send back an empty response.
    // (If you want to handle "SET_PARAMETER" properly, you can do so by defining a subclass of "RTSPServer"
    // and "RTSPServer::RTSPClientSession", and then reimplement this virtual function in your subclass.)
    snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
           "RTSP/1.0 200 OK\r\nCSeq: %s\r\n%sSession: %08X\r\n\r\n",
           cseq, dateHeader(), fOurSessionId);
}

Boolean RTSPServer::RTSPClientSession::authenticationOK(char const* cmdName, 
                                                        char const* cseq,
                                                        char const* urlSuffix, 
                                                        char const* fullRequestStr) 
{
    // If we weren't set up with an authentication database, we're OK:
    if (fOurServer.fAuthDB == NULL) return True;

    return False;
}

void RTSPServer::RTSPClientSession::noteLiveness() 
{
#ifdef DEBUG
    fprintf(stderr, "Liveness indication from client at %s\n", AddressString(fClientAddr).val());
#endif
    if (fOurServer.fReclamationTestSeconds > 0) 
    {
        envir().taskScheduler()
            .rescheduleDelayedTask( fLivenessCheckTask,
                                    fOurServer.fReclamationTestSeconds*1000000,
                                    (TaskFunc*)livenessTimeoutTask, this);
    }
}

void RTSPServer::RTSPClientSession::noteClientLiveness(RTSPClientSession* clientSession) 
{
    clientSession->noteLiveness();
}

void RTSPServer::RTSPClientSession::livenessTimeoutTask(RTSPClientSession* clientSession) 
{
    // If this gets called, the client session is assumed to have timed out,
    // so delete it:
#ifdef DEBUG
    fprintf(stderr, 
            "RTSP client session from %s has timed out (due to inactivity)\n", 
            AddressString(clientSession->fClientAddr).val());
#endif
    delete clientSession;
}

RTSPServer::RTSPClientSession*
RTSPServer::createNewClientSession(unsigned sessionId, int clientSocket, struct sockaddr_in clientAddr) 
{
    return new RTSPClientSession(*this, sessionId, clientSocket, clientAddr);
}

void RTSPServer::RTSPClientSession
::changeClientInputSocket(int newSocketNum, unsigned char const* extraData, unsigned extraDataSize) 
{
    envir().taskScheduler().turnOffBackgroundReadHandling(fClientInputSocket);
    fClientInputSocket = newSocketNum;
    envir()
        .taskScheduler()
        .turnOnBackgroundReadHandling(  fClientInputSocket,
                                        (TaskScheduler::BackgroundHandlerProc*)&incomingRequestHandler, 
                                        this
                                     );

    // Also write any extra data to our buffer, and handle it:
    if (extraDataSize > 0 && extraDataSize <= fRequestBufferBytesLeft/*sanity check; should always be true*/) 
    {
        unsigned char* ptr = &fRequestBuffer[fRequestBytesAlreadySeen];
        for (unsigned i = 0; i < extraDataSize; ++i) 
        {
            ptr[i] = extraData[i];
        }
        handleRequestBytes(extraDataSize);
    }
}


////////// ServerMediaSessionIterator implementation //////////

RTSPServer::ServerMediaSessionIterator
::ServerMediaSessionIterator(RTSPServer& server)
  : fOurIterator((server.fServerMediaSessions == NULL)
                 ? NULL : HashTable::Iterator::create(*server.fServerMediaSessions)) 
{
}

RTSPServer::ServerMediaSessionIterator::~ServerMediaSessionIterator() 
{
    delete fOurIterator;
}

ServerMediaSession* RTSPServer::ServerMediaSessionIterator::next() 
{
    if (fOurIterator == NULL) return NULL;

    char const* key; // dummy
    return (ServerMediaSession*)(fOurIterator->next(key));
}


////////// UserAuthenticationDatabase implementation //////////

UserAuthenticationDatabase::UserAuthenticationDatabase(char const* realm,
                                                       Boolean passwordsAreMD5)
  : fTable(HashTable::create(STRING_HASH_KEYS)),
    fRealm(strDup(realm == NULL ? "AnFengDe Streaming Media" : realm)),
    fPasswordsAreMD5(passwordsAreMD5) 
{
}

UserAuthenticationDatabase::~UserAuthenticationDatabase() 
{
    delete[] fRealm;

    // Delete the allocated 'password' strings that we stored in the table, and then the table itself:
    char* password;
    while ((password = (char*)fTable->RemoveNext()) != NULL) 
    {
        delete[] password;
    }
    delete fTable;
}

void UserAuthenticationDatabase::addUserRecord(char const* username,
                                               char const* password) 
{
    fTable->Add(username, (void*)(strDup(password)));
}

void UserAuthenticationDatabase::removeUserRecord(char const* username) 
{
    char* password = (char*)(fTable->Lookup(username));
    fTable->Remove(username);
    delete[] password;
}

char const* UserAuthenticationDatabase::lookupPassword(char const* username) 
{
    return (char const*)(fTable->Lookup(username));
}
