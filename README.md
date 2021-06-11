# FFmpeg Player Project
- [Development environment](#development-environment)
- [Implementation](#implementation)
- [FFmpeg Player](#ffmpeg-player)
- [FFmpeg Common sense](#ffmpeg-common-sense)
- [More..](#more)

</br>
</br>

## Development environment
- Visual Stdio : C++
- FFmpeg Library
- Waveform Library
- ini

</br>
</br>

## Implementation
- [x] Video File(Container)에서 Video Stream, Audio Stream 찾기
- [x] Video Stream, Audio Stream에 해당하는 Codec Open
- [x] Codec으로 압축 풀어 패킷 받기
- [x] Codec 내부 Buffer 에러처리
- [x] sws_scale : 색상포맷에 따른 변환
- [x] bitmap
- [x] Thread분리 -> PlayThread 생성. Video 출력 -> uSleep함수 이용한 연속재생
- [x] Audio 출력을 위한 Waveform사용 -> 오디오 리샘플링, 가변패킷 저장 algorithm
- [x] Window 분할 -> Pannel(Open, Pause, + - D button), Stage, List(목록)
- [x] 파일 Open, 글 목록
- [x] Video File에 따른 Window 창 크기설정, Video File 드래그하여 목록에 추가
- [x] ini File에 설정 정보 저장(목록, window창 위치 등)
- [x] 볼륨 설정 -> 볼륨값도 비트맵에 같이 표시

</br>
</br>

## FFmpeg Common sense

### Container

<img width="300" alt="컨테이너" src="https://user-images.githubusercontent.com/78518132/121274385-58658600-c905-11eb-9d51-cf60adaaf234.png">

- **동영상 파일은 재생과 편집을 원할하게 하기 위한 일련의 규격**을 담고 있는데, 이러한 규격을 컨테이너라고 한다. 동영상 파일이 어떤 규격의 컨테이너를 가졌는지는 파일의 확장자로 알 수 있다. ( avi, nkv, mp4도 컨테이너임 )
- 컨테이너는 오직 스트림을 제어하기 위한 정보만 가지고 있을 뿐, 스트림이 어떤 방식으로 압축되었는지는 알 수 없다. 또한, 규격에 맞는 스트림만을 담을 수 있기 때문에 모든 형식의 스트림을 담을 수도 없다.

</br>
</br>

### Muxing & Demuxing

<img width="500" alt="동작" src="https://user-images.githubusercontent.com/78518132/121275096-de360100-c906-11eb-9547-f2bf2df0de8b.png">

- 캡처된 비디오와 녹음된 오디오를 저장하기 위해서는 반드시 이를 제어할 수 있는 컨테이너에 담아야 한다. 이때 **컨터에너에 스트림을 담는 일련의 과정을 Muxing** 이라고 한다. ( 여러 입력을 하나로 합치는 과정 )
- 반대로 **컨테이너에 있는 스트림을 컨테이너에서 분리하는 일련의 과정을 Demuxing** 이라고 한다. ( 하나로 합쳐진 입력을 다시 여러 출력으로 만드는 과정 )
- Muxing 된 동영상을 재생하기 위해서는 Demuxing을 통해서 압축된 정지 영상과 오디오로 분리한 후 각각의 데이터를 Decoding 해야한다. -> **FFmpeg Library**

</br>
</br>

### Codec

<img width="450" alt="codec" src="https://user-images.githubusercontent.com/78518132/121275078-d37b6c00-c906-11eb-8751-d3990b118052.png">

- 영상 또는 오디오의 신호를 디지털 신호로 변환하는 **COder**와 그 반대로 변환시켜 주는 **DECoder**를 통틀어 부르는 용어이다.
- **Encoding** : 코덱은 아날로그 신호나 스트림 데이터로 이루어진 비디오와 오디오를 **압축**된 부호로 변환하기 위한 압축 규격을 제공한다.  →  용량을 줄이기 위해 한다.
- **Decoding** : 코덱은 압축된 데이터를 본래의 아날로그 신호나 스트림 데이터로 **복원**하기 위한 규격도 제공한다.
- 코덱의 압축 방식에는 원본이 손상되는 **손실 압축**과 원본의 손실 없이 그대로 보전되는 **무손실 압축**이 있다. 
압축 효율은 손실 압축 방식이 더 높지만 원본을 보전해야 하는 파일같은 경우는 무손실 압축 방식을 선호하기도 한다.

</br>
</br>

### Popular Codec

- [H.264 / AVC(Advanced Video Coding)](http://www.ktword.co.kr/abbr_view.php?m_temp1=3067)
- [AAC ( Advanced Audio Coding )](http://www.ktword.co.kr/abbr_view.php?nav=2&id=713&m_temp1=4195)

</br>
</br>

### FFmpeg Container

<img width="400" alt="ffmpeg" src="https://user-images.githubusercontent.com/78518132/121279681-e6466e80-c90f-11eb-91da-48cb761cd9f1.png">

- AVFormatContext 구조체 안에는 적어도 하나 이상의 스트림이 있는데, 컨테이너  종류에 따라 다르다. 스트림의 정보는 AVStream안에 있다.
- AVFormatContext 구조체 안에는 적어도 하나 이상의 스트림이 있는데, 컨테이너  종류에 따라 다르다. 스트림의 정보는 AVStream안에 있다.
- AVStream 안에는 코덱과 관련된 정보를 가지고 있는 AVCodecContext가 있다. 코덱은 종류에 따라 다른 정보를 가지고 있을 수 있다, ( 비디오 → 해상도..., 오디오→채널개수... 등 )

</br>
</br>

## FFmpeg Player

<img width="706" alt="sample" src="https://user-images.githubusercontent.com/78518132/121280711-b7c99300-c911-11eb-8dab-f779db3bf21d.png">

1. Player 실행시 창 위치, 파일 목록, 시청하던 영상(video or audio)을 ini파일 불러와서 적용
2. [Open] 버튼으로 영상을 실행
3. [+] 버튼으로 목록에 영상을 추가. 폴더에서 드래그 하여 추가 가능.
4. [-] , [D]버튼으로 목록에서 영상 삭제
5. [volunm] 조절
6. 저장된 영상이 아닌, RTSP 영상도 가능.

</br>
</br>

## More..

### FFmpeg Library - codec search and open

```c++
	vidx = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	aidx = av_find_best_stream(fmtCtx, AVMEDIA_TYPE_AUDIO, -1, vidx, NULL, 0);
  if (vidx >= 0) {
		vStream = fmtCtx->streams[vidx];
		vPara = vStream->codecpar;
		vCodec = avcodec_find_decoder(vPara->codec_id);
		vCtx = avcodec_alloc_context3(vCodec);
		avcodec_parameters_to_context(vCtx, vPara);
		avcodec_open2(vCtx, vCodec, NULL);
	}
	if (aidx >= 0) {
		aStream = fmtCtx->streams[aidx];
		aPara = aStream->codecpar;
		aCodec = avcodec_find_decoder(aPara->codec_id);
		aCtx = avcodec_alloc_context3(aCodec);
		avcodec_parameters_to_context(aCtx, aPara);
		avcodec_open2(aCtx, aCodec, NULL);
	}

```
video stream, audio stream의 index 구하고, 해당 codec open

</br>
</br>

### Packet Receive

<img width="232" alt="패킷받기2" src="https://user-images.githubusercontent.com/78518132/121284208-6b815180-c917-11eb-83dc-a6d511e9e9c2.png"><img width="456" alt="패킷받기" src="https://user-images.githubusercontent.com/78518132/121283959-0168ac80-c917-11eb-9ac8-c31681f49da4.png">

Video 와 Audio packet receive 구조



</br>
</br>

### Codec Buffer

<img width="341" alt="코덱내부버퍼에러처리" src="https://user-images.githubusercontent.com/78518132/121284393-bb601880-c917-11eb-88e5-38c5e2cc3391.png">

EAGAIN가 양쪽에서 발생할 수 없음(모순). send의 EAGAIN을 방지하도록 설계.

</br>
</br>

### Video (software Scale, BitMap)

- Video 마다 동영상 포맷은 다름 -> 포맷별로 변환함수를 모두 만들면 비효율적이므로 ffmpeg 기능 swsCtx 사용
- Pixel을 모두 찍어서 Video 출력하면 비효율적 -> bitmap 사용

</br>
</br>

### Audio(Waveform Library) 가변길이 패킷

<img width="524" alt="멀티헤더" src="https://user-images.githubusercontent.com/78518132/121286839-5e666180-c91b-11eb-8414-120b251bbc8e.png"><img width="410" alt="가변길이패킷" src="https://user-images.githubusercontent.com/78518132/121286863-6cb47d80-c91b-11eb-86ef-508a8e75ce5f.png">

1. 헤더 10개, 버퍼 사이즈 17640, 패킷사이즈 20000
2. frame_read 하고 오디오 stream이면, if문 안에 들어옴
3. 처음에 wa.hWaveDev 가 NULL이므로, 장치와 주요 변수 초기화. 장치 Open 
4. 두 메모리 연결, wa.availhdr (사용 가능 헤더) = wa.hdrnum(헤더 갯수 10) 으로 초기화. → **처음에는 10개 다 사용가능하기 때문.**
5. wa.nowhdr (현재 헤더) = 0  → **처음에는 0번째 헤더부터 사용하기 때문.**
6. wa.bufptr (버퍼 선두) = wa.samplebuf[wa.nowhdr]; → 현재 헤더의 선두를 burptr에 넣음. → **이해 쉽게 0이라고 보자.**
7. wa.pktbuf를 pktsize인 20000 만큼 할당
8. 리샘플러 초기화하고, sampnum에 해당 포맷을 wav로 리샘플링한 패킷 읽음
9. **wa.pktptr(패킷 선두)** 에 wa.pktbuf를 넣음 → **현재 0이라고 보자**
10. **패킷 버퍼의 남은 데이터 양인 remainpkt**을 바이트 단위로 바꿈. → **20000남아있음**
11. remainbuf는 **남은 버퍼** → 버퍼 사이즈(17640) - 버퍼 선두(0) - 현재 헤더 선두(0) = **17640**
12. 남은 버퍼보다 패킷 사이즈가 더 크므로 **if문 무시**하고 지나감.
13. 버퍼 선두 0 지점에 패킷선두에서부터 버퍼의 남은양 17640만큼 채움 -> **헤더1 : 0 -17640 만큼 패킷 데이터가 차있음. **
14. 남은 패킷의 데이터 remainpkt를 처음에 남았던 버퍼 양 remainbuf만큼 빼줌 → 20000 - 17640 = **2360** → **다음 버퍼에는 이정도의 패킷의 데이터만 저장해야되니까..**
15. 패킷의 선두 wa.pktptr는 처음에 남았던 버퍼 양 remainbuf만큼 더해줌 → **패킷의 2360 지점부터 저장해야되니까..**
16. 헤더를 장치로 보내 재생한다.
17. 사용 가능 헤더 수인 availhdr을 1 감소시킴 → **헤더1 꽉 찼으니까..**
18. 만약 사용가능한 헤더 없으면 기다리는데, 지금은 9개남았으니 넘어간다.
19. 중간에 꺼지거나 하면 break
20. 현재 헤더를 1 증가시킨다.  → **헤더1 꽉 찼으니까 다음 헤더 사용..**
21. 현재 헤더가 헤더 갯수(10)과 같으면 헤더를 다시 0으로 만들어 1번째 헤더를 사용하도록 한다. → **여기 if문을 왔다는것은 availhdr이 1개라도 있다는 것이고, 그게 1번째 헤더일것이기 때문.**
22. wa.bufptr을 증가시킨 헤더의 buf로 초기화한다. → **다음 헤더를 사용하기위한 준비**
23. for문 선두로 돌아간다.  **remainbuf** = 17640 - 0 - 0 = **17640** 이다.
24. 2360 < 17640 이므로, 이번에는 if문에 들어간다. **wa,bufptr에 wa.pktptr부터 (2360지점부터) 남은 데이터양 remainpkt(2360)만큼 저장. 다음 패킷은 2360 이후의 버퍼부터 저장해야 하므로, wa.bufptr += remainpkt; 하고 break;**
25. 다시 패킷의 데이터양 20000 받아온다.. → 끝날때가지 위의 과정 반복

### 각종 기능
윈도우 분할, 파일 열기, 파일 목록, 설정 저장(ini), 볼륨 


</br>
[출처](http://soen.kr/)
