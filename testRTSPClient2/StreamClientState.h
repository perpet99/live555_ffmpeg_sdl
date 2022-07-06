#pragma once
#include "../liveMedia/include/MediaSession.hh"
#include "../UsageEnvironment/include/UsageEnvironment.hh"


// Define a class to hold per-stream state that we maintain throughout each stream's lifetime:

class StreamClientState {
public:
	StreamClientState();
	virtual ~StreamClientState();

public:
	MediaSubsessionIterator* iter;
	MediaSession* session;
	MediaSubsession* subsession;
	TaskToken streamTimerTask;
	double duration;
};

