// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件

#ifndef PCH_H
#define PCH_H

// TODO: 添加要在此处预编译的标头


#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")

#pragma comment(lib, "BasicUsageEnvironment.lib")
#pragma comment(lib, "groupsock.lib")
#pragma comment(lib, "liveMedia.lib")
#pragma comment(lib, "UsageEnvironment.lib")

#pragma comment(lib, "SDL2.lib")

#pragma comment(lib, "SDL2main.lib")

//#pragma comment(lib, "Wininet.lib")
//#pragma comment(lib, "Iphlpapi.lib")

#pragma comment(lib, "Ws2_32.lib")

extern "C" {
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"

#include "SDL.h"
#include "SDL_main.h"

}

#include "UsageEnvironment.hh"
#include "BasicUsageEnvironment.hh"

#include "RTSPClient.hh"
#include "H264VideoRTPSource.hh"

#endif //PCH_H
