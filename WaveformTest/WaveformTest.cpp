#include <stdio.h>
#include <conio.h>
#include <windows.h>

#pragma comment(lib, "winmm.lib")

HWAVEOUT hWaveDev;
HANDLE hFile;
DWORD dwRead;
char* samplebuf;
DWORD bufsize;
WAVEFORMATEX wf;
WAVEHDR hdr = { NULL, };

// 대기를 위한 변수
bool bWait = true;

void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2) {
    // 재생이 끝나면 대기를 풀어준다.
    if (uMsg == WOM_DONE) {
        bWait = false;
    }
}

int main() {
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

    // 웨이브 파일을 연다.
    hFile = CreateFile(TEXT("c:\\ffstudy\\techno.wav"), GENERIC_READ, FILE_SHARE_READ, NULL,
       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    //에러처리
    if (hFile == INVALID_HANDLE_VALUE) {
        puts("file not found");
        return -1;
    }
    // 재생 장치를 연다.
    wf.cbSize = sizeof(WAVEFORMATEX);
    wf.wFormatTag = WAVE_FORMAT_PCM;
    wf.nChannels = 2;
    wf.wBitsPerSample = 16;
    wf.nSamplesPerSec = 44100;
    wf.nBlockAlign = wf.nChannels * wf.wBitsPerSample / 8;
    wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
    waveOutOpen(&hWaveDev, WAVE_MAPPER, &wf, (DWORD)waveOutProc, 0, CALLBACK_FUNCTION);
    // 헤더는 건너 뛰고 버퍼에 샘플 데이터를 읽어들인다.
    SetFilePointer(hFile, 44, NULL, SEEK_SET);
    bufsize = wf.nAvgBytesPerSec;
    samplebuf = (char*)malloc(bufsize);
    hdr.lpData = samplebuf;
    do {
        ReadFile(hFile, samplebuf, bufsize, &dwRead, NULL);
        printf("Read %d\n", dwRead);
        hdr.dwBufferLength = dwRead;
        waveOutPrepareHeader(hWaveDev, &hdr, sizeof(WAVEHDR));
        waveOutWrite(hWaveDev, &hdr, sizeof(WAVEHDR));
        // 대기가 풀릴 때까지 기다린다.
        while (bWait) Sleep(0);
        bWait = true;
    } while (dwRead == bufsize);
    // 뒷정리한다.
    waveOutUnprepareHeader(hWaveDev, &hdr, sizeof(WAVEHDR));
    free(samplebuf);
    waveOutClose(hWaveDev);
    CloseHandle(hFile);
    return 0;
}
