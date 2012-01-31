typedef void (GetBuffer)(void* clientId, unsigned char const *clientData, unsigned frameSize, double duration,struct timeval presentationTime );

typedef void (GetResult)( void* clientId, int resultCode, char* resultString);

typedef void (GetRtcp)(void* clientId, char *clientData);

typedef void (GetSdp)(void* clientId, char *clientData);

void* CreateClient();

int Play(void* clientId, char* RTSP_URL,GetBuffer* CallBackForGetBuffer);

int Pause( void* clientId );

int RePlay(void* clientId,double percent);

int Fast(void* clientId,double resacle);

int Slow(void* clientId,double resacle);

int Stop(void* clientId );

