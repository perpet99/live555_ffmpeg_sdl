#include "pch.h"
#include "DummySink.h"

#pragma warning (disable : 4996) 

// Implementation of "DummySink":

// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 100000

DummySink* DummySink::createNew(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId) {
	return new DummySink(env, subsession, streamId);
}

DummySink::DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId)
	: MediaSink(env),
	fSubsession(subsession) {
	fStreamId = strDup(streamId);
	fReceiveBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];
	fReceiveBufferAV = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE + 4];
	fReceiveBufferAV[0] = 0;
	fReceiveBufferAV[1] = 0;
	fReceiveBufferAV[2] = 0;
	fReceiveBufferAV[3] = 1;


	avcodec_register_all();
	av_register_all();


	av_init_packet(&avpkt);
	avpkt.flags |= AV_PKT_FLAG_KEY;
	avpkt.pts = avpkt.dts = 0;

	/* set end of buffer to 0 (this ensures that no overreading happens for damaged mpeg streams) */
	//memset(inbuf + INBUF_SIZE, 0, FF_INPUT_BUFFER_PADDING_SIZE);

	//codec = avcodec_find_decoder(CODEC_ID_MPEG1VIDEO);
	codec = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (!codec) {
		envir() << "codec not found!";
		exit(4);
	}

	c = avcodec_alloc_context3(codec);
	picture = av_frame_alloc();

	//if (codec->capabilities & CODEC_CAP_TRUNCATED) {
	//	c->flags |= CODEC_FLAG_TRUNCATED; // we do not send complete frames
	//}

	c->width = 1280;
	c->height = 720;
	c->pix_fmt = AV_PIX_FMT_YUV420P;

	/* for some codecs width and height MUST be initialized there becuase this info is not available in the bitstream */

	if (avcodec_open2(c, codec, NULL) < 0) {
		envir() << "could not open codec";
		exit(5);
	}


}

void DummySink::initSDL(int w, int h,int w2,int h2)
{

	//--------SDL Initialization----------
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER))
	{

	}
	//create window
	screen_w = w;
	screen_h = h;
	screen = SDL_CreateWindow("Simple FFmpeg Player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		screen_w, screen_h, SDL_WINDOW_OPENGL);
	if (!screen)
	{

	}
	// create renderer and texture;
	sdlRenderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	//IYUV: Y + U + V (3 planes)
	//YV12: Y + V + U (3 planes)
	sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, w2, h2);

	sdlRect.x = 0;
	sdlRect.y = 0;
	sdlRect.w = screen_w;
	sdlRect.h = screen_h;

	mutex = SDL_CreateMutex();
}

DummySink::~DummySink() {
	delete[] fReceiveBuffer;
	delete[] fStreamId;
}

void DummySink::afterGettingFrame(void* clientData, unsigned frameSize, unsigned numTruncatedBytes,
	struct timeval presentationTime, unsigned durationInMicroseconds) {
	DummySink* sink = (DummySink*)clientData;
	sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}




void pgm_save(unsigned char* buf, int wrap, int xsize, int ysize, char* filename) {
	FILE* fp;
	int i;

	fp = fopen(filename, "wb");
	fprintf(fp, "P5\n%d %d\n%d\n", xsize, ysize, 255);
	for (i = 0; i < ysize; i++) {
		fwrite(buf + i * wrap, 1, xsize, fp);
	}
	fclose(fp);

}
void DummySink::setSprop(u_int8_t const* prop, unsigned size) {
	uint8_t* buf;
	uint8_t* buf_start;
	buf = (uint8_t*)malloc(1000);
	buf_start = buf + 4;

	avpkt.data = buf;
	avpkt.data[0] = 0;
	avpkt.data[1] = 0;
	avpkt.data[2] = 0;
	avpkt.data[3] = 1;
	memcpy(buf_start, prop, size);
	avpkt.size = size + 4;
	len = avcodec_decode_video2(c, picture, &got_picture, &avpkt);
	if (len < 0) {
		envir() << "Error while decoding frame" << frame;
		//		exit(6);
	}

	envir() << "after setSprop\n";
	//	exit (111);
}

