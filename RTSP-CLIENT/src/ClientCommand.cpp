#include "ClientInterface.h"
#include "ClientCommand.h"

void* CreateClient(){
ClientInterface* RtspClient = NULL;
RtspClient = ClientInterface::createNew();
return RtspClient;
}

int Play(void* RtspClient, char* RTSP_URL,GetBuffer* CallBackForGetBuffer)
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

//int RegisterCallBackForGetRtcpStatus(void* RtspClient,GetRtcp* GetRtcp)
//{
//	ClientInterface* myclient = (ClientInterface*)RtspClient;
//	return myclient->RegisterCallBackForGetRtcpStatus(GetRtcp);
//	
//}
//int RegisterCallBackForGetResult(void* RtspClient,GetResult* GetResult)
//{
//	ClientInterface* myclient = (ClientInterface*)RtspClient;
//	return myclient->RegisterCallBackForGetResult(GetResult);
//
//}
//int RegisterCallBackForGetSdp(void* RtspClient,GetSdp* GetSdp)
//{
//	ClientInterface* myclient = (ClientInterface*)RtspClient;
//	return myclient->RegisterCallBackForGetSdp(GetSdp);
//}