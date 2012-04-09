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

void STD_CALLBACK handle_Describe(int *ret, char* url, char* sdp_desc)
{
    *ret = 1;
   char sdp[531]="v=0\r\no=- 1329294539622088 1 IN IP4 127.0.0.1\r\ns=H.264 Video, streamed by the LIVE555 Media Server\r\ni=3.264\r\nt=0 0\r\na=tool:LIVE555 Streaming Media v2012.02.03\r\na=type:broadcast\r\na=control:*\r\na=range:npt=0-200\r\na=x-qt-text-nam:H.264 Video, streamed by the LIVE555 Media Server\r\na=x-qt-text-inf:3.264\r\nm=video 0 RTP/AVP 96\r\nc=IN IP4 0.0.0.0\r\nb=AS:500\r\na=rtpmap:96 H264/90000\r\na=fmtp:96 packetization-mode=1;profile-level-id=4D001F;sprop-parameter-sets=J00AH9oBQBbsBVIAAAMAAgAAAwBkwIAAIAAAAwAQAA3vfC8IhGo=,KO48gA==\r\na=control:track1\r\n"; 
strcpy(sdp_desc,sdp);
 //sdp_desc=sdp;
  //unsigned sdpDescriptionSize = strlen(sdp_desc); 
// sdp_desc[sdpDescriptionSize+1]='\0';
   /// sdp_desc=desc;
}
int main()
{
    char cmd[256];
    st_Handle_Cmd_Callback cb;
    
    ::memset(&cb, 0x00, sizeof(cb));
    cb.options = handle_Options;
    cb.describe = handle_Describe;
            
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

