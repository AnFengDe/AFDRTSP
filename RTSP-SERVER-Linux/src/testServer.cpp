//#include <windows.h>
//#pragma comment(lib, "wsock32.lib")
//#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
extern int runRTSPsever(unsigned short serverPortNum);


int main(int argc, char *argv[])
{
     runRTSPsever(554);
	
return 0;
}
