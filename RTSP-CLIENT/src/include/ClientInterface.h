#ifndef _MEDIA_CLIENT_HH
#define _MEDIA_CLIENT_HH
#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"
//#include <Windows.h>
#include <NetAddress.hh>

	/**********************************
	功能：取出视频数据的回调函数
	参数：
	clientData:数据缓冲区地址
	frameSize:数据长度
	duration:媒体时间长度
	presentationTime：RTP时间戳
	**********************************/

	typedef void (doGetBuffer)(void* clientId, unsigned char const *clientData, unsigned frameSize, double duration,struct timeval presentationTime );

	/**********************************
	功能：处理错误信息的回调函数
	参数：
	resultCode:错误代码 0，正确；非0，错误
	resultString:错误原因
	**********************************/

	typedef void (doGetResult)( void* clientId,int resultCode, char* resultString);

	/**********************************
	功能：取RTCP状态的回调函数
	参数：
	clientData：字符串SR，RR，BYE
	**********************************/

	typedef void (doGetRtcp)(void* clientId,char *clientData);

	/**********************************
	功能：取SDP的回调函数
	参数：
	clientData：SDP数据
	**********************************/

	typedef void (doGetSdp)(void* clientId,char *clientData);



class RTSPClient;
class FileSink;

class ClientInterface: public Medium{


public:
	ClientInterface(UsageEnvironment& env);

	~ClientInterface();
	static ClientInterface*createNew();
#if 0
	static void RtcpStatus(char *status);
		void doRtcpStatus(char *status);
#endif

	//int RegisterCallBackForGetRtcpStatus(doGetRtcp* doAfterGetRtcp);

	//int RegisterCallBackForGetResult(doGetResult* doGetResult);

	//int RegisterCallBackForGetSdp(doGetSdp* doGetSdp);

	virtual	int Start(char* RTSP_URL,doGetBuffer* doGetBuffer);

	virtual	int Pause();

	virtual int Resume(double percent);

	virtual int Fast(double resacle);

	virtual int Slow(double resacle);

	virtual int Stop();

private:

	Medium* createClient(UsageEnvironment& env, char const* url, int verbosityLevel, char const* applicationName);

	void getOptions(RTSPClient::responseHandler* afterFunc);

	void getSDPDescription(RTSPClient::responseHandler* afterFunc) ;

	void getSetup(MediaSubsession* subsession, Boolean streamUsingTCP, RTSPClient::responseHandler* afterFunc) ;	

	void getPlay(MediaSession* session, double start, double end, float scale, RTSPClient::responseHandler* afterFunc) ;
	

	void getPause(MediaSession* session, RTSPClient::responseHandler* responseHandler);	

	void getTeardown(MediaSession* session, RTSPClient::responseHandler* afterFunc);

#if 0
	int Init();
#endif



#if 0
	int Release();
#endif

	static void continueAfterOPTIONS(RTSPClient*, int resultCode, char* resultString) ;
		//void doAfterOPTIONS(RTSPClient*, int resultCode, char* resultString) ;


	static void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
		void doAfterDESCRIBE(RTSPClient* rtspClient, int resultCode, char* resultString);
	
	static void continueAfterSETUP(RTSPClient*, int resultCode, char* resultString) ;
		//void doAfterSETUP(RTSPClient*, int resultCode, char* resultString) ;

	//static void continueAfter(RTSPClient*, int resultCode, char* resultString);
		 //void ClientInterface::docontinueAfter(RTSPClient*  rtspClient, int resultCode, char* resultString); 



	void setupStreams() ;

	static void continueAfter(RTSPClient*, int resultCode, char* resultString);
	//	void ClientInterface::doAfterPLAY(RTSPClient*, int resultCode, char* resultString); 

	void closeMediaSinks();

	//void shutdown();



	//static void subsessionByeHandler(void* clientData);
	
	//static void subsessionSRHandler(void* clientData);
		//void dosubsessionSRHandler();

		//static void subsessionRRHandler(void* clientData);
//		void dosubsessionRRHandler();	

	//	static void subsessionBYEBYEHandler(void* clientData);
		//void dosubsessionBYEBYEHandler();	

	static void subsessionAfterPlaying(void* clientData);
		void dosubsessionAfterPlaying();


	//static void sessionAfterPlaying(void* clientData) ;

		//void dosessionAfterPlaying();

	//static void sessionTimerHandler(void* clientData) ;

	static void continueAfterTEARDOWN(RTSPClient*, int resultCode, char* resultString) ;
		
		void docontinueAfterTEARDOWN(RTSPClient*, int resultCode, char* resultString) ;
	
	//void signalHandlerShutdown(int /*sig*/);

	//static void checkForPacketArrival(void* clientData) ;

	//static void checkInterPacketGaps(void* clientData) ;

	static void afterGetFrameData(unsigned char const *clientData, unsigned frameSize, struct timeval presentationTime,char *outFileName,void *);
		void doafterGetFrameData(unsigned char const *clientData, unsigned frameSize, struct timeval presentationTime,char *outFileName);
	void setSessionTimerTask(TaskToken token){sessionTimerTask = token;};
	void setWatchVariable(char c){watchVariable = c;};
//	void setResultCode(int code){resultCode = code;};

	static void PlayThrd(LPVOID lParam);


private:
		HANDLE m_PlayThrd;
	RTSPClient* ourRTSPClient;
	//Boolean allowProxyServers ;
	//Boolean controlConnectionUsesTCP ;
	//Boolean supportCodecSelection ;
	//char const* clientProtocolName ;
	Authenticator* ourAuthenticator;

	//doGetRtcp* CallBackForGetRtcpStatus;
	//doGetResult* CallBackForGetResult;
	doGetBuffer* CallBackForGetBuffer;
	//doGetSdp* CallBackForGetSdp;



	MediaSubsessionIterator* setupIter; //fengyu modifyed
	//char const* progName;
	FileSink* fileSink;//fengyu modifyed
	//portNumBits tunnelOverHTTPPortNum;
	u_int16_t tunnelOverHTTPPortNum;

#if 0
	BasicUsageEnvironment* env;
	TaskScheduler* scheduler;
#endif
#if 0
	Mutex_t h_mutex;
#endif
	MediaSession* session ;
	TaskToken sessionTimerTask;
	TaskToken arrivalCheckTimerTask;
	TaskToken interPacketGapCheckTimerTask;
	TaskToken qosMeasurementTimerTask;
	Boolean createReceivers ;
	//Boolean notifyOnPacketArrival;
	///Boolean outputAVIFile;
	//unsigned interPacketGapMaxTime;
	//unsigned totNumPacketsReceived;// used if checking inter-packet gaps
	//Boolean playContinuously;
	char const* singleMedium;
	//char const* streamURL;
	Boolean oneFilePerFrame ;
	Boolean streamUsingTCP;
	//Boolean syncStreams;
	unsigned short desiredPortNum;
	double duration;
	double durationSlop; // extra seconds to play at the end
	double initialSeekTime;
	float scale;
	double endTime;
	int simpleRTPoffsetArg;
	unsigned fileSinkBufferSize;
	unsigned socketInputBufferSize;
	/*fengyu moved here*/
	struct timeval startTime;
	bool areAlreadyShuttingDown;
	int shutdownExitCode;

	char watchVariable;
	int resultCode;

#if 0
	MediaSubsession *subsession;
#endif
	Boolean madeProgress;
};





#endif