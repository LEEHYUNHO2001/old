#include <stdio.h>
#include <windows.h>

#pragma comment(lib, "winmm.lib")

int main() {
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
}