void DummySink::afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
	struct timeval presentationTime, unsigned /*durationInMicroseconds*/) {
	// We've just received a frame of data.  (Optionally) print out information about it:
#ifdef DEBUG_PRINT_EACH_RECEIVED_FRAME
	if (fStreamId != NULL) envir() << "Stream \"" << fStreamId << "\"; ";
	envir() << fSubsession.mediumName() << "/" << fSubsession.codecName() << ":\tReceived " << frameSize << " bytes";
	if (numTruncatedBytes > 0) envir() << " (with " << numTruncatedBytes << " bytes truncated)";
	char uSecsStr[6 + 1]; // used to output the 'microseconds' part of the presentation time
	sprintf(uSecsStr, "%06u", (unsigned)presentationTime.tv_usec);
	envir() << ".\tPresentation time: " << (int)presentationTime.tv_sec << "." << uSecsStr;
	if (fSubsession.rtpSource() != NULL && !fSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP()) {
		envir() << "!"; // mark the debugging output to indicate that this presentation time is not RTCP-synchronized
	}
#ifdef DEBUG_PRINT_NPT
	envir() << "\tNPT: " << fSubsession.getNormalPlayTime(presentationTime);
#endif
	envir() << "\n";
#endif
	if (strcmp(fSubsession.codecName(), "H264") == 0) {
		avpkt.data = fReceiveBufferAV;
		//	r2sprop();
		//	r2sprop2();
		//	avpkt.size = (int)fReceiveBuffer[0];
		avpkt.size = frameSize + 4;
		//	avpkt.size = frameSize;
		if (avpkt.size != 0) {
			memcpy(fReceiveBufferAV + 4, fReceiveBuffer, frameSize);
			avpkt.data = fReceiveBufferAV; //+2;
	//		avpkt.data = fReceiveBuffer; //+2;
			len = avcodec_decode_video2(c, picture, &got_picture, &avpkt);
			if (len < 0) {
				envir() << "Error while decoding frame" << frame;
				//			exit(6);
			}
			if (got_picture) {
				// do something with it
				if (!bInitSDL)
				{
					bInitSDL = true;

					initSDL(640, 480, c->width, c->height);

				}

				if (1)
				{
					//https://github.com/ramsafin/RTSPLiveClient/blob/master/src/SDLVideoSink.cpp

					SDL_LockMutex(mutex);

					SDL_UpdateYUVTexture(sdlTexture, nullptr, picture->data[0], picture->linesize[0],
						picture->data[1], picture->linesize[1], picture->data[2], picture->linesize[2]);

					SDL_RenderClear(sdlRenderer);
					SDL_RenderCopy(sdlRenderer, sdlTexture, nullptr, nullptr);
					SDL_RenderPresent(sdlRenderer);

					SDL_UnlockMutex(mutex);
				}
				else
				{
					//sws_scale(img_convert_ctx, (const uint8_t* const*)picture->data, picture->linesize, 0,
					//	c->height, pFrameYUV->data, pFrameYUV->linesize);
					////SDL 
					//SDL_UpdateTexture(sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);
					//SDL_RenderClear(sdlRenderer);
					//SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
					//SDL_RenderPresent(sdlRenderer);
				}


				//sws_freeContext(img_convert_ctx);
				frame++;
			}
			else {
				envir() << "no picture :( !\n";
			}
		}

	}



	SDL_PollEvent(&event);

	switch (event.type) {
	case SDL_QUIT:
		SDL_Quit();
		exit(0);
	default:
		break;
	}


	// Then continue, to request the next frame of data:
	continuePlaying();
}

Boolean DummySink::continuePlaying() {
	if (fSource == NULL) return False; // sanity check (should not happen)

	// Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
	fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE,
		afterGettingFrame, this,
		onSourceClosure, this);
	return True;
}


// If you don't want to see debugging output for each received frame, then comment out the following line:
//#define DEBUG_PRINT_EACH_RECEIVED_FRAME 1
/*
void DummySink::r2sprop2() {
	avpkt.data[0]	= 0;
	avpkt.data[1]	= 0;
	avpkt.data[2]	= 0;
	avpkt.data[3]	= 1;
	avpkt.data[4]	= 0x68;
	avpkt.data[5]	= 0xeb;
	avpkt.data[6]	= 0xe3;
	avpkt.data[7]	= 0xcb;
	avpkt.data[8]	= 0x22;
	avpkt.data[9]	= 0xc0;
	avpkt.size=10;
	len = avcodec_decode_video2 (c, picture, &got_picture, &avpkt);
	if (len < 0) {
//		envir() << "Error while decoding frame" << frame;
//		exit(6);
	}
}
void DummySink::r2sprop() {
	avpkt.data[0]	= 0;
	avpkt.data[1]	= 0;
	avpkt.data[2]	= 0;
	avpkt.data[3]	= 1;
	avpkt.data[4]	= 0x67;
	avpkt.data[5]	= 0x64;
	avpkt.data[6]	= 0x00;
	avpkt.data[7]	= 0x1e;
	avpkt.data[8]	= 0xac;
	avpkt.data[9]	= 0xd9;
	avpkt.data[10]	= 0x40;
	avpkt.data[11]	= 0xa0;
	avpkt.data[12]	= 0x2f;
	avpkt.data[13]	= 0xf9;
	avpkt.data[14]	= 0x70;
	avpkt.data[15]	= 0x11;
	avpkt.data[16]	= 0x00;
	avpkt.data[17]	= 0x00;
	avpkt.data[18]	= 0x03;
	avpkt.data[19]	= 0x03;
	avpkt.data[20]	= 0xe8;
	avpkt.data[21]	= 0x00;
	avpkt.data[22]	= 0x00;
	avpkt.data[23]	= 0xe9;
	avpkt.data[24]	= 0xba;
	avpkt.data[25]	= 0x8f;
	avpkt.data[26]	= 0x16;
	avpkt.data[27]	= 0x2d;
	avpkt.data[28]	= 0x96;
	avpkt.size=29;
	len = avcodec_decode_video2 (c, picture, &got_picture, &avpkt);
	if (len < 0) {
//		envir() << "Error while decoding frame" << frame;
//		exit(6);
	}
}
*/