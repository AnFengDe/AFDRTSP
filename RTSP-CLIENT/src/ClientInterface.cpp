#include "ClientInterface.h"

ClientInterface* ClientData;


ClientInterface::ClientInterface(UsageEnvironment& env):Medium(env)
{
	OurRTSPClient = NULL;
	OurAuthenticator =NULL;
	CallBackForGetBuffer = NULL;
    setupIter = NULL; 
	fileSink = NULL;
	OverHTTPPortNum = 0;
     session = NULL;
	sessionTimerTask = NULL;
	arrivalTimerTask = NULL;
	PacketTimerTask = NULL;
	MeasurementTimerTask = NULL;
	createReceivers = True;
	singleMedium = NULL;
	OneFilePerFrame = False;
	streamUsingTCP = False;
	duration = 0.0f;
	SeekTime = 0.0f;
	scale = 1.0f;
	endTime = 0.0f;
	simpleRTP = -1;
	fileSinkBufferSize = 100000;
	areAlreadyShuttingDown = False;
	watchVariable = 0;
	madeProgress = False;


}
ClientInterface::~ClientInterface(){
	if (NULL != OurAuthenticator)
	{
		delete OurAuthenticator;
		OurAuthenticator = NULL;
	}
	Medium::close(OurRTSPClient);
	OurRTSPClient = NULL;
}
ClientInterface* ClientInterface
:: createNew() 
{	
	TaskScheduler* scheduler = BasicTaskScheduler::createNew();
	BasicUsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);
	return new ClientInterface(*env);
}

Medium* ClientInterface::CreateClient(UsageEnvironment& env, char const* url, int verbosityLevel, char const* applicationName) {
    ClientData=this;
	return OurRTSPClient = RTSPClient::createNew(env,url, verbosityLevel, applicationName, OverHTTPPortNum);
}

void ClientInterface::GetOptions(RTSPClient::responseHandler* afterFunc) { 
	OurRTSPClient->sendOptionsCommand(afterFunc, OurAuthenticator);
}

void ClientInterface::GetSdpDescription(RTSPClient::responseHandler* afterFunc) {
	OurRTSPClient->sendDescribeCommand(afterFunc, OurAuthenticator);
}

void ClientInterface::GetSetup(MediaSubsession* subsession, Boolean streamUsingTCP, RTSPClient::responseHandler* afterFunc) {
	Boolean forceMulticastOnUnspecified = False;
	OurRTSPClient->sendSetupCommand(*subsession, afterFunc, False, streamUsingTCP, forceMulticastOnUnspecified, OurAuthenticator);
}

void ClientInterface::GetPlay(MediaSession* session, double start, double end, float scale, RTSPClient::responseHandler* afterFunc) {
	OurRTSPClient->sendPlayCommand(*session, afterFunc, start, end, scale, OurAuthenticator);
}

void ClientInterface::GetPause(MediaSession* session, RTSPClient::responseHandler* responseHandler) {
	OurRTSPClient->sendPauseCommand(*session, responseHandler, OurAuthenticator);
}

void ClientInterface::GetTeardown(MediaSession* session, RTSPClient::responseHandler* afterFunc) {
	OurRTSPClient->sendTeardownCommand(*session, afterFunc, OurAuthenticator);
}
int ClientInterface::Start(char* RTSP_URL,GetBuffer* GetBuffer)
{
int verbosityLevel = 1;
areAlreadyShuttingDown = False;
CallBackForGetBuffer = GetBuffer;
CreateClient(envir(),RTSP_URL, verbosityLevel, /*progName*/"HyerVision");
if (OurRTSPClient == NULL) 
{}
else
{	
	if((m_PlayThrd = CreateThread( 	(LPSECURITY_ATTRIBUTES)NULL, 0,	(LPTHREAD_START_ROUTINE)PlayThrd,(void *)this,	0, NULL)) == NULL)
		return -1;
	}
return 0;
}
void ClientInterface::PlayThrd(LPVOID lParam)
{
	ClientInterface* testClient = (ClientInterface*)lParam;
	testClient->GetOptions(AfterOPTIONS);
	testClient->SetWatchVariable(0);
	testClient->envir().taskScheduler().doEventLoop(&(testClient->watchVariable )); 
}
int ClientInterface::Pause()
{	GetPause(session, After);
	return 0;
}

int ClientInterface::Resume(double percent)
{	double NTP_time;
	if (duration == 0.0f)
		NTP_time = percent;
	else
		NTP_time = duration * percent;
	GetPlay(session, NTP_time, endTime, scale, After);
	return 0;
}

int ClientInterface::Fast(double resacle)
{	GetPlay(session, -0.1, endTime, resacle, After);
	return 0;
}
int ClientInterface::Slow(double resacle)
{	GetPlay(session, -0.1, endTime, resacle, After);
	return 0;
}

