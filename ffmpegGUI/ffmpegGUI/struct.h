#include "header.h"

//패널내의 컨트롤ID. 패널의 높이
enum {
	ID_BTNOPEN = 1, ID_BTNPAUSE, ID_STTIME, ID_SLVOLUME,
	ID_BTNEXIT, ID_BTNFULL, ID_BTNMAX, ID_BTNMIN
};
const int PanelHeight = 60;

//sWave
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

//Relayout Option
struct sOption {
	bool listShow = true;
	bool listRight = false;
	int listWidth = 300;
	int gap = 4;
};
sOption op;
//미디어 정보담을 구조체
struct sMedia {
	TCHAR Path[MAX_PATH];
	int64_t duration;
	int64_t size;
};
//PlayThread에서 재생끝날때 메세지
#define WM_MEDIA_DONE WM_USER+1