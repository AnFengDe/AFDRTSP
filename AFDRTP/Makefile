INCLUDES = -I. 
##### Change the following for your environment: 
COMPILE_OPTS =		$(INCLUDES) -I. -O2 -DDEBUG -DSOCKLEN_T=socklen_t -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64
C =			c
C_COMPILER =		cc
C_FLAGS =		$(COMPILE_OPTS)
CPP =			cpp
CPLUSPLUS_COMPILER =	c++
CPLUSPLUS_FLAGS =	$(COMPILE_OPTS) -Wall -fpic -DPIC 
OBJ =			o
LINK =			c++ -o
LINK_OPTS =		-L.
CONSOLE_LINK_OPTS =	$(LINK_OPTS)
LIBRARY_LINK =		ar cr 
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
		$(AFDRTP_LIB_OBJS)

AFDRTPServer.$(CPP):	AFDRTPServer.h
AFDRTPClient.$(CPP):	AFDRTPClient.h  

clean:
	-rm -rf *.$(OBJ) $(ALL) core *.core *~ include/*~

##### Any additional, platform-specific rules come here:
