#ifndef _MEDIA_CLIENT_HH
#include "ClientInterface.h"
#endif

ClientInterface* ClientData;


ClientInterface::ClientInterface(UsageEnvironment& env):Medium(env)
{
	ourRTSPClient = NULL;
	//allowProxyServers = False;
	//controlConnectionUsesTCP = True;
	//supportCodecSelection = False;
	//clientProtocolName = "RTSP";
	ourAuthenticator =NULL;
	//CallBackForGetRtcpStatus = NULL;
	//CallBackForGetResult= NULL;
	CallBackForGetBuffer = NULL;
	//CallBackForGetSdp =NULL;


	setupIter = NULL; //fengyu modifyed
	//progName = NULL;
	fileSink = NULL;//fengyu modifyed
	tunnelOverHTTPPortNum = 0;
     session = NULL;
	sessionTimerTask = NULL;
	arrivalCheckTimerTask = NULL;
	interPacketGapCheckTimerTask = NULL;
	qosMeasurementTimerTask = NULL;
	createReceivers = True;
	//notifyOnPacketArrival = False;
	//outputAVIFile = False;
	//interPacketGapMaxTime = 0;
	//totNumPacketsReceived = ~0; // used if checking inter-packet gaps
	//playContinuously = False;
	singleMedium = NULL;
	//streamURL = NULL;
	oneFilePerFrame = False;
	streamUsingTCP = False;
	// syncStreams = False;
	desiredPortNum = 0;
	duration = 0.0f;
	durationSlop = -1.0; // extra seconds to play at the end
	initialSeekTime = 0.0f;
	scale = 1.0f;
	endTime = 0.0f;//fengyu @ i don't know???
	simpleRTPoffsetArg = -1;
	fileSinkBufferSize = 100000;
	socketInputBufferSize = 0;
	areAlreadyShuttingDown = False;
	shutdownExitCode = 1;
	watchVariable = 0;
	madeProgress = False;


}
ClientInterface::~ClientInterface(){
	if (NULL != ourAuthenticator)
	{
		delete ourAuthenticator;
		ourAuthenticator = NULL;
	}
	Medium::close(ourRTSPClient);
	ourRTSPClient = NULL;
}
ClientInterface* ClientInterface
:: createNew() 
{	
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	BasicUsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);
	return new ClientInterface(*env);
}

Medium* ClientInterface::createClient(UsageEnvironment& env, char const* url, int verbosityLevel, char const* applicationName) {
    ClientData=this;
	return ourRTSPClient = RTSPClient::createNew(env,url, verbosityLevel, applicationName, tunnelOverHTTPPortNum);
}

void ClientInterface::getOptions(RTSPClient::responseHandler* afterFunc) { 
	ourRTSPClient->sendOptionsCommand(afterFunc, ourAuthenticator);
}

void ClientInterface::getSDPDescription(RTSPClient::responseHandler* afterFunc) {
	ourRTSPClient->sendDescribeCommand(afterFunc, ourAuthenticator);
}

void ClientInterface::getSetup(MediaSubsession* subsession, Boolean streamUsingTCP, RTSPClient::responseHandler* afterFunc) {
	Boolean forceMulticastOnUnspecified = False;
	ourRTSPClient->sendSetupCommand(*subsession, afterFunc, False, streamUsingTCP, forceMulticastOnUnspecified, ourAuthenticator);
}

void ClientInterface::getPlay(MediaSession* session, double start, double end, float scale, RTSPClient::responseHandler* afterFunc) {
	ourRTSPClient->sendPlayCommand(*session, afterFunc, start, end, scale, ourAuthenticator);
}

void ClientInterface::getPause(MediaSession* session, RTSPClient::responseHandler* responseHandler) {
	ourRTSPClient->sendPauseCommand(*session, responseHandler, ourAuthenticator);
}

