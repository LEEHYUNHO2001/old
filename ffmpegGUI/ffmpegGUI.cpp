extern "C" {
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
}

#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
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

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {

    HWND hWnd;
    MSG Message;
    WNDCLASS WndClass;

    g_hInst = hInstance;

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


    hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, (HMENU)NULL, hInstance, NULL);
    ShowWindow(hWnd, nCmdShow);

    while (GetMessage(&Message, NULL, 0, 0)) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }
    return (int)Message.wParam;
}


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

void CloseMovie() {
    if (vCtx) { avcodec_free_context(&vCtx); }
    if (aCtx) { avcodec_free_context(&aCtx); }
    if (fmtCtx) { avformat_close_input(&fmtCtx); }
}

int DrawFrame(HDC hdc) {

    int ret;
    AVPacket packet = { 0, };
    AVFrame vFrame = { 0, };

    while (av_read_frame(fmtCtx, &packet) == 0) {
        if (packet.stream_index == vidx) {
            ret = avcodec_send_packet(vCtx, &packet);
            if (ret != 0) { continue; }
            for (;;) {
                ret = avcodec_receive_frame(vCtx, &vFrame);
                if (ret == AVERROR(EAGAIN)) break;
                TCHAR info[128];
                wsprintf(info, TEXT("Video format : %d(%d x %d)"),
                    vFrame.format, vFrame.width, vFrame.height);
                TextOut(hdc, 10, 10, info, lstrlen(info));
            }
            av_packet_unref(&packet);
            return 0;
        }
    }
    av_frame_unref(&vFrame);
    return 1;
}