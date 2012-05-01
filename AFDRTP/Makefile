INCLUDES = -I. 
##### Change the following for your environment: 
COMPILE_OPTS =		$(INCLUDES) -I. -I../jrtplib-3.8.2/src/ -I../jthread-1.2.1/src -O2 
C =			c
C_COMPILER =		cc
C_FLAGS =		$(COMPILE_OPTS)
CPP =			cpp
CPLUSPLUS_COMPILER =	c++
CPLUSPLUS_FLAGS =	$(COMPILE_OPTS) -Wall -fPIC 
OBJ =			o
LINK =			c++ -o
LINK_OPTS =		-L.
CONSOLE_LINK_OPTS =	$(LINK_OPTS)
LIBRARY_LINK =		ar cr 
#LIBRARY_LINK_OPTS = -L../jrtplib-3.8.2/src/.libs -ljrtp -L../jthread-1.2.1/src/.libs -ljthread 
LIBRARY_LINK_OPTS =  
LIB_SUFFIX =			a
LIBS_FOR_CONSOLE_APPLICATION =
LIBS_FOR_GUI_APPLICATION =
EXE =
JTHREAD_SRC = src/pthread
##### End of variables to change

AFDRTP_LIB = libAFDRTP.$(LIB_SUFFIX)
ALL = $(AFDRTP_LIB)
all:	$(ALL)

.$(C).$(OBJ):
	$(C_COMPILER) -c $(C_FLAGS) $<       

.$(CPP).$(OBJ):
	$(CPLUSPLUS_COMPILER) -c $(CPLUSPLUS_FLAGS) $<

RTP_OBJS = AFDRTPServer.$(OBJ) AFDRTPClient.$(OBJ) 

AFDRTP_LIB_OBJS = $(RTP_OBJS)  

$(AFDRTP_LIB): $(AFDRTP_LIB_OBJS) \
    $(PLATFORM_SPECIFIC_LIB_OBJS)
	$(LIBRARY_LINK)$@ $(LIBRARY_LINK_OPTS) \
		../jrtplib-3.8.2/src/.libs/libjrtp.a ../jthread-1.2.1/src/.libs/libjthread.a $(AFDRTP_LIB_OBJS)

AFDRTPServer.$(CPP):	
#AFDRTPClient.$(CPP):	rtpsession.h rtppacket.h rtpipv4address.h rtpsessionparams.h rtpudpv4transmitter.h rtptransmitter.h   
#rtpsession.h:    rtpconfig.h rtplibraryversion.h rtppacketbuilder.h rtpsessionsources.h rtptransmitter.h rtpcollisionlist.h rtcpscheduler.h rtcppacketbuilder.h rtptimeutilities.h rtcpcompoundpacketbuilder.h rtpmemoryobject.h
#rtpconfig.h:

clean:
	-rm -rf *.$(OBJ) $(ALL) core *.core *~ include/*~

##### Any additional, platform-specific rules come here:
