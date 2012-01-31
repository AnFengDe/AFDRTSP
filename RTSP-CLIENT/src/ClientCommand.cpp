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

int RePlay(void* RtspClient,double percent)
{
	ClientInterface* myclient = (ClientInterface*)RtspClient;
	return myclient->RePlay(percent);
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
