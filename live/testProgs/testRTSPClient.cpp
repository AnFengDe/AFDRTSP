#include <stdio.h>
#include <string.h>
#include "AFDRTSPClient.h"

int main()
{
    char cmd[256];
    const void* handle = NULL;

    while(1) //input loop
    {
        ::memset(cmd, 0x00, 256);
        printf(	"Please input command : \n -init init rtsp client lib\n"
        				" -create create rtsp client session\n"
                        " -play send play command\n"
                        " -pause send pause command\n"
                        " -resume back to play \n"
                        " -fast play fast\n"
                        " -slow play slow\n"
        				" -exit exit program\n  Please input:");
        
        gets(cmd);
        
        if (0 == ::memcmp(cmd, "init", 4))
        {
            if (false == client_init())
            {
                printf("AFDRTSPClient init failure\n");
                return -1;
            }
            else
            {
                printf("init AnFengDe RTSP Client success\n");
            }
        }
        else if (0 == ::memcmp(cmd, "create", 6)) 
        {
            char* purl = cmd + 7;
            if ( 0 == ::memcmp(purl, "rtsp", 4))
            {
                handle = create_new(purl, 1);
                printf("create handle is 0x%x\n", handle);
            }
            else
            {
                printf("Input error! You must input like: create rtsp://192.168.1.1/1.mp3\n");
            }
        }
        else if (0 == ::memcmp(cmd, "play", 4))
        {
            client_init();
            handle = create_new("rtsp://127.0.0.1:8554/1.1", 1);
            if ( NULL == handle) 
            {
                printf("Input order error!\n You must input create command first.\n");
            }
            else
            {
                unsigned ret = play(handle);
                printf("the play command return value is %d\n", ret);
            }
        }
        else if (0 == ::memcmp(cmd, "pause", 5))
        {
            unsigned ret = pause(handle);
            printf("the pause command return value is %d\n", ret);
        }
        else if (0 == ::memcmp(cmd, "resume", 6))
        {
            unsigned ret = resume(handle, 45.0);
            printf("the resume command return value is %d\n", ret);
        }
        else if (0 == ::memcmp(cmd, "fast", 4))
        {
            unsigned ret = fast(handle, 2.0);
            printf("the fast command return value is %d\n", ret);
        }
        else if (0 == ::memcmp(cmd, "slow", 4))
        {
            unsigned ret = slow(handle, 0.125);
            printf("the slow command return value is %d\n", ret);
        }
        else if (0 == ::memcmp(cmd, "exit", 4))
        {
            if (false == client_cleanup())
            {
                printf("AFDRTSPClient cleanup failure\n");
                return -1;
            }
        }
        else
        {
            printf("input command error!\n");
        }
    }
    
    return 0;
}

