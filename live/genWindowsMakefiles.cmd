@Echo OFF
SETLOCAL
for %%I in (%0) do %%~dI
for %%I in (%0) do cd "%%~pI"
cd liveMedia
del /Q liveMedia.mak
type Makefile.head ..\win32config Makefile.tail > liveMedia.mak

cd ../groupsock
del /Q groupsock.mak
type Makefile.head ..\win32config Makefile.tail > groupsock.mak

cd ../UsageEnvironment
del /Q UsageEnvironment.mak
type Makefile.head ..\win32config Makefile.tail > UsageEnvironment.mak

cd ../BasicUsageEnvironment
del /Q BasicUsageEnvironment.mak
type Makefile.head ..\win32config Makefile.tail > BasicUsageEnvironment.mak

cd ../jthread-1.3.1
del /Q jthread.mak
type Makefile.head ..\win32config Makefile.tail > jthread.mak

cd ../AFDRTSP
del /Q AFDRTSP.mak
type Makefile.head ..\win32config Makefile.tail > AFDRTSP.mak

cd ../testProgs
del /Q testProgs.mak
type Makefile.head ..\win32config Makefile.tail > testProgs.mak

cd ../mediaServer
del /Q mediaServer.mak
type Makefile.head ..\win32config Makefile.tail > mediaServer.mak

ENDLOCAL
