
cd liveMedia
del *.obj *.lib
nmake /B /f liveMedia.mak

cd ../groupsock
del *.obj *.lib
nmake /B /f groupsock.mak

cd ../UsageEnvironment
del *.obj *.lib
nmake /B /f UsageEnvironment.mak

cd ../BasicUsageEnvironment
del *.obj *.lib
nmake /B /f BasicUsageEnvironment.mak

cd ../AFDRTSP
del *.obj *.lib *.exe
nmake /B /f AFDRTSP.mak

cd ../testProgs
del *.obj *.lib *.exe
nmake /B /f testProgs.mak

cd ../mediaServer
del *.obj *.lib
nmake /B /f mediaServer.mak

cd ../
