#pragma warning (disable : 6387)
#pragma warning (disable : 26451)
#pragma warning (disable : 6308)
#pragma warning (disable : 28182)
extern "C" {
#include <libavcodec/avcodec.h>//���ڴ�, ���ڴ�
#include <libavdevice/avdevice.h>//ĸó �� ������ ��� ����
#include <libavfilter/avfilter.h>//�̵�� ����
#include <libavformat/avformat.h>//�����̳� �Լ�, ��Լ�
#include <libavutil/avutil.h>//���� ������, ���� ��ƾ ���� ��ƿ��Ƽ ��� ����
#include <libswscale/swscale.h>//�̹��� �����ϸ�, ���� ��ȯ
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>//����� �����ø�
}
#include <windows.h>//�׷��� ȯ��
#include <commctrl.h>//���� ��Ʈ��
#include <tchar.h>//�����ڵ� ���ڿ� ó��
#include <stdio.h>
//PathFindFileName �Լ� ����ϱ� ���� ���. ���̺귯��
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
