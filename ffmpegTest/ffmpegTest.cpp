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
    int ret = avformat_open_input(&fmtCtx, "C:\\ffstudy\\sample.mp4", NULL, NULL);
    if (ret != 0) { return -1; }
    avformat_find_stream_info(fmtCtx, NULL);

    printf("스트림 개수 = %d\n", fmtCtx->nb_streams);
    printf("시간 = %I64d초\n", fmtCtx->duration / AV_TIME_BASE);
    printf("비트레이트 = %I64d\n", fmtCtx->bit_rate);

    avformat_close_input(&fmtCtx);
    return 0;
}
