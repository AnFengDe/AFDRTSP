/*!
 * \author chtian
 * \brief AFD RTP Client implementation	
 * \file AFDRTPClient.cpp
 * \version 1.0 
 * \date 2012-04-25 23:37
 * */

#include "rtpsession.h"
#include "rtppacket.h"
#include "rtpipv4address.h"
#include "rtpsessionparams.h"
#include "rtpudpv4transmitter.h"
#include "rtptransmitter.h"

typedef struct _t_RTPAddress
{
    uint16_t portbase;
    uint16_t destport;
    uint32_t destip;

    _t_RTPAddress()
    {   
        portbase = 0;
        destport = 0;
        destip = 0;
    }   
}t_RTPAddress;

class RTPClientSession : public RTPSession
{
public:
    RTPClientSession();
    ~RTPClientSession();


    RTPIPv4Address RTPAddr;
};

RTPClientSession::RTPClientSession()
{
}

RTPClientSession::~RTPClientSession()
{
    RTPSession::Destroy();
}

extern "C" bool init_rtp_client()
{
    return true;
}


extern "C" bool cleanup_rtp_client()
{
    return true;
}

extern "C" const void* create_rtp_client()
{
    return NULL;
}
