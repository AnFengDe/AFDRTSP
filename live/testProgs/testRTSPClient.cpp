#include <stdio.h>
#include "AFDRTSPClient.h"

int main()
{
    init();
#if 0
    if (false == init())
    {
        printf("AFDRTSPClient init failure\n");
        return -1;
    }
    //printf("Hello, I am testRTSPClient\n");
    const void* handle = create_new("rtsp://localhost/test.mp3", 1);

    printf("create handle is 0x%x\n", handle);

    if (false == cleanup())
    {
        printf("AFDRTSPClient cleanup failure\n");
        return -1;
    }
#endif
    return 0;
}

