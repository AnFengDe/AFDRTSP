#include "ClientCommand.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include<stdio.h>
#include <string.h>
#include <pthread.h>
void* a;
void add_Data(void* clientId,unsigned char const *clientData, unsigned frameSize, double Duration,struct timeval presentationTime)
{
}
int main(int argc, char *argv[])
{
  
	
	while(1)
	{
		char order=getchar();// 
       		switch(order)
		{   
		case 'a':

			a = CreateClient();
			Play(a, "rtsp://127.0.0.1:8554/3.264",add_Data);
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

