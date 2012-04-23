#include <stdio.h>
#include <string.h>
#include "AFDRTSPServer.h"

#ifdef WIN32
#define STD_CALLBACK    __stdcall
#else
#define STD_CALLBACK    
#endif

/*! \brief process RTSP OPTIONS command function 
    \param cmd_names the cmd_names is all or subset of 
           "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER".
            normally, the handler of command not be neccessary, keep it NULL 
*/
void STD_CALLBACK handle_Options(char* cmd_names)
{
    sprintf(cmd_names, "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER");
}

/*! \brief process RTSP DESCRIBE command function 
    \param ret the DESCRIBE command process return value, if less equal zero means failure.
           the reason maybe :
           1. the media file in url is not found
           2. can't surport sevice by customize check
    \param url the media file url, you can process by ruler yourself 
    \param sdp_desc sdp description, your must complete yourself
    \param duration the media file total time length
*/
void STD_CALLBACK handle_Describe(int *ret, const char* url, char* sdp_desc, float *duration)
{
    //check url is exist
    *ret = 1;
    //sdp line : You must set sps line yourself, the H.264 sample below
    char sdp[531]=  "m=video 0 RTP/AVP 96\r\n"
                    "c=IN IP4 0.0.0.0\r\n"
                    "b=AS:500\r\n"
                    "a=rtpmap:96 H264/90000\r\n"
                    "a=fmtp:96 packetization-mode=1;profile-level-id=4D001F;"
                    "sprop-parameter-sets=J00AH9oBQBbsBVIAAAMAAgAAAwBkwIAAIAAAAwAQAA3vfC8IhGo=,KO48gA==\r\n"
                    "a=control:track1\r\n"; 
    strcpy(sdp_desc, sdp);
}

/*! \brief process RTSP PAUSE command function 
    \param sessionid the server rtsp session id, the unique number for session
*/
void STD_CALLBACK handle_Pause(const unsigned sessionid)
{
    //put pause process code here
}

/*! \brief process RTSP TEARDOWN command function 
    \param sessionid the server rtsp session id, the unique number for session
*/
void STD_CALLBACK handle_Teardown(const unsigned sessionid)
{
    //put teardown code here
}

/*! \brief process RTSP PLAY command function 
    \param sessionid the server rtsp session id, the unique number for session
    \param scale the play speed ratio 
    \param start meida play start npt
    \param end meida play end npt
*/
void STD_CALLBACK handle_Play(const unsigned sessionid, 
                              const float scale, 
                              const double start, 
                              const double end)
{
}

/*! \brief process RTSP SETUP command function 
    \param sessionid the server rtsp session id, the unique number for session
    \param url the media file url 
    \param rtp_client_port the rtp client port, this value for rtp server,
           and as defalut, the rtcp port is follow this
    \param rtp_server_port the rtcp server port, this value must be set in callback function
*/
void STD_CALLBACK handle_Setup(const unsigned sessionid, 
                               const char* url, 
                               const unsigned short rtp_client_port, 
                               unsigned short *rtp_server_port)
{
    //set rtp server port
    *rtp_server_port = 20000;
}

int main()
{
    char cmd[256];
    st_Handle_Cmd_Callback cb;
    
    ::memset(&cb, 0x00, sizeof(cb));
    cb.options  = handle_Options;
    cb.describe = handle_Describe;
    cb.setup    = handle_Setup; 
    cb.play     = handle_Play; 
    cb.pause    = handle_Pause; 
    cb.teardown = handle_Teardown;

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
        printf("Please input command : \n -exit exit program\n  Please input:");
        
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

