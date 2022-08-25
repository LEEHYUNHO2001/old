#include "header.h"

//�гγ��� ��Ʈ��ID. �г��� ����
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
//�̵�� �������� ����ü
struct sMedia {
	TCHAR Path[MAX_PATH];
	int64_t duration;
	int64_t size;
};
//PlayThread���� ��������� �޼���
#define WM_MEDIA_DONE WM_USER+1