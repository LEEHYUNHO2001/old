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

#include <windows.h>//그래픽 환경
#include <commctrl.h>//공통 컨트롤
#include <tchar.h>//유니코드 문자열 처리
#include <stdio.h>

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
void OpenMovie(LPCTSTR movie);
void CloseMovie();
int DrawFrame(HDC hdc);

HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("MicroPlayer");
HWND hWndMain;

AVFormatContext* fmtCtx;
int vidx, aidx;
AVStream* vStream, * aStream;
AVCodecParameters* vPara, * aPara;
AVCodec* vCodec, * aCodec;
AVCodecContext* vCtx, * aCtx;

bool isOpen;

SwsContext* swsCtx;
AVFrame RGBFrame;
uint8_t* rgbbuf;

//Main
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
    HWND hWnd;
    MSG Message;
    WNDCLASS WndClass;
    g_hInst = hInstance;
    //WndClass
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WndClass.hInstance = hInstance;
    WndClass.lpfnWndProc = WndProc;
    WndClass.lpszClassName = lpszClass;
    WndClass.lpszMenuName = NULL;
    WndClass.style = 0;
    RegisterClass(&WndClass);
    //CreateWindow
    hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, (HMENU)NULL, hInstance, NULL);
    ShowWindow(hWnd, nCmdShow);
    //메세지 루프
    while (GetMessage(&Message, NULL, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }
    return (int)Message.wParam;
}
//Window 프로시저
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
    HDC hdc;
    PAINTSTRUCT ps;

    switch (iMessage) {
    case WM_CREATE:
        hWndMain = hWnd;
        InitCommonControls();
        OpenMovie(TEXT("c:\\ffstudy\\sample.mp4"));
        SetTimer(hWnd, 0, 33, NULL);
        return 0;

    case WM_TIMER:
        hdc = GetDC(hWnd);
        if (isOpen) {
            if (DrawFrame(hdc) == 1) {
                KillTimer(hWnd, 0);
            }
        }
        ReleaseDC(hWnd, hdc);
        return 0;

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        return 0;

    case WM_DESTROY:
        CloseMovie();
        PostQuitMessage(0);
        return 0;
    }
    return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}
//코덱 오픈
void OpenMovie(LPCTSTR movie) {
    char MoviePathAnsi[MAX_PATH];
    WideCharToMultiByte(CP_ACP, 0, movie, -1, MoviePathAnsi, MAX_PATH, NULL, NULL);
    int ret = avformat_open_input(&fmtCtx, MoviePathAnsi, NULL, NULL);

    if (ret != 0) {
        isOpen = false;
        MessageBox(hWndMain, TEXT("동영상 파일이 없습니다."), TEXT("알림"), MB_OK);
        return;
    }
    avformat_find_stream_info(fmtCtx, NULL);
    vidx = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    aidx = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_AUDIO, -1, vidx, NULL, 0);
    if (vidx >= 0) {
        vStream = fmtCtx->streams[vidx];
        vPara = vStream->codecpar;
        vCodec = avcodec_find_decoder(vPara->codec_id);
        vCtx = avcodec_alloc_context3(vCodec);
        avcodec_parameters_to_context(vCtx, vPara);
        avcodec_open2(vCtx, vCodec, NULL);
    }
    if (aidx >= 0) {
        aStream = fmtCtx->streams[aidx];
        aPara = aStream->codecpar;
        aCodec = avcodec_find_decoder(aPara->codec_id);
        aCtx = avcodec_alloc_context3(aCodec);
        avcodec_parameters_to_context(aCtx, aPara);
        avcodec_open2(aCtx, aCodec, NULL);
    }
    isOpen = true;
}
//메모리 정리
void CloseMovie() {
    if (rgbbuf) { av_free(rgbbuf); rgbbuf = NULL; }
    if (swsCtx) { sws_freeContext(swsCtx); swsCtx = NULL; }
    if (vCtx) { avcodec_free_context(&vCtx); }
    if (aCtx) { avcodec_free_context(&aCtx); }
    if (fmtCtx) { avformat_close_input(&fmtCtx); }
}
//패킷 읽고 처리
int DrawFrame(HDC hdc) {
    int ret;
    AVPacket packet = { 0, };
    AVFrame vFrame = { 0, }, aFrame = { 0, };
    HDC MemDC;
    HBITMAP OldBitmap;
    MemDC = CreateCompatibleDC(hdc);

    while (av_read_frame(fmtCtx, &packet) == 0) {
        if (packet.stream_index == vidx) {
            ret = avcodec_send_packet(vCtx, &packet);
            if (ret != 0) { continue; }
            for (;;) {
                ret = avcodec_receive_frame(vCtx, &vFrame);
                if (ret == AVERROR(EAGAIN)) break;
                // 스케일 컨텍스트 생성
                if (swsCtx == NULL) {
                    swsCtx = sws_getContext(
                        vFrame.width, vFrame.height, AVPixelFormat(vFrame.format),
                        vFrame.width, vFrame.height, AV_PIX_FMT_BGRA,
                        SWS_BICUBIC, NULL, NULL, NULL);

                    // 변환 결과를 저장할 프레임 버퍼 할당
                    int rasterbufsize = av_image_get_buffer_size(AV_PIX_FMT_BGRA,
                        vFrame.width, vFrame.height, 1);
                    rgbbuf = (uint8_t*)av_malloc(rasterbufsize);
                    av_image_fill_arrays(RGBFrame.data, RGBFrame.linesize, rgbbuf,
                        AV_PIX_FMT_BGRA, vFrame.width, vFrame.height, 1);
                }
                // 변환을 수행한다.
                sws_scale(swsCtx, vFrame.data, vFrame.linesize, 0, vFrame.height,
                    RGBFrame.data, RGBFrame.linesize);
                // 비트맵으로 뿌리기
                HBITMAP bitmap = CreateBitmap(vFrame.width, vFrame.height, 1, 32, RGBFrame.data[0]);
                OldBitmap = (HBITMAP)SelectObject(MemDC, bitmap);
                BitBlt(hdc, 0, 0, vFrame.width, vFrame.height, MemDC, 0, 0, SRCCOPY);
                SelectObject(MemDC, OldBitmap);
                DeleteObject(bitmap);
            }
            av_packet_unref(&packet);
            return 0;
        }
    }
    av_frame_unref(&vFrame);
    av_frame_unref(&aFrame);
    return 1;
}