void ClientInterface::getTeardown(MediaSession* session, RTSPClient::responseHandler* afterFunc) {
	ourRTSPClient->sendTeardownCommand(*session, afterFunc, ourAuthenticator);
}
int ClientInterface::Start(char* RTSP_URL,doGetBuffer* doGetBuffer)
{
areAlreadyShuttingDown = False;
CallBackForGetBuffer = doGetBuffer;
#if 1
int verbosityLevel = 1; // by default, print verbose output
createClient(envir(),RTSP_URL, verbosityLevel, /*progName*/"HyerVision");
//streamURL = RTSP_URL;
if (ourRTSPClient == NULL) 
{}
else
#endif
{	
	if((m_PlayThrd = CreateThread( 	(LPSECURITY_ATTRIBUTES)NULL, 0,	(LPTHREAD_START_ROUTINE)PlayThrd,(void *)this,	0, NULL)) == NULL)
		return -1;
	}
return 0;
}
void ClientInterface::PlayThrd(LPVOID lParam)
{
	ClientInterface* _this = (ClientInterface*)lParam;
	_this->getOptions(continueAfterOPTIONS);
	_this->setWatchVariable(0);
	_this->envir().taskScheduler().doEventLoop(&(_this->watchVariable )); 
}
int ClientInterface::Pause()
{	getPause(session, continueAfter );
	return 0;
}

int ClientInterface::Resume(double percent)
{	double NTP_time;
	if (duration == 0.0f)
		NTP_time = percent;
	else
		NTP_time = duration * percent;
	getPlay(session, NTP_time, endTime, scale, continueAfter);
	return 0;
}

int ClientInterface::Fast(double resacle)
{	getPlay(session, -0.1, endTime, resacle, continueAfter);
	return 0;
}
int ClientInterface::Slow(double resacle)
{	getPlay(session, -0.1, endTime, resacle, continueAfter);
	return 0;
}

int ClientInterface::Stop()
{	
#if 1
	if (&envir() != NULL) {
		envir().taskScheduler().unscheduleDelayedTask(sessionTimerTask);
		envir().taskScheduler().unscheduleDelayedTask(arrivalCheckTimerTask);
		envir().taskScheduler().unscheduleDelayedTask(interPacketGapCheckTimerTask);
		envir().taskScheduler().unscheduleDelayedTask(qosMeasurementTimerTask);
	}
#endif

	// Teardown, then shutdown, any outstanding RTP/RTCP subsessions
	if (session != NULL) {
		getTeardown(session, continueAfterTEARDOWN);

	} else {
		continueAfterTEARDOWN(ourRTSPClient, 0, NULL);
	}

	return 0;

}
void ClientInterface::continueAfterOPTIONS(RTSPClient* rtspClient, int resultCode, char* resultString) {
	if (NULL != rtspClient)
	{
        if (NULL != resultString)
	{
		delete[] resultString;
	}
	if (NULL != rtspClient)
	{		
        ClientData->getSDPDescription(continueAfterDESCRIBE);
	}	
	}
}
void ClientInterface::continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString) 
{
	if (NULL != rtspClient)
	{
		ClientData->doAfterDESCRIBE(rtspClient, resultCode, resultString);
	}
}
void ClientInterface::doAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString){
	if (resultCode == 0)
{
  char* sdpDescription = resultString;
  // // Create a media session object from this SDP description:
  session = MediaSession::createNew(envir(), sdpDescription);
  delete[] sdpDescription;
  	 if (session != NULL&&session->hasSubsessions())
	{
  // Then, setup the "RTPSource"s for the session:
  MediaSubsessionIterator iter(*session);
  MediaSubsession *subsession;
  Boolean madeProgress = False;
  char const* singleMediumToTest = singleMedium;
  while ((subsession = iter.next()) != NULL) {
    // If we've asked to receive only a single medium, then check this now:
    if (singleMediumToTest != NULL) {
      if (strcmp(subsession->mediumName(), singleMediumToTest) != 0) {
			continue;
      } else {
	// Receive this subsession only
	singleMediumToTest = "xxxxx";
	    // this hack ensures that we get only 1 subsession of this type
      }
    }
    if (createReceivers) {
      if (!subsession->initiate(simpleRTPoffsetArg)) {
	
      } else {
	
	madeProgress = True;	
	if (subsession->rtpSource() != NULL) {}
      }
    } 
    else {
      if (subsession->clientPortNum() == 0) {      } else {
		madeProgress = True;
      }
    }
  }
  
  if (madeProgress) {	
  // Perform additional 'setup' on each subsession, before playing them:
  setupStreams();
  }
     }
}
}

void ClientInterface::continueAfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) 
{
	if (NULL != rtspClient)
	{
        if (resultCode == 0) {
   ClientData->madeProgress = True;
  } else {
  }
 ClientData->setupStreams(); //
	}
}