int ClientInterface::Stop()
{	
#if 1
	if (&envir() != NULL) {
		envir().taskScheduler().unscheduleDelayedTask(sessionTimerTask);
		envir().taskScheduler().unscheduleDelayedTask(arrivalTimerTask);
		envir().taskScheduler().unscheduleDelayedTask(PacketTimerTask);
		envir().taskScheduler().unscheduleDelayedTask(MeasurementTimerTask);
	}
#endif
	if (session != NULL) {
		GetTeardown(session, AfterTEARDOWN);

	} else {
		AfterTEARDOWN(OurRTSPClient, 0, NULL);
	}

	return 0;

}
void ClientInterface::AfterOPTIONS(RTSPClient* rtspClient, int resultCode, char* resultString) {
	if (NULL != rtspClient)
	{
        if (NULL != resultString)
	{
		delete[] resultString;
	}
	if (NULL != rtspClient)
	{		
        ClientData->GetSdpDescription(AfterDESCRIBE);
	}	
	}
}
void ClientInterface::AfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString) 
{
	if (NULL != rtspClient)
	{
		ClientData->DoAfterDESCRIBE(rtspClient, resultCode, resultString);
	}
}
void ClientInterface::DoAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString){
	if (resultCode == 0)
{
  char* sdpDescription = resultString;
  session = MediaSession::createNew(envir(), sdpDescription);
  delete[] sdpDescription;
  	 if (session != NULL&&session->hasSubsessions())
	{
  MediaSubsessionIterator iter(*session);
  MediaSubsession *subsession;
  Boolean madeProgress = False;
  char const* singleMediumToTest = singleMedium;
  while ((subsession = iter.next()) != NULL) {
    if (singleMediumToTest != NULL) {
      if (strcmp(subsession->mediumName(), singleMediumToTest) != 0) {
			continue;
      } else {
	singleMediumToTest = "xxxxx";
      }
    }
    if (createReceivers) {
      if (!subsession->initiate(simpleRTP)) {
	
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
  SetupStreams();
  }
     }
}
}

void ClientInterface::AfterSETUP(RTSPClient* rtspClient, int resultCode, char* resultString) 
{
	if (NULL != rtspClient)
	{
        if (resultCode == 0) {
   ClientData->madeProgress = True;
  } else {
  }
 ClientData->SetupStreams(); //
	}
}

void ClientInterface::SetupStreams() {
  if (setupIter == NULL) 
	  setupIter = new MediaSubsessionIterator(*session);
	MediaSubsession *subsession;
  while ((subsession = setupIter->next()) != NULL) {
    if (subsession->clientPortNum() == 0) continue; 
    GetSetup(subsession, streamUsingTCP, AfterSETUP);
    return;
  }
  delete setupIter;
  setupIter = NULL;
  if (madeProgress) 
	{
  if (createReceivers) {
	if (True) {
    madeProgress = False;
      MediaSubsessionIterator iter(*session);
      while ((subsession = iter.next()) != NULL) {
	if (subsession->readSource() == NULL) continue; 
	char outFileName[1000]="test.264";
	  fileSink = FileSink::createNew(envir(), outFileName,
					 fileSinkBufferSize, OneFilePerFrame,this);
	  fileSink->afterGetingFrameData(AfterGetFrameData,outFileName);
	subsession->sink = fileSink;
	if (subsession->sink == NULL) {
	 } 
    if (subsession->sink != NULL) {
	  if (singleMedium == NULL) {
	  } else {
	   }
	  subsession->sink->startPlaying(*(subsession->readSource()),
					 SubsessionAfterPlaying,
					 subsession); madeProgress = True;
	}
      }
      }
  }
	 if (madeProgress) 
	 {
  if (duration == 0) {
    if (scale > 0) duration = session->playEndTime() - SeekTime; // use SDP end time
    else if (scale < 0) duration = SeekTime;
  }
  if (duration < 0) duration = 0.0;
  endTime = SeekTime;
  if (scale > 0) {
    if (duration <= 0) endTime = -1.0f;
    else endTime = SeekTime + duration;
  } else {
    endTime = SeekTime - duration;
    if (endTime < 0) endTime = 0.0f;
  }
  GetPlay(session, SeekTime, endTime, scale, After);
}
}
}

void ClientInterface::After(RTSPClient* rtspClient, int resultCode, char* resultString) 
{
}
void ClientInterface::CloseMediaSinks() {
  if (session == NULL) return;
  MediaSubsessionIterator iter(*session);
  MediaSubsession* subsession;
  while ((subsession = iter.next()) != NULL) {
    Medium::close(subsession->sink);
    subsession->sink = NULL;
  }
}
void ClientInterface::SubsessionAfterPlaying(void* clientData) {
	
  MediaSubsession* tmp_subsession = (MediaSubsession*)clientData;
  Medium::close(tmp_subsession->sink);
  tmp_subsession->sink = NULL;
  MediaSession& tmp_session = tmp_subsession->parentSession();
  MediaSubsessionIterator iter(tmp_session);
  while ((tmp_subsession = iter.next()) != NULL) {
    if (tmp_subsession->sink != NULL) return; 
  }
  }
void ClientInterface::AfterTEARDOWN(RTSPClient*rtspClient, int resultCode, char* resultString) {
	if (NULL != rtspClient)
	{
		ClientData->SetWatchVariable(~0);
		ClientData->DoAfterTEARDOWN(rtspClient,resultCode,resultString);
	}
}
void ClientInterface::DoAfterTEARDOWN(RTSPClient*rtspClient, int resultCode, char* resultString) {
  CloseMediaSinks();
  Medium::close(session);
  session = NULL;
 }
void ClientInterface::AfterGetFrameData(unsigned char const *clientData, unsigned frameSize, struct timeval presentationTime,char *outFileName,void* mediaC)
{	
	
}
