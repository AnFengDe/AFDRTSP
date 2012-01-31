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
	Scale = 1.0f;
	EndTime = 0.0f;
	simpleRTP = -1;
	fileSinkBufferSize = 100000;
	areAlreadyShuttingDown = False;
	watchVariable = 0;
	m_Progress = False;


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

void ClientInterface::GetPlay(MediaSession* session, double start, double end, float Scale, RTSPClient::responseHandler* afterFunc) {
	OurRTSPClient->sendPlayCommand(*session, afterFunc, start, end, Scale, OurAuthenticator);
}

void ClientInterface::GetPause(MediaSession* session, RTSPClient::responseHandler* responseHandler) {
	OurRTSPClient->sendPauseCommand(*session, responseHandler, OurAuthenticator);
}

void ClientInterface::GetTeardown(MediaSession* session, RTSPClient::responseHandler* afterFunc) {
	OurRTSPClient->sendTeardownCommand(*session, afterFunc, OurAuthenticator);
}
int ClientInterface::Start(char* url,GetBuffer* GetBuffer)
{
areAlreadyShuttingDown = False;
CallBackForGetBuffer = GetBuffer;
CreateClient(envir(),url, 1,"NewClient");
if (OurRTSPClient != NULL) 
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

int ClientInterface::RePlay(double percent)
{	double NTP_time;
	if (duration == 0.0f)
		NTP_time = percent;
	else
		NTP_time = duration * percent;
	GetPlay(session, NTP_time, EndTime, Scale, After);
	return 0;
}

int ClientInterface::Fast(double resacle)
{	GetPlay(session, -0.1, EndTime, resacle, After);
	return 0;
}
int ClientInterface::Slow(double resacle)
{	GetPlay(session, -0.1, EndTime, resacle, After);
	return 0;
}

int ClientInterface::Stop()
{	
	if (&envir() != NULL) {
		envir().taskScheduler().unscheduleDelayedTask(sessionTimerTask);
		envir().taskScheduler().unscheduleDelayedTask(arrivalTimerTask);
		envir().taskScheduler().unscheduleDelayedTask(PacketTimerTask);
		envir().taskScheduler().unscheduleDelayedTask(MeasurementTimerTask);
	}
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
         ClientData->GetSdpDescription(AfterDESCRIBE);
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
  Boolean m_Progress = False;
  char const* singleMediumToTest = singleMedium;
  while ((subsession = iter.next()) != NULL) {
    if (createReceivers) {
      if (subsession->initiate(simpleRTP)) {
		
	m_Progress = True;	
      }
    } 
    else {
      if (subsession->clientPortNum() == 0) {      } else {
		m_Progress = True;
      }
    }
  }
  
  if (m_Progress) {	
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
   ClientData->m_Progress = True;
  } else {
  }
 ClientData->SetupStreams(); 
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
  if (m_Progress) 
	{
  if (createReceivers) {
	if (True) {
    m_Progress = False;
      MediaSubsessionIterator iter(*session);
      while ((subsession = iter.next()) != NULL) {
	if (subsession->readSource() == NULL) continue; 
	char outFileName[1000]="test.264";
	  fileSink = FileSink::createNew(envir(), outFileName,
					 fileSinkBufferSize, OneFilePerFrame,this);
	  fileSink->afterGetingFrameData(AfterGetFrameData,outFileName);
	subsession->sink = fileSink;
	    if (subsession->sink != NULL) {
		  subsession->sink->startPlaying(*(subsession->readSource()),
					 SubsessionAfterPlaying,
					 subsession); m_Progress = True;
	}
      }
      }
  }
	 if (m_Progress) 
	 {
  if (duration == 0) {
    if (Scale > 0) duration = session->playEndTime() - SeekTime; // use SDP end time
    else if (Scale < 0) duration = SeekTime;
  }
  if (duration < 0) duration = 0.0;
  EndTime = SeekTime;
  if (Scale > 0) {
    if (duration <= 0) EndTime = -1.0f;
    else EndTime = SeekTime + duration;
  } else {
    EndTime = SeekTime - duration;
    if (EndTime < 0) EndTime = 0.0f;
  }
  GetPlay(session, SeekTime, EndTime, Scale, After);
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
