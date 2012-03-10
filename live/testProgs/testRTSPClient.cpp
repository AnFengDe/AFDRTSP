#include <stdio.h>
#include "AFDRTSPClient.h"

int main()
{
    if (false == init())
    {
        printf("AFDRTSPClient init failure\n");
        return -1;
    }else
    {
        printf("init AnFengDe RTSP Client success\n");
    }
    
    const void* handle = create_new("rtsp://localhost/test.mp3", 1);

    printf("create handle is 0x%x\n", handle);

    if (false == cleanup())
    {
        printf("AFDRTSPClient cleanup failure\n");
        return -1;
    }
    return 0;
}

