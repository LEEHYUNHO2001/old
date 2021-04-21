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
#include <conio.h>

AVFormatContext* fmtCtx;
int vidx = -1, aidx = -1;
AVStream * vStream, * aStream;
AVCodecParameters* vPara, * aPara;
AVCodec * vCodec, * aCodec;
AVCodecContext * vCtx, * aCtx;
//AVPacket packet;
//AVFrame vFrame, aFrame;

void arDump(void* array, int length) {
    for (int i = 0; i < length; i++) {
        printf("%02X ", *((unsigned char*)array + i));
    }
}

int main(void) {
    AVPacket packet = { 0, };
    AVFrame vFrame = { 0, }, aFrame = { 0, };

    int ret = avformat_open_input(&fmtCtx, "C:\\ffstudy\\sample.mp4", NULL, NULL);
    if (ret != 0) { return -1; }
    avformat_find_stream_info(fmtCtx, NULL);

    //비디오, 오디오 스트림 첨자
    vidx = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    aidx = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_AUDIO, -1, vidx, NULL, 0);
    printf("------비디오, 오디오 스트림 첨자--------\n");
    printf("video = %d번, audio = %d번\n\n", vidx, aidx);

    //동영상 파일 정보
    printf("------동영상 파일 정보--------\n");
    printf("스트림 개수 = %d\n", fmtCtx->nb_streams);
    printf("시간 = %I64d초\n", fmtCtx->duration / AV_TIME_BASE);
    printf("비트레이트 = %I64d\n\n", fmtCtx->bit_rate);

    //비디오, 오디오 스트림 정보, if문 에러처리(비디오, 오디오 존재 안할수도있어서)
    printf("------비디오 스트림 정보--------\n");
    if (vidx >= 0) {
        vStream = fmtCtx->streams[vidx];
        printf("프레임 개수 = %I64d\n", vStream->nb_frames);
        printf("프레임 레이트 = %d / %d\n", vStream->avg_frame_rate.num, vStream->avg_frame_rate.den);
        printf("타임 베이스 = %d / %d\n", vStream->time_base.num, vStream->time_base.den);
        vPara = vStream->codecpar;
        printf("폭 = %d\n", vPara->width);
        printf("높이 = %d\n", vPara->height);
        printf("색상 포맷 = %d\n", vPara->format);
        printf("코덱 = %d\n\n", vPara->codec_id);
    }
    if (aidx >= 0) {
        printf("------오디오 스트림 정보--------\n");
        aStream = fmtCtx->streams[aidx];
        printf("프레임 개수 = %I64d\n", aStream->nb_frames);
        printf("타임 베이스 = %d / %d\n", aStream->time_base.num, aStream->time_base.den);
        aPara = aStream->codecpar;
        printf("사운드 포맷 = %d\n", aPara->format);
        printf("코덱 = %d\n", aPara->codec_id);
        printf("채널 = %d\n", aPara->channels);
        printf("샘플 레이트 = %d\n\n", aPara->sample_rate);
    }

    // 비디오, 오디오 코덱 오픈, if문 에러처리(비디오, 오디오 존재 안할수도있어서)
    printf("------코덱 정보 조사--------\n");
    if (vidx >= 0) {
        vStream = fmtCtx->streams[vidx];
        vPara = vStream->codecpar;
        vCodec = avcodec_find_decoder(vPara->codec_id);
        vCtx = avcodec_alloc_context3(vCodec);
        avcodec_parameters_to_context(vCtx, vPara);
        avcodec_open2(vCtx, vCodec, NULL);
        printf("비디오 코덱 : %d, %s(%s)\n", vCodec->id, vCodec->name, vCodec->long_name);
        printf("능력치 : %x\n", vCodec->capabilities);
    }
    if (aidx >= 0) {
        aStream = fmtCtx->streams[aidx];
        aPara = aStream->codecpar;
        aCodec = avcodec_find_decoder(aPara->codec_id);
        aCtx = avcodec_alloc_context3(aCodec);
        avcodec_parameters_to_context(aCtx, aPara);
        avcodec_open2(aCtx, aCodec, NULL);
        printf("오디오 코덱 : %d, %s(%s)\n", aCodec->id, aCodec->name, aCodec->long_name);
        printf("능력치 : %x\n\n", aCodec->capabilities);
    }

    // 루프를 돌며 패킷을 모두 읽는다. ret으로 에러처리 해줌.
    printf("------패킷 읽기--------\n");
    int vcount = 0, acount = 0;
    while (av_read_frame(fmtCtx, &packet) == 0) {
        if (packet.stream_index == vidx) {
            //send 음수면 루프 재시도
            ret = avcodec_send_packet(vCtx, &packet);
            if (ret != 0) { continue; }
            //for문으로 receive 반복 -> 패킷 다 읽게해서 버퍼 비우기. EAGAIN이면 버퍼 빈것!(break)
            for (;;) {
                ret = avcodec_receive_frame(vCtx, &vFrame);
                if (ret == AVERROR(EAGAIN)) break; 
                if (vcount == 0) {
                    printf("Video format : %d(%d x %d).\n",
                        vFrame.format, vFrame.width, vFrame.height);
                }
                printf("V%-3d(pts=%3I64d,size=%5d) : ", vcount++, vFrame.pts, vFrame.pkt_size);
                for (int i = 0; i < 3; i++) {
                    printf("%d ", vFrame.linesize[i]);
                }
                arDump(vFrame.data[0], 4);
                arDump(vFrame.data[1], 2);
                arDump(vFrame.data[2], 2);
            }
        }
        if (packet.stream_index == aidx) {
            ret = avcodec_send_packet(aCtx, &packet);
            if (ret != 0) { continue; }
            for (;;) {
                ret = avcodec_receive_frame(aCtx, &aFrame);
                if (ret == AVERROR(EAGAIN)) break;
                if (acount == 0) {
                    printf("Audio format : %d, %dch %d\n",
                        aFrame.format, aFrame.channels, aFrame.sample_rate);
                }
                printf("A%-3d(pts=%3I64d,size=%5d) : ", acount++, aFrame.pts, aFrame.pkt_size);
                arDump(aFrame.extended_data, 16);
            }
        }
        av_packet_unref(&packet);
        printf("\n");
        if (_getch() == 27) break;
    }

    // 메모리 해제
    av_frame_unref(&vFrame);
    av_frame_unref(&aFrame);
    avcodec_free_context(&vCtx);
    avcodec_free_context(&aCtx);
    avformat_close_input(&fmtCtx);
    return 0;
}
