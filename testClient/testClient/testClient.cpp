#include "stdafx.h"
#include <windows.h>
#include "ClientCommand.h"
#pragma   comment(lib,   "Wsock32.lib ")
void* a;
void add_Data(void* clientId,unsigned char const *clientData, unsigned frameSize, double Duration,struct timeval presentationTime)
{
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
			Play(a, "rtsp://127.0.0.1/3.264",add_Data);
			break;
		case 'b':
			Stop(a);
			break;
		
		case 'c':
			Pause(a);
			break;
		
		case 'd':
			RePlay(a,0.8);
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

