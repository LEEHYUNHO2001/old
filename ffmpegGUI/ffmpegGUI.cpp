//video -> 비트맵, 스레드, software scale, winapi(32)구조, usleep 등 사용
//audio -> Waveform
#pragma warning (disable : 6387)
extern "C" {
#include <libavcodec/avcodec.h>//디코더, 인코더
#include <libavdevice/avdevice.h>//캡처 및 랜더링 기능 제공
#include <libavfilter/avfilter.h>//미디어 필터
#include <libavformat/avformat.h>//컨테이너 먹서, 디먹서
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
LRESULT CALLBACK StageWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PanelWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ListWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

void OpenMovie(LPCTSTR movie);
void CloseMovie();
DWORD WINAPI PlayThread(LPVOID para);
void uSleep(int64_t usec);
DWORD WINAPI CallbackThread(LPVOID para);
void Relayout(int width, int height);
void OpenMediaFile(bool reset);
void AdjustWindowSizePos(int width, int height);

//패널내의 컨트롤ID. 패널의 높이
enum {
	ID_BTNOPEN = 1, ID_BTNPAUSE, ID_STTIME, ID_SLVOLUME,
	ID_BTNEXIT, ID_BTNFULL, ID_BTNMAX, ID_BTNMIN
};
const int PanelHeight = 60;

HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("플레이어");
HWND hWndMain;
HWND hPanel;
HWND hBtnOpen, hBtnPause, hStTime, hVolume;
HWND hBtnExit, hBtnFull, hBtnMax, hBtnMin;
HWND hStage;
HWND hListWnd;

AVFormatContext* fmtCtx;
int vidx, aidx;
AVStream* vStream, * aStream;
AVCodecParameters* vPara, * aPara;
AVCodec* vCodec, * aCodec;
AVCodecContext* vCtx, * aCtx;
SwsContext* swsCtx;
AVFrame RGBFrame;
uint8_t* rgbbuf;

bool isOpen;
DWORD ThreadID;
HANDLE hPlayThread;
enum ePlayStatus { P_STOP, P_RUN, P_EXIT, P_EOF };
ePlayStatus status = ePlayStatus::P_STOP;
LARGE_INTEGER frequency;

struct sWave {
	HWAVEOUT hWaveDev;
	WAVEFORMATEX wf;
	static const int hdrnum = 10;
	static const int bufsize = 17640;
	static const int pktsize = 20000;
	WAVEHDR hdr[hdrnum];
	char samplebuf[hdrnum][bufsize];
	long availhdr;
	int nowhdr;
	char* pktbuf;
	char* pktptr;
	char* bufptr;
};
sWave wa;
SwrContext* swrCtx;
DWORD CallbackThreadID;
struct sOption {
	bool listShow = true;
	bool listRight = false;
	int listWidth = 300;
	int gap = 4;
};
sOption op;

//Main
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;

	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = lpszClass;
	WndClass.lpszMenuName = NULL;
	WndClass.style = 0;
	RegisterClass(&WndClass);

	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	WndClass.lpfnWndProc = (WNDPROC)StageWndProc;
	WndClass.lpszClassName = TEXT("Stage");
	WndClass.style = CS_DBLCLKS;
	RegisterClass(&WndClass);

	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	WndClass.lpfnWndProc = (WNDPROC)PanelWndProc;
	WndClass.lpszClassName = TEXT("Panel");
	WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	RegisterClass(&WndClass);

	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	WndClass.lpfnWndProc = (WNDPROC)ListWndProc;
	WndClass.lpszClassName = TEXT("MediaList");
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
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

//Window 프로시저
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;

	switch (iMessage) {
	case WM_CREATE:
		hWndMain = hWnd;
		InitCommonControls();
		QueryPerformanceFrequency(&frequency);
		CloseHandle(CreateThread(NULL, 0, CallbackThread, NULL, 0, &CallbackThreadID));

		//WM_CREATE에서 child 윈도우를 생성하고 WM_SIZE에서 배치한다.
		hStage = CreateWindow(TEXT("Stage"), NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
			0, 0, 0, 0, hWnd, (HMENU)NULL, g_hInst, NULL);
		hPanel = CreateWindow(TEXT("Panel"), NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
			0, 0, 0, 0, hWnd, (HMENU)NULL, g_hInst, NULL);
		hListWnd = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("MediaList"), NULL,
			WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
			0, 0, 0, 0, hWnd, (HMENU)NULL, g_hInst, NULL);
		OpenMovie(TEXT("c:\\ffstudy\\sample.mp4"));
		return 0;
	case WM_SIZE:
		Relayout(LOWORD(lParam), HIWORD(lParam));
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

LRESULT CALLBACK StageWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	switch (iMessage) {
	case WM_CREATE:
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

LRESULT CALLBACK PanelWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;

	switch (iMessage) {
	case WM_CREATE:
		hBtnExit = CreateWindow(TEXT("button"), TEXT("X"), WS_CHILD | WS_VISIBLE,
			0, 0, 0, 0, hWnd, (HMENU)ID_BTNEXIT, g_hInst, NULL);
		hBtnFull = CreateWindow(TEXT("button"), TEXT("F"), WS_CHILD | WS_VISIBLE,
			0, 0, 0, 0, hWnd, (HMENU)ID_BTNFULL, g_hInst, NULL);
		hBtnMax = CreateWindow(TEXT("button"), TEXT("M"), WS_CHILD | WS_VISIBLE,
			0, 0, 0, 0, hWnd, (HMENU)ID_BTNMAX, g_hInst, NULL);
		hBtnMin = CreateWindow(TEXT("button"), TEXT("_"), WS_CHILD | WS_VISIBLE,
			0, 0, 0, 0, hWnd, (HMENU)ID_BTNMIN, g_hInst, NULL);
		hBtnOpen = CreateWindow(TEXT("button"), TEXT("Open"), WS_CHILD | WS_VISIBLE,
			10, 5, 80, 25, hWnd, (HMENU)ID_BTNOPEN, g_hInst, NULL);
		hBtnPause = CreateWindow(TEXT("button"), TEXT("Pause"), WS_CHILD | WS_VISIBLE,
			100, 5, 80, 25, hWnd, (HMENU)ID_BTNPAUSE, g_hInst, NULL);
		hStTime = CreateWindow(TEXT("static"), TEXT(""), WS_CHILD | WS_VISIBLE,
			200, 10, 120, 25, hWnd, (HMENU)ID_STTIME, g_hInst, NULL);
		CreateWindow(TEXT("static"), TEXT("Volume"), WS_CHILD | WS_VISIBLE,
			330, 10, 60, 25, hWnd, (HMENU)-1, g_hInst, NULL);
		hVolume = CreateWindow(TRACKBAR_CLASS, NULL, WS_CHILD | WS_VISIBLE,
			390, 5, 100, 25, hWnd, (HMENU)ID_SLVOLUME, g_hInst, NULL);
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		return 0;
	case WM_SIZE:
		MoveWindow(hBtnExit, LOWORD(lParam) - 35, 5, 25, 25, TRUE);
		MoveWindow(hBtnFull, LOWORD(lParam) - 65, 5, 25, 25, TRUE);
		MoveWindow(hBtnMax, LOWORD(lParam) - 95, 5, 25, 25, TRUE);
		MoveWindow(hBtnMin, LOWORD(lParam) - 125, 5, 25, 25, TRUE);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}

LRESULT CALLBACK ListWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	switch (iMessage) {
	case WM_CREATE:
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
	hPlayThread = CreateThread(NULL, 0, PlayThread, NULL, 0, &ThreadID);
	isOpen = true;
}

//쓰레드 닫기
void CloseMovie() {
	status = P_EXIT;
	WaitForSingleObject(hPlayThread, INFINITE);
	CloseHandle(hPlayThread);
}
//쓰레드 (패킷 읽고 처리)
DWORD WINAPI PlayThread(LPVOID para) {
	int ret;
	AVPacket packet = { 0, };
	AVFrame vFrame = { 0, }, aFrame = { 0, };
	HDC hdc;
	hdc = GetDC(hStage);
	HDC MemDC;
	HBITMAP OldBitmap;
	MemDC = CreateCompatibleDC(hdc);
	double framerate = av_q2d(vStream->r_frame_rate);
	int64_t framegap = int64_t(AV_TIME_BASE / framerate);

	while (av_read_frame(fmtCtx, &packet) == 0) {
		if (status == P_EXIT) break;
		//비디오
		if (packet.stream_index == vidx) {
			ret = avcodec_send_packet(vCtx, &packet);
			if (ret != 0) { continue; }
			do {
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
				//지연
				uSleep(framegap);
			} while (true);
		}
		//오디오
		if (packet.stream_index == aidx) {
			ret = avcodec_send_packet(aCtx, &packet);
			if (ret != 0) continue;
			do {
				ret = avcodec_receive_frame(aCtx, &aFrame);
				if (ret == AVERROR(EAGAIN)) break;

				//주요변수 및 장치 초기화
				if (wa.hWaveDev == NULL) {
					wa.wf.cbSize = sizeof(WAVEFORMATEX);
					wa.wf.wFormatTag = WAVE_FORMAT_PCM;
					wa.wf.nChannels = 2;
					wa.wf.nSamplesPerSec = 44100;
					wa.wf.wBitsPerSample = 16;
					/*SetFilePointer(hFile, 22, NULL, SEEK_SET); ReadFile(hFile, &wf.nChannels, 2, &dwRead, NULL);
					SetFilePointer(hFile, 24, NULL, SEEK_SET); ReadFile(hFile, &wf.nSamplesPerSec, 4, &dwRead, NULL);
					SetFilePointer(hFile, 34, NULL, SEEK_SET); ReadFile(hFile, &wf.wBitsPerSample, 2, &dwRead, NULL);*/
					wa.wf.nBlockAlign = wa.wf.nChannels * wa.wf.wBitsPerSample / 8;
					wa.wf.nAvgBytesPerSec = wa.wf.nSamplesPerSec * wa.wf.nBlockAlign;
					waveOutOpen(&wa.hWaveDev, WAVE_MAPPER, &wa.wf,
						(DWORD_PTR)CallbackThreadID, 0, CALLBACK_THREAD);
					//두 메모리 연결
					for (int i = 0; i < wa.hdrnum; i++) {
						wa.hdr[i].lpData = wa.samplebuf[i];
					}

					wa.availhdr = wa.hdrnum;
					wa.nowhdr = 0;
					wa.bufptr = wa.samplebuf[wa.nowhdr];
					wa.pktbuf = (char*)malloc(wa.pktsize);
				}
				//리샘플러 초기화
				if (swrCtx == NULL) {
					int srate = aFrame.sample_rate;
					int channel = aFrame.channels;
					int64_t chanlay = av_get_default_channel_layout(channel);
					AVSampleFormat format = (AVSampleFormat)aFrame.format;
					swrCtx = swr_alloc_set_opts(NULL, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16,
						44100, chanlay, format, srate, 0, NULL);
					swr_init(swrCtx);
				}

				// 해당 포맷을 Wave로 리샘플링한 패킷을 읽는다.
				int sampnum = swr_convert(swrCtx, (uint8_t**)&wa.pktbuf, wa.pktsize / 4,
					(const uint8_t**)aFrame.extended_data, aFrame.nb_samples);

				// 패킷 포인터 초기화. 남은 패킷은 바이트 단위로 바꾼다.
				wa.pktptr = wa.pktbuf;
				int remainpkt = sampnum * 4;

				// 패킷을 다 쓸 때까지 반복한다.
				for (;;) {
					int remainbuf = int(wa.bufsize - (wa.bufptr - wa.samplebuf[wa.nowhdr]));

					// 패킷이 작으면 채워 넣고 다음 패킷 대기
					if (remainpkt < remainbuf) {
						memcpy(wa.bufptr, wa.pktptr, remainpkt);
						wa.bufptr += remainpkt;
						break;
					}

					// 버퍼 가득 채우고 남은 패킷 및 패킷 포인터 조정
					memcpy(wa.bufptr, wa.pktptr, remainbuf);
					remainpkt -= remainbuf;
					wa.pktptr += remainbuf;

					// 장치로 보내 재생
					wa.hdr[wa.nowhdr].dwBufferLength = wa.bufsize;
					waveOutPrepareHeader(wa.hWaveDev, &wa.hdr[wa.nowhdr], sizeof(WAVEHDR));
					waveOutWrite(wa.hWaveDev, &wa.hdr[wa.nowhdr], sizeof(WAVEHDR));

					// 헤더 수 감소시키고 남은 헤더가 생길 때까지 대기
					InterlockedDecrement(&wa.availhdr);
					while (wa.availhdr == 0) {
						if (status == P_EXIT) break;
						Sleep(20);
					}
					if (status == P_EXIT) break;

					if (++wa.nowhdr == wa.hdrnum) wa.nowhdr = 0;
					wa.bufptr = wa.samplebuf[wa.nowhdr];
				}
			} while (true);
		}
		av_packet_unref(&packet);
	}

	av_frame_unref(&vFrame);
	av_frame_unref(&aFrame);
	DeleteDC(MemDC);
	ReleaseDC(hStage, hdc);

	if (status == P_EXIT) {
		waveOutReset(wa.hWaveDev);
	}
	while (waveOutClose(wa.hWaveDev) == WAVERR_STILLPLAYING) { Sleep(20); }
	wa.hWaveDev = NULL;

	if (swrCtx != NULL) { swr_free(&swrCtx); }
	if (wa.pktbuf) { free(wa.pktbuf); wa.pktbuf = NULL; }
	if (rgbbuf) { av_free(rgbbuf); rgbbuf = NULL; }
	if (swsCtx) { sws_freeContext(swsCtx); swsCtx = NULL; }
	if (vCtx) { avcodec_free_context(&vCtx); }
	if (aCtx) { avcodec_free_context(&aCtx); }
	if (fmtCtx) { avformat_close_input(&fmtCtx); }
	isOpen = false;
	return 0;
}
//대기
void uSleep(int64_t usec) {
	LARGE_INTEGER start, end;
	int64_t elapse;

	QueryPerformanceCounter(&start);
	do {
		QueryPerformanceCounter(&end);
		elapse = (end.QuadPart - start.QuadPart) * 1000000 / frequency.QuadPart;
	} while (elapse < usec);
}
//출력 버퍼에 있는 데이터 모두 재생하면 availhdr하나 증가시킴
DWORD WINAPI CallbackThread(LPVOID para) {
	MSG Message;
	while (GetMessage(&Message, NULL, 0, 0)) {
		if (Message.message == MM_WOM_DONE) {
			if (wa.availhdr < wa.hdrnum)
				InterlockedIncrement(&wa.availhdr);
		}
	}
	return (int)Message.wParam;
}
void Relayout(int width, int height) {
	int lwidth, lgap;

	if (op.listShow) {
		lwidth = op.listWidth;
		lgap = op.gap;
		ShowWindow(hListWnd, SW_SHOW);
	}
	else {
		lwidth = 0;
		lgap = 0;
		ShowWindow(hListWnd, SW_HIDE);
	}
	if (op.listRight) {
		MoveWindow(hListWnd, width - lwidth, 0, lwidth, height, TRUE);
		MoveWindow(hStage, 0, 0, width - lwidth - lgap, height - PanelHeight, TRUE);
		MoveWindow(hPanel, 0, height - PanelHeight, width - lwidth, PanelHeight, TRUE);
	}
	else {
		MoveWindow(hListWnd, 0, 0, lwidth, height, TRUE);
		MoveWindow(hStage, lwidth + lgap, 0, width - lwidth - lgap, height - PanelHeight, TRUE);
		MoveWindow(hPanel, lwidth, height - PanelHeight, width - lwidth, PanelHeight, TRUE);
	}
}