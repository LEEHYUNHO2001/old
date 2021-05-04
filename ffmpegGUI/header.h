#pragma warning (disable : 6387)
#pragma warning (disable : 26451)
#pragma warning (disable : 6308)
#pragma warning (disable : 28182)
extern "C" {
#include <libavcodec/avcodec.h>//디코더, 인코더
#include <libavdevice/avdevice.h>//캡처 및 랜더링 기능 제공
#include <libavfilter/avfilter.h>//미디어 필터
#include <libavformat/avformat.h>//컨테이너 먹서, 디먹서
#include <libavutil/avutil.h>//난수 생성기, 수학 루틴 등의 유틸리티 기능 제공
#include <libswscale/swscale.h>//이미지 스케일링, 색상 변환
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>//오디오 리샘플링
}
#include <windows.h>//그래픽 환경
#include <commctrl.h>//공통 컨트롤
#include <tchar.h>//유니코드 문자열 처리
#include <stdio.h>
//PathFindFileName 함수 사용하기 위한 헤더. 라이브러리
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
