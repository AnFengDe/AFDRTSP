// test1.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include "ClientCommand.h"
#pragma   comment(lib,   "Wsock32.lib ")


//typedef void (GetBuffer)(void* clientId, unsigned char const *clientData, unsigned frameSize, double duration,struct timeval presentationTime );

//extern void*  CreateClient();
//extern int  Play(void* RtspClient, char* RTSP_URL,GetBuffer* CallBackForGetBuffer);
void* a;
void* b;
void printsdp( void* clientId,char* resultString) 
{
	printf("SDP:\n%s\n",resultString);
}
void AfterStop( void* clientId,int resultCode, char* resultString) 
{
	printf("RESULT:%d,%s\n",resultCode,resultString);
	if(0 != resultCode)
		Stop(clientId);

}

void add_Data(void* clientId,unsigned char const *clientData, unsigned frameSize, double duration,struct timeval presentationTime)
{

	printf("Get DATA　SIZE is %d\n",frameSize);

}

void doAfterGetRtcp(void* clientId,char* status)
{
	printf("%s\n",status);
}

int _tmain(int argc, _TCHAR* argv[])
{
	
	while(1)
	{
		char order = getc(stdin);
		switch(order)
		{   
		case 'a':

			a = CreateClient();
			//RegisterCallBackForGetResult(a,AfterStop);
			//RegisterCallBackForGetRtcpStatus(a, doAfterGetRtcp);
			//RegisterCallBackForGetSdp(a,printsdp);
			Play(a, "rtsp://127.0.0.1/1.mp3",add_Data);
			break;
		case 'b':
			Stop(a);
			break;
		
		case 'c':
			Pause(a);
			break;
		
		case 'd':
			Resume(a,0.8);
			break;
		
		case 'e':
			Fast(a,1.5);
			break;
		
		case 'f':
			Slow(a,0.5);
			break;
		
		case 'g':
			goto end;
			break;
		default:
			break;
		}

	}

end:

	return 0;
}

