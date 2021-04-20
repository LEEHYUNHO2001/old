#pragma comment( lib, "avformat.lib" )
#pragma comment( lib, "avutil.lib" )
extern "C" {
#include <libavformat/avformat.h> //컨테이너 먹서, 디먹서
#include <libavcodec/avcodec.h> //디코더, 인코더
#include <libavdevice/avdevice.h>//캡처 및 랜더링 기능 제공
#include <libavfilter/avfilter.h>//미디어 필터
#include <libavutil/avutil.h>//난수 생성기, 수학 루틴 등의 유틸리티 기능 제공
#include <libswscale/swscale.h>//이미지 스케일링, 색상 변환
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>//오디오 리샘플링
}

#include <stdio.h>
#include <Windows.h>

AVFormatContext* fmtCtx;

int main(void) {
    av_log(NULL, AV_LOG_INFO, "AVFromat major version = %d\n", LIBAVFORMAT_VERSION_MAJOR);
    return 0;
}