/**********************
testClient.hh
For testRtspClient
Added by zhaojin
***********************/
#ifndef _TEST_CLIENT_HH
#define _TEST_CLIENT_HH

#include "liveMedia.hh"

extern Medium* createClient(UsageEnvironment& env, char const* URL, int verbosityLevel, char const* applicationName);
extern RTSPClient* ourRTSPClient;

extern void getOptions(RTSPClient::responseHandler* afterFunc);

extern void getSDPDescription(RTSPClient::responseHandler* afterFunc);

extern void setupSubsession(MediaSubsession* subsession, Boolean streamUsingTCP, RTSPClient::responseHandler* afterFunc);

extern void startPlayingSession(MediaSession* session, double start, double end, float scale, RTSPClient::responseHandler* afterFunc);

extern void PauseSession(MediaSession* session, RTSPClient::responseHandler* responseHandler);

extern void tearDownSession(MediaSession* session, RTSPClient::responseHandler* afterFunc);

extern Authenticator* ourAuthenticator;
extern Boolean allowProxyServers;
extern Boolean controlConnectionUsesTCP;
extern Boolean supportCodecSelection;
extern char const* clientProtocolName;

#endif


