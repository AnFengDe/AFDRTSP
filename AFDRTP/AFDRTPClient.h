/*!
 * \author chtian
 * \brief AFD RTP Client API definition	
 * \file AFDRTPClient.h
 * \version 1.0 
 * \date 2012-04-25 23:36
 * */
#ifndef _AFDRTPCLIENT_H_
#define _AFDRTPCLIENT_H_

#ifdef __cplusplus
extern "C" {
#endif
bool init_rtp_client();

bool cleanup_rtp_client();

const void* create_rtp_client();


#ifdef __cplusplus
}
#endif

#endif
