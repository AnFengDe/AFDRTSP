#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"
#include <NetAddress.hh>
typedef void (GetBuffer)(void* clientId, unsigned char const *clientData, unsigned frameSize, double Duration,struct timeval presentationTime );

typedef void (GetResult)( void* clientId,int resultCode, char* resultString);

typedef void (GetRtcp)(void* clientId,char *clientData);

typedef void (GetSdp)(void* clientId,char *clientData);

//class RTSPClient;
//class FileSink;

class ClientInterface: public Medium{


public:
	ClientInterface(UsageEnvironment& env);

	~ClientInterface();
	static ClientInterface*createNew();

	virtual	int Start(char* RTSP_URL,GetBuffer* GetBuffer);

	virtual	int Pause();

	virtual int RePlay(double percent);

	virtual int Fast(double resacle);

	virtual int Slow(double resacle);

	virtual int Stop();

private:

	Medium* CreateClient(UsageEnvironment& env, char const* url, int verbosityLevel, char const* applicationName);

	void GetOptions(RTSPClient::responseHandler* afterFunc);

	void GetSdpDescription(RTSPClient::responseHandler* afterFunc) ;

	void GetSetup(MediaSubsession* subsession, Boolean StreamUsingTCP, RTSPClient::responseHandler* afterFunc) ;	

	void GetPlay(MediaSession* session, double start, double end, float Scale, RTSPClient::responseHandler* afterFunc) ;
	
	void GetPause(MediaSession* session, RTSPClient::responseHandler* responseHandler);	

	void GetTeardown(MediaSession* session, RTSPClient::responseHandler* afterFunc);

	static void AfterOPTIONS(RTSPClient*, int resultCode, char* resultString) ;
	static void AfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
	void DoAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
	static void AfterSETUP(RTSPClient*, int resultCode, char* resultString) ;
	void SetupStreams() ;
	static void After(RTSPClient*, int resultCode, char* resultString);
	void CloseMediaSinks();
	static void SubsessionAfterPlaying(void* clientData);
	void DoSubsessionAfterPlaying();
    static void AfterTEARDOWN(RTSPClient*, int resultCode, char* resultString) ;
	void DoAfterTEARDOWN(RTSPClient*, int resultCode, char* resultString) ;
	static void AfterGetFrameData(unsigned char const *clientData, unsigned frameSize, struct timeval presentationTime,char *outFileName,void *);
	void SetSessionTimerTask(TaskToken token){SessionTimerTask = token;};
	void SetWatchVariable(char c){WatchVariable = c;};
	static void PlayThrd(LPVOID lParam);


private:
		HANDLE m_PlayThrd;
	RTSPClient* OurRTSPClient;
	Authenticator* OurAuthenticator;
	GetBuffer* CallBackForGetBuffer;
	MediaSubsessionIterator* setupIter; 
	FileSink* fileSink;
	u_int16_t OverHTTPPortNum;
	MediaSession* session ;
	TaskToken SessionTimerTask;
	TaskToken ArrivalTimerTask;
	TaskToken PacketTimerTask;
	TaskToken MeasurementTimerTask;
	Boolean CreateReceivers ;
	char const* SingleMedium;
	Boolean OneFilePerFrame ;
	Boolean StreamUsingTCP;
	double Duration;
	double SeekTime;
	float Scale;
	double EndTime;
	int SimpleRTP;
	unsigned fileSinkBufferSize;
	//struct timeval startTime;
	//bool areAlreadyShuttingDown;
	char WatchVariable;
	//int resultCode;
	Boolean m_Progress;
};
