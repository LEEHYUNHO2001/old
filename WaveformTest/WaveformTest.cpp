#include <stdio.h>
#include <conio.h>
#include <windows.h>

#pragma comment(lib, "winmm.lib")

HWAVEOUT hWaveDev;
HANDLE hFile;
DWORD dwRead;
WAVEFORMATEX wf;
const int hdrnum = 3;
const int bufsize = 10000;
long availhdr = hdrnum;
int nowhdr = 0;



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
    WAVEHDR* hdr;
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
    //헤더와 버퍼 한번에 할당
    hdr = (WAVEHDR*)calloc(hdrnum, sizeof(WAVEHDR) + bufsize);
    char* bufstart = (char*)hdr + sizeof(WAVEHDR) * hdrnum;
    for (int i = 0; i < hdrnum; i++) {
        hdr[i].lpData = bufstart + bufsize * i;
    }
    // 파일을 다 읽을 때까지 루프를 돈다.
    for (;;) {
        ReadFile(hFile, hdr[nowhdr].lpData, bufsize, &dwRead, NULL);
        if (dwRead == 0) break;
        hdr[nowhdr].dwBufferLength = dwRead;
        // 출력 장치로 전송
        waveOutPrepareHeader(hWaveDev, &hdr[nowhdr], sizeof(WAVEHDR));
        waveOutWrite(hWaveDev, &hdr[nowhdr], sizeof(WAVEHDR));
        // 가용 버퍼 하나 감소
        InterlockedDecrement(&availhdr);
        // 가용 버퍼가 없으면 풀릴 때까지 대기
        while (availhdr == 0) Sleep(20);
        // 다음 버퍼 위치로 이동
        if (++nowhdr == hdrnum) nowhdr = 0;
    }
    for (int i = 0; i < hdrnum; i++) {
        waveOutUnprepareHeader(hWaveDev, &hdr[i], sizeof(WAVEHDR));
    }
    while (waveOutClose(hWaveDev) == WAVERR_STILLPLAYING) { Sleep(10); }
    CloseHandle(hFile);
    return 0;
}

int main() {
    PlayWave(TEXT("c:\\ffstudy\\techno.wav"));
}