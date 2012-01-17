#include "ClientInterface.h"
#include "ClientCommand.h"

void* CreateClient(){
ClientInterface* RtspClient = NULL;
RtspClient = ClientInterface::createNew();
return RtspClient;
}

int Play(void* RtspClient, char* RTSP_URL,doGetBuffer* CallBackForGetBuffer)
{
	ClientInterface* myclient = (ClientInterface*)RtspClient;
	return myclient->Start(RTSP_URL,CallBackForGetBuffer);
}

int Pause( void* RtspClient )
{
	ClientInterface* myclient = (ClientInterface*)RtspClient;
	return myclient->Pause();
}

int Resume(void* RtspClient,double percent)
{
	ClientInterface* myclient = (ClientInterface*)RtspClient;
	return myclient->Resume(percent);
}
int Fast(void* RtspClient,double resacle)
{
	ClientInterface* myclient = (ClientInterface*)RtspClient;
	return myclient->Fast(resacle);
}
int Slow(void* RtspClient,double resacle)
{
	ClientInterface* myclient = (ClientInterface*)RtspClient;
	return myclient->Slow(resacle);
}
int Stop(void* RtspClient )
{
	ClientInterface* myclient = (ClientInterface*)RtspClient;
	return myclient->Stop();
}

//int RegisterCallBackForGetRtcpStatus(void* RtspClient,doGetRtcp* doGetRtcp)
//{
//	ClientInterface* myclient = (ClientInterface*)RtspClient;
//	return myclient->RegisterCallBackForGetRtcpStatus(doGetRtcp);
//	
//}
//int RegisterCallBackForGetResult(void* RtspClient,doGetResult* doGetResult)
//{
//	ClientInterface* myclient = (ClientInterface*)RtspClient;
//	return myclient->RegisterCallBackForGetResult(doGetResult);
//
//}
//int RegisterCallBackForGetSdp(void* RtspClient,doGetSdp* doGetSdp)
//{
//	ClientInterface* myclient = (ClientInterface*)RtspClient;
//	return myclient->RegisterCallBackForGetSdp(doGetSdp);
//}