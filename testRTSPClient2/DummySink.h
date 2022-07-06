#pragma once

#include "liveMedia.hh"
#include "UsageEnvironment.hh"
#include "NetCommon.h"




#define INBUF_SIZE 4096

// Define a data sink (a subclass of "MediaSink") to receive the data for each subsession (i.e., each audio or video 'substream').
// In practice, this might be a class (or a chain of classes) that decodes and then renders the incoming audio or video.
// Or it might be a "FileSink", for outputting the received data into a file (as is done by the "openRTSP" application).
// In this example code, however, we define a simple 'dummy' sink that receives incoming data, but does nothing with it.

class DummySink : public MediaSink {
public:
	static DummySink* createNew(UsageEnvironment& env,
		MediaSubsession& subsession, // identifies the kind of data that's being received
		char const* streamId = NULL); // identifies the stream itself (optional)

private:
	DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId);
	// called only by "createNew()"
	virtual ~DummySink();

	static void afterGettingFrame(void* clientData, unsigned frameSize,
		unsigned numTruncatedBytes,
		struct timeval presentationTime,
		unsigned durationInMicroseconds);
	void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
		struct timeval presentationTime, unsigned durationInMicroseconds);
	void initSDL(int w, int h, int w2, int h2);
private:
	// redefined virtual functions:
	virtual Boolean continuePlaying();

private:
	u_int8_t* fReceiveBuffer;
	u_int8_t* fReceiveBufferAV;
	MediaSubsession& fSubsession;
	char* fStreamId;

private: //H264
	u_int8_t const* sps;
	unsigned spsSize;
	u_int8_t const* pps;
	unsigned ppsSize;
public:	void setSprop(u_int8_t const* prop, unsigned size);

private: //FFMPEG
	AVCodec* codec;
	AVCodecContext* c;
	int frame;
	int got_picture;
	int len;
	AVFrame* picture;
	//uint8_t inbuf[INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];
	char buf[1024];
	AVPacket avpkt;

private: //SDL

	//SDL Variables
	int screen_w, screen_h;
	SDL_Window* screen;
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
	SDL_Rect sdlRect;
	SDL_Thread* video_tid;
	SDL_Event event;
	AVPacket* packet;

	SDL_mutex* mutex;
	bool bInitSDL = false;
//
//	SDL_Surface* screen;
////	SDL_Overlay* bmp;
//	SDL_Rect rect;
};