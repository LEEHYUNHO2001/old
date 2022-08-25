#include "header.h"
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
bool GetMediaInfo(TCHAR* Path, sMedia* pMedia);
bool AppendMedia(int list, TCHAR* MediaPath, bool tolist);
void AppendMedia(int list, sMedia media, bool tolist);
void AppendMediaToList(sMedia media);
void FillList(int list);
void ChangeMedia(int list, int idx);
void DropFiles(HDROP hDrop, bool Reset);
void LoadOption();
void SaveOption();
void AdjustVolumeTo(int vol);
void AdjustVolume(bool up);
void SetOverlayMsg(LPCTSTR msg, UINT hpos = 1, UINT vpos = 0, int timeout = 2000);
void DrawOverlayMsg(HDC hdc, AVFrame* pFrame);
