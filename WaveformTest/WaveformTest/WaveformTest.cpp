#include <stdio.h>
#include <windows.h>

#pragma comment(lib, "winmm.lib")

HWAVEOUT hWaveDev;
HANDLE hFile;
DWORD dwRead;
WAVEFORMATEX wf;
const int hdrnum = 3;
const int bufsize = 3000;
const int pktsize = 2000;
WAVEHDR hdr[hdrnum];
char samplebuf[hdrnum][bufsize];
long availhdr = hdrnum;
int nowhdr = 0;
char pktbuf[pktsize];
char* pktptr;
char* bufptr;


void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    // 재생이 끝나면 대기를 풀어준다.
    if (uMsg == WOM_DONE) {
        InterlockedIncrement(&availhdr);
    }
}

int PlayWave(LPCTSTR song){
    /*
    //사운드 출력 장치 조사
    UINT wavenum;
    char devname[128];
    wavenum = waveOutGetNumDevs();
    printf("장치 개수 = %d\n", wavenum);
    WAVEOUTCAPS cap;
    for (UINT i = 0; i < wavenum; i++) {
        waveOutGetDevCaps(i, &cap, sizeof(WAVEOUTCAPS));
        WideCharToMultiByte(CP_ACP, 0, cap.szPname, -1, devname, 128, NULL, NULL);
        printf("%d번 : %d 채널,지원 포맷=%x,기능=%x,이름=%s\n",
            i, cap.wChannels, cap.dwFormats, cap.dwSupport, devname);
    }
    */

    // 웨이브 파일을 연다.
    hFile = CreateFile(song, GENERIC_READ, FILE_SHARE_READ, NULL,
       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    //에러처리
    if (hFile == INVALID_HANDLE_VALUE) {
        puts("file not found");
        return -1;
    }
    // 재생 장치를 연다.
    wf.cbSize = sizeof(WAVEFORMATEX);
    wf.wFormatTag = WAVE_FORMAT_PCM;
    SetFilePointer(hFile, 22, NULL, SEEK_SET); ReadFile(hFile, &wf.nChannels, 2, &dwRead, NULL);
    SetFilePointer(hFile, 24, NULL, SEEK_SET); ReadFile(hFile, &wf.nSamplesPerSec, 4, &dwRead, NULL);
    SetFilePointer(hFile, 34, NULL, SEEK_SET); ReadFile(hFile, &wf.wBitsPerSample, 2, &dwRead, NULL);
    wf.nBlockAlign = wf.nChannels * wf.wBitsPerSample / 8;
    wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
    waveOutOpen(&hWaveDev, WAVE_MAPPER, &wf, (DWORD)waveOutProc, 0, CALLBACK_FUNCTION);
    // 헤더는 건너 뛰고 버퍼에 샘플 데이터를 읽어들인다.
    SetFilePointer(hFile, 44, NULL, SEEK_SET);

    //ipData멤버로 두 메모리 연결
    for (int i = 0; i < hdrnum; i++) {
        hdr[i].lpData = samplebuf[i];
    }
    // 샘플 버퍼 포인터 초기화.
    bufptr = samplebuf[nowhdr];

    // 파일을 다 읽을 때까지 반복
    for (;;) {
        ReadFile(hFile, pktbuf, pktsize, &dwRead, NULL);

        // 파일 끝 도달했으면 나머지 샘플 데이터 보내고 종료한다.
        if (dwRead == 0) {
            hdr[nowhdr].dwBufferLength = bufptr - samplebuf[nowhdr];
            waveOutPrepareHeader(hWaveDev, &hdr[nowhdr], sizeof(WAVEHDR));
            waveOutWrite(hWaveDev, &hdr[nowhdr], sizeof(WAVEHDR));
            InterlockedDecrement(&availhdr);
            break;
        }
        // 패킷 포인터 초기화. 남은 패킷 초기화
        pktptr = pktbuf;
        int remainpkt = dwRead;

        // 패킷 하나를 다 쓸 때까지 반복한다.
        for (;;) {
            int remainbuf = bufsize - (bufptr - samplebuf[nowhdr]);

            // 버퍼 남은양보다 패킷이 더 작으면 채워 넣고 버퍼 포인터 이동 후 파일 읽기 루틴으로 돌아간다.
            if (remainpkt < remainbuf) {
                memcpy(bufptr, pktptr, remainpkt);
                bufptr += remainpkt;
                break;
            }
            // 패킷이 더 많으면 남은 버퍼를 가득 채운다.
            memcpy(bufptr, pktptr, remainbuf);
            // 쓴만큼 패킷은 감소하고 버퍼 포인터는 뒤로 이동한다.
            remainpkt -= remainbuf;
            pktptr += remainbuf;
            //버퍼 가득 채운 해더 장비로 보내고 다음헤더 이동
            hdr[nowhdr].dwBufferLength = bufsize;
            waveOutPrepareHeader(hWaveDev, &hdr[nowhdr], sizeof(WAVEHDR));
            waveOutWrite(hWaveDev, &hdr[nowhdr], sizeof(WAVEHDR));
            InterlockedDecrement(&availhdr);
            while (availhdr == 0) Sleep(20);

            // 다음 버퍼로 이동하고 버퍼 포인터를 초기화한다.
            if (++nowhdr == hdrnum) nowhdr = 0;
            bufptr = samplebuf[nowhdr];
        }
    }
    while (waveOutClose(hWaveDev) == WAVERR_STILLPLAYING) { Sleep(10); }
    CloseHandle(hFile);
    return 0;
}

int main() {
    PlayWave(TEXT("c:\\ffstudy\\takeonme.wav"));
}