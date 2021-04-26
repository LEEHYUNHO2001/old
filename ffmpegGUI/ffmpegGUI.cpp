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
        OpenMovie(TEXT("C:\\ffstudy\\sample.mp4"));
        return 0;

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        return 0;

    case WM_LBUTTONDOWN:
        hdc = GetDC(hWnd);
        if (isOpen) {
            DrawFrame(hdc);
        }
        ReleaseDC(hWnd, hdc);
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
    if (vCtx) { avcodec_free_context(&vCtx); }
    if (aCtx) { avcodec_free_context(&aCtx); }
    if (fmtCtx) { avformat_close_input(&fmtCtx); }
}
//패킷 읽고 처리
int DrawFrame(HDC hdc) {
    int ret;
    AVPacket packet = { 0, };
    AVFrame vFrame = { 0, }, aFrame = { 0, };

    while (av_read_frame(fmtCtx, &packet) == 0) {
        if (packet.stream_index == vidx) {
            ret = avcodec_send_packet(vCtx, &packet);
            if (ret != 0) { continue; }
            for (;;) {
                ret = avcodec_receive_frame(vCtx, &vFrame);
                if (ret == AVERROR(EAGAIN)) break;

                // 압축 해제한 이미지 출력
                for (int y = 0; y < vFrame.height; y++) {
                    for (int x = 0; x < vFrame.width; x++) {
                        // 프레임 버퍼에서 YUV 요소를 구한다. 420
                        unsigned char Y, U, V;
                        Y = vFrame.data[0][vFrame.linesize[0] * y + x];
                        U = vFrame.data[1][vFrame.linesize[1] * (y / 2) + x / 2];
                        V = vFrame.data[2][vFrame.linesize[2] * (y / 2) + x / 2];
                        // 공식에 따라 YUV를 RGB로 변환한다.
                        int r, g, b;
                        r = int(Y + 1.3707 * (V - 128));
                        g = int(Y - 0.6980 * (U - 128) - 0.3376 * (V - 128));
                        b = int(Y + 1.7324 * (U - 128));
                        // 색상 요소가 0 ~ 255 범위를 넘지 않도록 한다.
                        r = max(0, min(255, r));
                        g = max(0, min(255, g));
                        b = max(0, min(255, b));
                        // 색상값으로 점을 찍는다.
                        COLORREF color = RGB(r, g, b);
                        SetPixel(hdc, x, y, color);
                    }
                }
            }
            av_packet_unref(&packet);
            return 0;
        }
    }
    av_frame_unref(&vFrame);
    av_frame_unref(&aFrame);
    return 1;
}