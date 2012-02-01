#include "stdafx.h"
#include <windows.h>
#pragma comment(lib, "wsock32.lib")
extern int runRTSPsever(unsigned short serverPortNum);


int _tmain(int argc, _TCHAR* argv[])
{
     runRTSPsever(554);
	
}

