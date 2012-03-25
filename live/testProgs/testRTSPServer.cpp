#include <stdio.h>
#include <string.h>
#include "AFDRTSPServer.h"

#ifdef WIN32
#define STD_CALLBACK    __stdcall
#else
#define STD_CALLBACK    
#endif

void STD_CALLBACK handle_Options(char* cmd_names)
{
    sprintf(cmd_names, "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER");
}

int main()
{
    char cmd[256];
    st_Handle_Cmd_Callback cb;
    
    ::memset(&cb, 0x00, sizeof(cb));
    cb.options = handle_Options;
            
    if (false == server_init())
    {
        printf("AFD RTSP Server init failure\n");
        return -1;
    }
    if (false == run_rtsp_srv(&cb, 8554)) 
    {
        printf("AFD RTSP Server init failure\n");
        return -1;
    }

    while(1) //input loop
    {
        ::memset(cmd, 0x00, 256);
        printf(	"Please input command : \n -exit exit program\n  Please input:");
        
        gets(cmd);
        
        if (0 == ::memcmp(cmd, "exit", 4))
        {
            if (false == server_cleanup())
            {
                printf("AFD RTSP Server cleanup failure\n");
                return -1;
            }
            break;
        }
        else
        {
            printf("input command error!\n");
        }
    }
    
    return 0;
}

