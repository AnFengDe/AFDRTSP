#include <stdio.h>
#include "AFDRTSPClient.h"

int main()
{
    //printf("Hello, I am testRTSPClient\n");
    const void* handle = create_new("rtsp://localhost/test.mp3", 1);

    printf("create handle is 0x%x\n", handle);

    return 0;
}

