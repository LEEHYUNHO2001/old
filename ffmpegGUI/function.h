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