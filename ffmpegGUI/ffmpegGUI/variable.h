#include "header.h"



HINSTANCE g_hInst;
LPCTSTR lpszClass = TEXT("�÷��̾�");
HWND hWndMain;
HWND hPanel;
HWND hStage;
HWND hListWnd;
HWND hBtnOpen, hBtnPause, hStTime, hVolume;
HWND hBtnExit, hBtnFull, hBtnMax, hBtnMin;


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


//������ �������� �迭�� �ٷ�� ���� Ŭ����
class MediaArray {
public:
	sMedia* ar;
	int size;
	int num;
	TCHAR name[64];
	int nowsel;

	MediaArray() { Init(); }
	~MediaArray() { Uninit(); }
	void Init() {
		size = 20;
		num = 0;
		ar = (sMedia*)malloc(size * sizeof(sMedia));
		lstrcpy(name, TEXT(""));
		nowsel = -1;
	}
	void Uninit() {
		if (ar) {
			free(ar);
			ar = NULL;
		}
	}
	void Insert(int idx, sMedia value) {
		int need;

		need = num + 1;
		if (need > size) {
			size = need + 10;
			ar = (sMedia*)realloc(ar, size * sizeof(sMedia));
		}
		memmove(ar + idx + 1, ar + idx, (num - idx) * sizeof(sMedia));
		ar[idx] = value;
		num++;
	}
	void Delete(int idx) {
		memmove(ar + idx, ar + idx + 1, (num - idx - 1) * sizeof(sMedia));
		num--;
	}
	void Append(sMedia value) { Insert(num, value); }
	void Clear() { Uninit(); Init(); }
	sMedia& operator [](int idx) { return ar[idx]; }
};
//���,����ð�,���� ũ��
const int maxlist = 10;
int listnum = 1;
int nowlist = 0;
MediaArray arMedia[maxlist];
//����Ʈ �ڵ� ����
HWND hList, hListTab;
HWND hBtnListMenu, hBtnListAdd, hBtnListRemove, hBtnListDel;
enum { ID_BTNLISTMENU = 101, ID_BTNLISTADD, ID_BTNLISTREMOVE, ID_BTNLISTDEL };


//���� ���� Ŭ����
class SettingFile {
public:
	TCHAR IniFile[MAX_PATH];

	void SetIniFile(LPCTSTR Path) {
		lstrcpy(IniFile, Path);
	}
	int Read(LPCTSTR app, LPCTSTR keyfmt, int defvalue, ...) {
		TCHAR key[1024];
		va_list args;
		va_start(args, defvalue);
		_vstprintf_s(key, 1024, keyfmt, args);
		return GetPrivateProfileInt(app, key, defvalue, IniFile);
	}
	// ���� ���̴� �ּ� 260 �Ǵ� Ȯ���� ���� ���� �ڽ� �־�� ��. 
	void Read(LPCTSTR app, LPCTSTR keyfmt, LPCTSTR defvalue, LPTSTR ret, ...) {
		TCHAR key[1024];
		va_list args;
		va_start(args, ret);
		_vstprintf_s(key, 1024, keyfmt, args);
		GetPrivateProfileString(app, key, defvalue, ret, MAX_PATH, IniFile);
	}
	void Write(LPCTSTR app, LPCTSTR keyfmt, int value, ...) {
		TCHAR key[1024];
		va_list args;
		va_start(args, value);
		_vstprintf_s(key, 1024, keyfmt, args);
		TCHAR tValue[128];
		wsprintf(tValue, TEXT("%d"), value);
		Write(app, key, tValue);
	}
	void Write(LPCTSTR app, LPCTSTR keyfmt, LPCTSTR value, ...) {
		TCHAR key[1024];
		va_list args;
		va_start(args, value);
		_vstprintf_s(key, 1024, keyfmt, args);
		WritePrivateProfileString(app, key, value, IniFile);
	}
};
SettingFile setting;
WINDOWPLACEMENT wndpl;

//���Ұ�, ���� ������ ����
bool isMute;
DWORD oldVolume;
//��, Ű����� ���� ���� ����
enum eAccel {
	A_SPACE, A_LEFT, A_RIGHT, A_UP, A_DOWN, A_PRIOR, A_NEXT, A_BACK,
	A_A, A_Q, A_M, A_W, A_S, A_RETURN, A_ESC
};
ACCEL arAccel[] = {
	{ FVIRTKEY, VK_SPACE, A_SPACE },
	{ FVIRTKEY, VK_LEFT, A_LEFT },
	{ FVIRTKEY | FCONTROL, VK_LEFT, A_LEFT },
	{ FVIRTKEY | FSHIFT, VK_LEFT, A_LEFT },
	{ FVIRTKEY, VK_RIGHT, A_RIGHT },
	{ FVIRTKEY | FCONTROL, VK_RIGHT, A_RIGHT },
	{ FVIRTKEY | FSHIFT, VK_RIGHT, A_RIGHT },
	{ FVIRTKEY, VK_UP, A_UP },
	{ FVIRTKEY, VK_DOWN, A_DOWN },
	{ FVIRTKEY, VK_PRIOR, A_PRIOR },
	{ FVIRTKEY, VK_NEXT, A_NEXT },
	{ FVIRTKEY, VK_BACK, A_BACK },
	{ FVIRTKEY, 'Q', A_Q },
	{ FVIRTKEY, 'A', A_A },
	{ FVIRTKEY, 'W', A_W },
	{ FVIRTKEY, 'S', A_S },
	{ FVIRTKEY, 'M', A_M },
	{ FVIRTKEY, VK_RETURN, A_RETURN },
	{ FVIRTKEY, VK_ESCAPE, A_ESC },
};
//�迭�� ��� ���� ���� ��ũ��
#define ARSIZE(ar) (sizeof(ar)/sizeof(ar[0]))
//������ �����ֱ� ���� ����
TCHAR overlayMsg[128];
UINT overlayMsgHpos;
UINT overlayMsgVpos;
ULONGLONG overlayMsgTimout;