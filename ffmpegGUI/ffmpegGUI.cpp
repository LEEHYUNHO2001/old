//video -> 비트맵, 스레드, software scale, winapi(32)구조, usleep 등 사용
//audio -> Waveform
//UI -> 윈도우 분할, Open, 윈도우 크기 변경, 글 목록, 드래그(Stage, List),
#include "header.h"
#include "struct.h"
#include "function.h"
#include "variable.h"

//Main
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	HWND hWnd;
	MSG Message;
	WNDCLASS WndClass;
	g_hInst = hInstance;
	//WndProc
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
	//StageWndProc
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	WndClass.lpfnWndProc = (WNDPROC)StageWndProc;
	WndClass.lpszClassName = TEXT("Stage");
	WndClass.style = CS_DBLCLKS;
	RegisterClass(&WndClass);
	//PanelWndProc
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	WndClass.lpfnWndProc = (WNDPROC)PanelWndProc;
	WndClass.lpszClassName = TEXT("Panel");
	WndClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	RegisterClass(&WndClass);
	//ListWndProc
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
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	static bool bFirstActivate = true;

	switch (iMessage) {
	case WM_CREATE:
		hWndMain = hWnd;
		InitCommonControls();
		QueryPerformanceFrequency(&frequency);
		CloseHandle(CreateThread(NULL, 0, CallbackThread, NULL, 0, &CallbackThreadID));
		//child 윈도우 만들기전에 읽어오기
		TCHAR inipath[MAX_PATH];
		GetModuleFileName(NULL, inipath, MAX_PATH);
		PathRemoveFileSpec(inipath);
		lstrcat(inipath, TEXT("\\setting.ini"));
		setting.SetIniFile(inipath);
		LoadOption();
		//WM_CREATE에서 child 윈도우를 생성하고 WM_SIZE에서 배치한다.
		hStage = CreateWindow(TEXT("Stage"), NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
			0, 0, 0, 0, hWnd, (HMENU)NULL, g_hInst, NULL);
		hPanel = CreateWindow(TEXT("Panel"), NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
			0, 0, 0, 0, hWnd, (HMENU)NULL, g_hInst, NULL);
		hListWnd = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("MediaList"), NULL,
			WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
			0, 0, 0, 0, hWnd, (HMENU)NULL, g_hInst, NULL);
		return 0;
	case WM_ACTIVATEAPP:
		if (wParam == TRUE) {
			if (bFirstActivate) {
				bFirstActivate = false;
				SetWindowPlacement(hWndMain, &wndpl);
				TabCtrl_SetCurSel(hListTab, nowlist);
				FillList(nowlist);
				if (arMedia[nowlist].nowsel != -1) {
					ChangeMedia(nowlist, arMedia[nowlist].nowsel);
				}
				else {
#ifdef _DEBUG
					OpenMovie(TEXT("c:\\ffstudy\\sample1.mp4"));
#endif
				}
			}
		}
		return 0;
	case WM_SIZE:
		Relayout(LOWORD(lParam), HIWORD(lParam));
		return 0;
	case WM_MEDIA_DONE:
		if (arMedia[nowlist].nowsel < arMedia[nowlist].num - 1) {
			ChangeMedia(nowlist, arMedia[nowlist].nowsel + 1);
		}
		return 0;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		return 0;
	case WM_DESTROY:
		SaveOption();
		CloseMovie();
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}
//StageWndProc
LRESULT CALLBACK StageWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	switch (iMessage) {
	case WM_CREATE:
		DragAcceptFiles(hWnd, TRUE);
		return 0;
		//DropFile 받는 대상에 추가
	case WM_DROPFILES:
		DropFiles((HDROP)wParam, true);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}
