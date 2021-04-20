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
int vidx = -1, aidx = -1;
AVStream * vStream, * aStream;
AVCodecParameters* vPara, * aPara;
AVCodec * vCodec, * aCodec;
AVCodecContext * vCtx, * aCtx;

int main(void) {
    int ret = avformat_open_input(&fmtCtx, "C:\\ffstudy\\sample.mp4", NULL, NULL);
    if (ret != 0) { return -1; }
    avformat_find_stream_info(fmtCtx, NULL);

    //비디오, 오디오 스트림 첨자
    vidx = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    aidx = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_AUDIO, -1, vidx, NULL, 0);

    printf("------비디오, 오디오 스트림 첨자--------\n");
    printf("video = %d번, audio = %d번\n\n", vidx, aidx);

    printf("------동영상 파일 정보--------\n");
    printf("스트림 개수 = %d\n", fmtCtx->nb_streams);
    printf("시간 = %I64d초\n", fmtCtx->duration / AV_TIME_BASE);
    printf("비트레이트 = %I64d\n", fmtCtx->bit_rate);


    printf("------비디오 스트림 정보--------\n");
    vStream = fmtCtx->streams[vidx];
    printf("프레임 개수 = %I64d\n", vStream->nb_frames);
    printf("프레임 레이트 = %d / %d\n", vStream->avg_frame_rate.num, vStream->avg_frame_rate.den);
    printf("타임 베이스 = %d / %d\n", vStream->time_base.num, vStream->time_base.den);
    vPara = vStream->codecpar;
    printf("폭 = %d\n", vPara->width);
    printf("높이 = %d\n", vPara->height);
    printf("색상 포맷 = %d\n", vPara->format);
    printf("코덱 = %d\n", vPara->codec_id);

    printf("------오디오 스트림 정보--------\n");
    aStream = fmtCtx->streams[aidx];
    printf("프레임 개수 = %I64d\n", aStream->nb_frames);
    printf("타임 베이스 = %d / %d\n", aStream->time_base.num, aStream->time_base.den);
    aPara = aStream->codecpar;
    printf("사운드 포맷 = %d\n", aPara->format);
    printf("코덱 = %d\n", aPara->codec_id);
    printf("채널 = %d\n", aPara->channels);
    printf("샘플 레이트 = %d\n", aPara->sample_rate);

    // 비디오 코덱 오픈
    vStream = fmtCtx->streams[vidx];
    vPara = vStream->codecpar;
    vCodec = avcodec_find_decoder(vPara->codec_id);
    vCtx = avcodec_alloc_context3(vCodec);
    avcodec_parameters_to_context(vCtx, vPara);
    avcodec_open2(vCtx, vCodec, NULL);

    // 오디오 코덱 오픈
    aStream = fmtCtx->streams[aidx];
    aPara = aStream->codecpar;
    aCodec = avcodec_find_decoder(aPara->codec_id);
    aCtx = avcodec_alloc_context3(aCodec);
    avcodec_parameters_to_context(aCtx, aPara);
    avcodec_open2(aCtx, aCodec, NULL);

    printf("------코덱 정보 조사--------\n");
    printf("비디오 코덱 : %d, %s(%s)\n", vCodec->id, vCodec->name, vCodec->long_name);
    printf("능력치 : %x\n", vCodec->capabilities);
    printf("오디오 코덱 : %d, %s(%s)\n", aCodec->id, aCodec->name, aCodec->long_name);
    printf("능력치 : %x\n", aCodec->capabilities);
    avcodec_free_context(&vCtx);
    avcodec_free_context(&aCtx);

    avformat_close_input(&fmtCtx);
    return 0;
}