void ClientInterface::setupStreams() {
  if (setupIter == NULL) //only setup once, zhaojin
	  setupIter = new MediaSubsessionIterator(*session);
	MediaSubsession *subsession;
  while ((subsession = setupIter->next()) != NULL) {
    // We have another subsession left to set up:
    if (subsession->clientPortNum() == 0) continue; // port # was not set
    getSetup(subsession, streamUsingTCP, continueAfterSETUP);
    return;
  }
  // We're done setting up subsessions.
  delete setupIter;
  setupIter = NULL;
  if (madeProgress) 
	{
  // Create output files:
  if (createReceivers) {
	if (True) {
   // } else {
      // Create and start "FileSink"s for each subsession:
      madeProgress = False;
      MediaSubsessionIterator iter(*session);
      while ((subsession = iter.next()) != NULL) {
	if (subsession->readSource() == NULL) continue; // was not initiated
	// Create an output file for each desired stream:
	char outFileName[1000]="test.264";
	  fileSink = FileSink::createNew(envir(), outFileName,
					 fileSinkBufferSize, oneFilePerFrame,this);
	  fileSink->afterGetingFrameData(afterGetFrameData,outFileName);//set CALLBACK function, read data stream,added by zhaojin
	subsession->sink = fileSink;
	if (subsession->sink == NULL) {
	 } 
    if (subsession->sink != NULL) {
	  if (singleMedium == NULL) {
	  } else {
	   }
	  subsession->sink->startPlaying(*(subsession->readSource()),
					 subsessionAfterPlaying,
					 subsession); madeProgress = True;
	}
      }
      }
  }
	 if (madeProgress) 
	 {
  // Finally, start playing each subsession, to start the data flow:
  if (duration == 0) {
    if (scale > 0) duration = session->playEndTime() - initialSeekTime; // use SDP end time
    else if (scale < 0) duration = initialSeekTime;
  }
  if (duration < 0) duration = 0.0;
  endTime = initialSeekTime;
  if (scale > 0) {
    if (duration <= 0) endTime = -1.0f;
    else endTime = initialSeekTime + duration;
  } else {
    endTime = initialSeekTime - duration;
    if (endTime < 0) endTime = 0.0f;
  }
  getPlay(session, initialSeekTime, endTime, scale, continueAfter);
}
}
}

void ClientInterface::continueAfter(RTSPClient* rtspClient, int resultCode, char* resultString) 
{
}
void ClientInterface::closeMediaSinks() {
	//Medium::close(fileSink);
  if (session == NULL) return;
  MediaSubsessionIterator iter(*session);
  MediaSubsession* subsession;
  while ((subsession = iter.next()) != NULL) {
    Medium::close(subsession->sink);
    subsession->sink = NULL;
  }
}
void ClientInterface::subsessionAfterPlaying(void* clientData) {
	
  // Begin by closing this media subsession's stream:
  MediaSubsession* tmp_subsession = (MediaSubsession*)clientData;
  Medium::close(tmp_subsession->sink);
  tmp_subsession->sink = NULL;
  // Next, check whether *all* subsessions' streams have now been closed:
  MediaSession& tmp_session = tmp_subsession->parentSession();
  MediaSubsessionIterator iter(tmp_session);
  while ((tmp_subsession = iter.next()) != NULL) {
    if (tmp_subsession->sink != NULL) return; // this subsession is still active
  }
  }
void ClientInterface::continueAfterTEARDOWN(RTSPClient*rtspClient, int resultCode, char* resultString) {
	if (NULL != rtspClient)
	{
		ClientData->setWatchVariable(~0);//ÖÕÖ¹doEventLoop
		ClientData->docontinueAfterTEARDOWN(rtspClient,resultCode,resultString);
	}
}
void ClientInterface::docontinueAfterTEARDOWN(RTSPClient*rtspClient, int resultCode, char* resultString) {
  closeMediaSinks();
  Medium::close(session);
  session = NULL;
 }
void ClientInterface::afterGetFrameData(unsigned char const *clientData, unsigned frameSize, struct timeval presentationTime,char *outFileName,void* mediaC)
{	
	ClientInterface* mediaClient = (ClientInterface*)mediaC;
	mediaClient->doafterGetFrameData(clientData, frameSize,presentationTime,outFileName);
}

void ClientInterface::doafterGetFrameData(unsigned char const *clientData, unsigned frameSize, struct timeval presentationTime,char *outFileName)
{
	if(CallBackForGetBuffer != NULL)
		CallBackForGetBuffer(this,clientData, frameSize, duration, presentationTime);
}

