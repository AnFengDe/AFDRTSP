/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2011 Live Networks, Inc.  All rights reserved.
// File sinks
// Implementation

#if (defined(__WIN32__) || defined(_WIN32)) && !defined(_WIN32_WCE)
#include <io.h>
#include <fcntl.h>
#endif
#include "FileSink.hh"
#include "GroupsockHelper.hh"
#include "OutputFile.hh"

////////// FileSink //////////

FileSink::FileSink(UsageEnvironment& env, FILE* fid, unsigned bufferSize,
		   char const* perFrameFileNamePrefix,ClientInterface* newclient)
  : MediaSink(env), fOutFid(fid), fBufferSize(bufferSize) {
  fBuffer = new unsigned char[bufferSize];
  if (perFrameFileNamePrefix != NULL) {
    fPerFrameFileNamePrefix = strDup(perFrameFileNamePrefix);
    fPerFrameFileNameBuffer = new char[strlen(perFrameFileNamePrefix) + 100];
  } else {
    fPerFrameFileNamePrefix = NULL;
    fPerFrameFileNameBuffer = NULL;
  }
  memset(fOutFileName, '\0', sizeof(fOutFileName));
  mediaclient = newclient;
}

FileSink::~FileSink() {
  delete[] fPerFrameFileNameBuffer;
  delete[] fPerFrameFileNamePrefix;
  delete[] fBuffer;
  if (fOutFid != NULL) fclose(fOutFid);
}

FileSink* FileSink::createNew(UsageEnvironment& env, char const* fileName,
			      unsigned bufferSize, Boolean OneFilePerFrame,ClientInterface* newclient) {

  do {
    FILE* fid;
    char const* perFrameFileNamePrefix;
    if (OneFilePerFrame) {
      // Create the fid for each frame
      fid = NULL;
      perFrameFileNamePrefix = fileName;
    } else {
      // Normal case: create the fid once	
     // fid = OpenOutputFile(env, fileName);
	fid = NULL;
#if 0
      if (fid == NULL) break;
#endif
      perFrameFileNamePrefix = NULL;
    }

    return new FileSink(env, fid, bufferSize, perFrameFileNamePrefix,newclient);
  } while (0);

  return NULL;
}

Boolean FileSink::continuePlaying() {
  if (fSource == NULL) return False;

  fSource->getNextFrame(fBuffer, fBufferSize,
			afterGettingFrame, this,
			onSourceClosure, this);

  return True;
}

void FileSink::afterGetingFrameData(afterGettingData *afterGetFunc,char *outFileName)
{
	fAfterGetFunc = afterGetFunc;
	strcpy(fOutFileName,outFileName);
}

void FileSink::afterGettingFrame(void* clientData, unsigned frameSize,
				 unsigned /*numTruncatedBytes*/,
				 struct timeval presentationTime,
				 unsigned /*durationInMicroseconds*/) {
  FileSink* sink = (FileSink*)clientData;
  sink->afterGettingFrame1(frameSize, presentationTime);
}

void FileSink::addData(unsigned char const* data, unsigned dataSize,
		       struct timeval presentationTime) {

   	if (fAfterGetFunc != NULL)
	{
		(*fAfterGetFunc)(data,dataSize,presentationTime,fOutFileName,mediaclient); //Callback, zhaojin
	}

  if (fPerFrameFileNameBuffer != NULL) {
    // Special case: Open a new file on-the-fly for this frame
    sprintf(fPerFrameFileNameBuffer, "%s-%lu.%06lu", fPerFrameFileNamePrefix,
	    presentationTime.tv_sec, presentationTime.tv_usec);
#if 0
    fOutFid = OpenOutputFile(envir(), fPerFrameFileNameBuffer);
#endif
	
	fOutFid = NULL;
  }

  // Write to our file:
#ifdef TEST_LOSS
  static unsigned const framesPerPacket = 10;
  static unsigned const frameCount = 0;
  static Boolean const packetIsLost;
  if ((frameCount++)%framesPerPacket == 0) {
    packetIsLost = (our_random()%10 == 0); // simulate 10% packet loss #####
  }

  if (!packetIsLost)
#endif
	  //write file, zhaojin
#if 0
  if (fOutFid != NULL && data != NULL) {
    fwrite(data, 1, dataSize, fOutFid);

  }
#endif
}

void FileSink::afterGettingFrame1(unsigned frameSize,
				  struct timeval presentationTime) {
  addData(fBuffer, frameSize, presentationTime);
#if 0
  if (fOutFid == NULL || fflush(fOutFid) == EOF) {
    // The output file has closed.  Handle this the same way as if the
    // input source had closed:
    onSourceClosure(this);

    stopPlaying();
    return;
  }

  if (fPerFrameFileNameBuffer != NULL) {
    if (fOutFid != NULL) { fclose(fOutFid); fOutFid = NULL; }
  }
#endif
  // Then try getting the next frame:
  continuePlaying();
}