//PanelWndProc
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

		SendMessage(hVolume, TBM_SETRANGE, FALSE, MAKELPARAM(0, 100));
		DWORD voldevice, vol;
		waveOutGetVolume(wa.hWaveDev, &voldevice);
		vol = DWORD(LOWORD(voldevice) * 100.0 / 0xffff + 0.5);
		SendMessage(hVolume, TBM_SETPOS, TRUE, vol);
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
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_BTNOPEN:
			OpenMediaFile(true);
			break;
		}
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}
//ListWndProc
LRESULT CALLBACK ListWndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	TCITEM tie;
	LVCOLUMN COL;

	switch (iMessage) {
	case WM_CREATE:
		//드래그 사용
		DragAcceptFiles(hWnd, TRUE);
		//목록 탭
		hListTab = CreateWindow(WC_TABCONTROL, TEXT(""), WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
			0, 0, 0, 0, hWnd, (HMENU)0, g_hInst, NULL);
		hBtnListMenu = CreateWindow(TEXT("button"), TEXT("M"), WS_CHILD | WS_VISIBLE,
			0, 0, 0, 0, hWnd, (HMENU)ID_BTNLISTMENU, g_hInst, NULL);
		tie.mask = TCIF_TEXT;
		for (int i = 0; i < listnum; i++) {
			tie.pszText = arMedia[i].name;
			TabCtrl_InsertItem(hListTab, i, &tie);
		}
		//파일 정보 UI
		hList = CreateWindow(WC_LISTVIEW, NULL, WS_CHILD | WS_VISIBLE |
			LVS_REPORT | LVS_SHOWSELALWAYS, 0, 0, 0, 0, hWnd, NULL, g_hInst, NULL);
		ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT);
		COL.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		COL.fmt = LVCFMT_LEFT;
		COL.cx = 200;
		COL.pszText = (TCHAR*)TEXT("이름");
		COL.iSubItem = 0;
		SendMessage(hList, LVM_INSERTCOLUMN, 0, (LPARAM)&COL);

		COL.cx = 50;
		COL.pszText = (TCHAR*)TEXT("시간");
		COL.iSubItem = 1;
		SendMessage(hList, LVM_INSERTCOLUMN, 1, (LPARAM)&COL);

		COL.cx = 50;
		COL.pszText = (TCHAR*)TEXT("크기");
		COL.iSubItem = 2;
		SendMessage(hList, LVM_INSERTCOLUMN, 2, (LPARAM)&COL);
		//+ - D 버튼
		hBtnListAdd = CreateWindow(TEXT("button"), TEXT("+"), WS_CHILD | WS_VISIBLE,
			0, 0, 0, 0, hWnd, (HMENU)ID_BTNLISTADD, g_hInst, NULL);
		hBtnListRemove = CreateWindow(TEXT("button"), TEXT("-"), WS_CHILD | WS_VISIBLE,
			0, 0, 0, 0, hWnd, (HMENU)ID_BTNLISTREMOVE, g_hInst, NULL);
		hBtnListDel = CreateWindow(TEXT("button"), TEXT("D"), WS_CHILD | WS_VISIBLE,
			0, 0, 0, 0, hWnd, (HMENU)ID_BTNLISTDEL, g_hInst, NULL);
		return 0;
	case WM_SIZE:
		//버튼 위치
		MoveWindow(hListTab, 0, 0, LOWORD(lParam) - 30, 30, TRUE);
		MoveWindow(hBtnListMenu, LOWORD(lParam) - 30, 0, 30, 30, TRUE);
		MoveWindow(hList, 0, 30, LOWORD(lParam), HIWORD(lParam) - 65, TRUE);
		MoveWindow(hBtnListAdd, 5, HIWORD(lParam) - 30, 25, 25, TRUE);
		MoveWindow(hBtnListRemove, 35, HIWORD(lParam) - 30, 25, 25, TRUE);
		MoveWindow(hBtnListDel, 65, HIWORD(lParam) - 30, 25, 25, TRUE);
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			//M 버튼
		case ID_BTNLISTMENU:
			HMENU hPopup;
			hPopup = CreatePopupMenu();
			AppendMenu(hPopup, MF_STRING, 40001, TEXT("추가"));
			AppendMenu(hPopup, MF_STRING, 40002, TEXT("삭제"));
			AppendMenu(hPopup, MF_STRING, 40003, TEXT("이름 변경"));
			UINT id;
			POINT pt;
			GetCursorPos(&pt);
			id = TrackPopupMenu(hPopup, TPM_LEFTALIGN | TPM_RETURNCMD,
				pt.x, pt.y, 0, hWnd, NULL);
			switch (id) {
			case 40001:
				//탭 10개까지만 생성 가능
				if (listnum < maxlist) {
					listnum++;
					nowlist = TabCtrl_GetItemCount(hListTab);
					arMedia[nowlist].Clear();
					ListView_DeleteAllItems(hList);
					tie.mask = TCIF_TEXT;
					lstrcpy(arMedia[nowlist].name, TEXT("새목록"));
					tie.pszText = arMedia[nowlist].name;
					TabCtrl_InsertItem(hListTab, nowlist, &tie);
					TabCtrl_SetCurSel(hListTab, nowlist);
				}
				break;
			case 40002:
				if (listnum > 1) {
					arMedia[nowlist].Uninit();
					for (int i = nowlist + 1; i < listnum; i++) {
						arMedia[i - 1] = arMedia[i];
					}
					listnum--;
					arMedia[listnum].ar = NULL;
					TabCtrl_DeleteItem(hListTab, nowlist);
					if (nowlist >= listnum) {
						nowlist--;
					}
					TabCtrl_SetCurSel(hListTab, nowlist);
					FillList(nowlist);
				}
				break;
			case 40003:
				TCHAR newName[64];
				lstrcpy(newName, TEXT("바꾼이름"));
				lstrcpy(arMedia[nowlist].name, newName);
				tie.mask = TCIF_TEXT;
				tie.pszText = newName;
				TabCtrl_SetItem(hListTab, nowlist, &tie);
				break;
			}
			DestroyMenu(hPopup);
			break;
		case ID_BTNLISTADD:
			OpenMediaFile(false);
			break;
		case ID_BTNLISTREMOVE:
		case ID_BTNLISTDEL:
			int idx;
			idx = ListView_GetNextItem(hList, -1, LVNI_ALL | LVNI_SELECTED);
			while (idx != -1) {
				ListView_DeleteItem(hList, idx);
				arMedia[nowlist].Delete(idx);
				if (LOWORD(lParam) == ID_BTNLISTDEL) {
					// 휴지통으로 보낼 것
				}
				idx = ListView_GetNextItem(hList, idx - 1, LVNI_ALL | LVNI_SELECTED);
			}
			break;
		}
		return 0;
	case WM_DROPFILES:
		DropFiles((HDROP)wParam, false);
		return 0;
	case WM_NOTIFY:
		LPNMHDR hdr;
		hdr = (LPNMHDR)lParam;

		if (hdr->hwndFrom == hList) {
			switch (hdr->code) {
			case NM_DBLCLK:
				LPNMITEMACTIVATE nia;
				nia = (LPNMITEMACTIVATE)lParam;
				ChangeMedia(nowlist, nia->iItem);
			}
		}
		if (hdr->hwndFrom == hListTab) {
			switch (hdr->code) {
			case TCN_SELCHANGE:
				nowlist = TabCtrl_GetCurSel(hListTab);
				FillList(nowlist);
				break;
			}
		}
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}
//코덱 오픈
void OpenMovie(LPCTSTR movie) {
	char MoviePathAnsi[MAX_PATH];
	//재생중인 파일 중지.
	TCHAR Title[MAX_PATH + 10];
	if (isOpen) {
		CloseMovie();
	}
	status = P_STOP;

	WideCharToMultiByte(CP_ACP, 0, movie, -1, MoviePathAnsi, MAX_PATH, NULL, NULL);

	int ret = avformat_open_input(&fmtCtx, MoviePathAnsi, NULL, NULL);
	if (ret != 0) {
		isOpen = false;
		MessageBox(hWndMain, TEXT("동영상 파일이 없습니다."), TEXT("알림"), MB_OK);
		return;
	}
	//hWndMain 설정
	wsprintf(Title, TEXT("%s - %s"), lpszClass, movie);
	SetWindowText(hWndMain, Title);

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
	if (vidx >= 0) {
		AdjustWindowSizePos(vPara->width, vPara->height);
	}
	else {
		AdjustWindowSizePos(500, 300);
		InvalidateRect(hWndMain, NULL, TRUE);
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
	double framerate;
	//framerate가 오디오일수도 있으므로 에러처리
	if (vidx == 0) {
		framerate = av_q2d(vStream->r_frame_rate);
	}
	else {
		framerate = av_q2d(aStream->r_frame_rate);
	}
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
	//정상으로 끝나면 메세지 보냄. 아니면 P_STOP
	if (status == P_EOF) {
		PostMessage(hWndMain, WM_MEDIA_DONE, 0, 0);
	}
	status = P_STOP;
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
//레이아웃
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
//파일 오픈, + 버튼
void OpenMediaFile(bool reset) {
	OPENFILENAME OFN;
	TCHAR* MoviePath;
	TCHAR* p;
	TCHAR Dir[MAX_PATH];
	TCHAR FilePath[MAX_PATH];

	memset(&OFN, 0, sizeof(OPENFILENAME));
	OFN.lStructSize = sizeof(OPENFILENAME);
	OFN.hwndOwner = hWndMain;
	OFN.lpstrFilter = TEXT("모든 파일(*.*)\0*.*\0");
	MoviePath = (TCHAR*)calloc(MAX_PATH, 1000);
	OFN.lpstrFile = MoviePath;
	OFN.nMaxFile = MAX_PATH * 100;
	OFN.Flags = OFN_EXPLORER | OFN_ALLOWMULTISELECT;
	if (GetOpenFileName(&OFN) == 0) {
		return;
	}

	p = MoviePath;
	if (reset) {
		arMedia[nowlist].Clear();
		ListView_DeleteAllItems(hList);
	}
	lstrcpy(Dir, MoviePath);
	p = p + lstrlen(Dir) + 1;

	if (*p == 0) {
		AppendMedia(nowlist, Dir, true);
	}
	else {
		for (; *p; p += lstrlen(p) + 1) {
			lstrcpy(FilePath, Dir);
			lstrcat(FilePath, TEXT("\\"));
			lstrcat(FilePath, p);
			AppendMedia(nowlist, FilePath, true);
		}
	}
	free(MoviePath);

	if (reset) {
		if (arMedia[nowlist].num != 0) {
			ChangeMedia(nowlist, 0);
		}
	}
}
//윈도우 크기 조정
void AdjustWindowSizePos(int width, int height) {
	RECT crt, wrt;

	if (IsZoomed(hWndMain)) {
		return;
	}

	if (op.listShow) {
		SetRect(&crt, 0, 0, width + op.listWidth + op.gap, height + PanelHeight);
	}
	else {
		SetRect(&crt, 0, 0, width, height + PanelHeight);
	}
	AdjustWindowRect(&crt, WS_OVERLAPPEDWINDOW, FALSE);
	GetWindowRect(hWndMain, &wrt);
	wrt.right = wrt.left + (crt.right - crt.left);
	wrt.bottom = wrt.top + (crt.bottom - crt.top);

	//모니터
	/*HMONITOR hMon = MonitorFromRect(&wrt, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO mi = { sizeof(MONITORINFO), };
	GetMonitorInfo(hMon, &mi);
	if (wrt.right - wrt.left > mi.rcWork.right - mi.rcWork.left) {
		wrt.right = wrt.left + mi.rcWork.right - mi.rcWork.left;
	}
	if (wrt.bottom - wrt.top > mi.rcWork.bottom - mi.rcWork.top) {
		wrt.bottom = wrt.top + mi.rcWork.bottom - mi.rcWork.top;
	}
	int xdiff = wrt.right - mi.rcWork.right;
	if (xdiff > 0) {
		wrt.left -= xdiff;
		wrt.right -= xdiff;
	}
	int ydiff = wrt.bottom - mi.rcWork.bottom;
	if (ydiff > 0) {
		wrt.top -= ydiff;
		wrt.bottom -= ydiff;
	}*/

	SetWindowPos(hWndMain, NULL, wrt.left, wrt.top, wrt.right - wrt.left, wrt.bottom - wrt.top,
		SWP_NOZORDER);
}
//동영상 점검 후 정보 sMedia 구조체에 채움.
bool GetMediaInfo(TCHAR* Path, sMedia* pMedia) {
	char MoviePathAnsi[MAX_PATH];
	AVFormatContext* t_fmtCtx = NULL;
	HANDLE hFile;
	LARGE_INTEGER size;

	WideCharToMultiByte(CP_ACP, 0, Path, -1, MoviePathAnsi, MAX_PATH, NULL, NULL);
	int ret = avformat_open_input(&t_fmtCtx, MoviePathAnsi, NULL, NULL);
	if (ret != 0) {
		return false;
	}
	lstrcpy(pMedia->Path, Path);
	avformat_find_stream_info(t_fmtCtx, NULL);
	pMedia->duration = t_fmtCtx->duration;
	avformat_close_input(&t_fmtCtx);

	hFile = CreateFile(Path, 0, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		return FALSE;
	}
	GetFileSizeEx(hFile, &size);
	pMedia->size = size.QuadPart;
	CloseHandle(hFile);

	return true;
}
//목록에 파일 추가
bool AppendMedia(int list, TCHAR* MediaPath, bool tolist) {
	sMedia movie;

	if (GetMediaInfo(MediaPath, &movie)) {
		AppendMedia(list, movie, tolist);
		return true;
	}
	return false;
}
void AppendMedia(int list, sMedia media, bool tolist) {
	arMedia[list].Append(media);

	if (tolist) {
		AppendMediaToList(media);
	}
}
void AppendMediaToList(sMedia media) {
	LVITEM LI;
	TCHAR buf[32];

	LI.mask = LVIF_TEXT;
	//경로
	LI.iSubItem = 0;
	LI.iItem = ListView_GetItemCount(hList);
	LI.pszText = PathFindFileName(media.Path);
	SendMessage(hList, LVM_INSERTITEM, 0, (LPARAM)&LI);
	//시간
	LI.iSubItem = 1;
	int sec = int(media.duration / AV_TIME_BASE);
	if (sec >= 3600) {
		wsprintf(buf, TEXT("%d:%d:%d"), sec / 3600, sec % 3600 / 60, sec % 60);
	}
	else {
		wsprintf(buf, TEXT("%d:%d"), sec / 60, sec % 60);
	}
	LI.pszText = buf;
	SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&LI);
	//크기
	LI.iSubItem = 2;
	wsprintf(buf, TEXT("%dM"), int(media.size / 1000000));
	LI.pszText = buf;
	SendMessage(hList, LVM_SETITEM, 0, (LPARAM)&LI);
}
//파일 관련 기능 -> 쉘 사용 -> arMedia 목록 전체를 리스트 뷰에 채움.
void FillList(int list) {
	ListView_DeleteAllItems(hList);
	for (int i = 0; i < arMedia[list].num; i++) {
		AppendMediaToList(arMedia[list][i]);
	}
}
//리스트의 idx 번째 항목 열기
void ChangeMedia(int list, int idx) {
	arMedia[list].nowsel = idx;
	ListView_SetItemState(hList, -1, 0, LVIS_FOCUSED | LVIS_SELECTED);
	ListView_SetItemState(hList, idx, LVIS_FOCUSED | LVIS_SELECTED,
		LVIS_FOCUSED | LVIS_SELECTED);
	OpenMovie(arMedia[list][arMedia[list].nowsel].Path);
}
//파일 드래그
void DropFiles(HDROP hDrop, bool Reset) {
	int count;
	TCHAR MoviePath[MAX_PATH] = TEXT("");

	if (Reset) {
		arMedia[nowlist].Clear();
		ListView_DeleteAllItems(hList);
	}

	count = DragQueryFile(hDrop, 0xffffffff, NULL, 0);
	for (int i = 0; i < count; i++) {
		DragQueryFile(hDrop, i, MoviePath, MAX_PATH);
		AppendMedia(nowlist, MoviePath, true);
	}

	if (Reset) {
		if (arMedia[nowlist].num != 0) {
			ChangeMedia(nowlist, 0);
			SetForegroundWindow(hWndMain);
		}
	}
}
//옵션 불러오기
void LoadOption() {
	TCHAR tApp[128];
	TCHAR tValue[128];
	sMedia media;

	wndpl.length = sizeof(WINDOWPLACEMENT);
	wndpl.flags = 0;
	wndpl.showCmd = setting.Read(TEXT("Position"), TEXT("showCmd"), SW_RESTORE);
	if (wndpl.showCmd == SW_SHOWMINIMIZED) {
		wndpl.showCmd = SW_RESTORE;
	}
	wndpl.rcNormalPosition.left = setting.Read(TEXT("Position"), TEXT("left"), 100);
	wndpl.rcNormalPosition.top = setting.Read(TEXT("Position"), TEXT("top"), 100);
	wndpl.rcNormalPosition.right = setting.Read(TEXT("Position"), TEXT("right"), 1024);
	wndpl.rcNormalPosition.bottom = setting.Read(TEXT("Position"), TEXT("bottom"), 580);
	wndpl.ptMinPosition.x = wndpl.ptMinPosition.y = 0;
	wndpl.ptMaxPosition.x = wndpl.ptMaxPosition.y = 0;

	listnum = setting.Read(TEXT("List"), TEXT("listnum"), 1);
	nowlist = setting.Read(TEXT("List"), TEXT("nowList"), 0);
	for (int list = 0; list < listnum; list++) {
		wsprintf(tApp, TEXT("List%d"), list);
		int num = setting.Read(tApp, TEXT("num"), 0);
		setting.Read(tApp, TEXT("name"), TEXT("목록"), arMedia[list].name);
		arMedia[list].nowsel = setting.Read(tApp, TEXT("nowsel"), -1);
		for (int i = 0; i < num; i++) {
			setting.Read(tApp, TEXT("Name%d"), TEXT(""), media.Path, i);
			setting.Read(tApp, TEXT("Size%d"), TEXT(""), tValue, i);
			media.size = _tcstoll(tValue, NULL, 10);
			setting.Read(tApp, TEXT("Duration%d"), TEXT(""), tValue, i);
			media.duration = _tcstoll(tValue, NULL, 10);

			AppendMedia(list, media, false);
		}
	}
}
//옵션 저장
void SaveOption() {
	TCHAR tApp[128];
	TCHAR tValue[128];
	sMedia media;

	wndpl.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(hWndMain, &wndpl);
	setting.Write(TEXT("Position"), TEXT("showCmd"), wndpl.showCmd);
	setting.Write(TEXT("Position"), TEXT("left"), wndpl.rcNormalPosition.left);
	setting.Write(TEXT("Position"), TEXT("top"), wndpl.rcNormalPosition.top);
	setting.Write(TEXT("Position"), TEXT("right"), wndpl.rcNormalPosition.right);
	setting.Write(TEXT("Position"), TEXT("bottom"), wndpl.rcNormalPosition.bottom);

	setting.Write(TEXT("List"), TEXT("listnum"), listnum);
	setting.Write(TEXT("List"), TEXT("nowlist"), nowlist);
	for (int list = 0; list < listnum; list++) {
		wsprintf(tApp, TEXT("List%d"), list);
		int num = arMedia[list].num;
		setting.Write(tApp, TEXT("num"), num);
		setting.Write(tApp, TEXT("name"), arMedia[list].name);
		setting.Write(tApp, TEXT("nowsel"), arMedia[list].nowsel);
		for (int i = 0; i < num; i++) {
			media = arMedia[list][i];
			setting.Write(tApp, TEXT("Name%d"), media.Path, i);
			wsprintf(tValue, TEXT("%I64d"), media.size);
			setting.Write(tApp, TEXT("Size%d"), tValue, i);
			wsprintf(tValue, TEXT("%I64d"), media.duration);
			setting.Write(tApp, TEXT("Duration%d"), tValue, i);
		}
	}
}
