
## 장비 및 센서
- Raspberry pi 3 b+
- pi camera
- 소리 센서
- 이산화탄소 감지 센서


<br>
<br>

## 기능

<img width="663" alt="description" src="https://user-images.githubusercontent.com/78518132/119444480-47920d80-bd66-11eb-96b7-0c4c4177226e.png">

- [x] 얼굴 인식, 눈 인식, 턱 인식
- [x] 눈을 감으면, count가 시작됨. -> 3초 후에 경고 알람 발생
- [x] 눈을 감고있으면 경고음 계속 발생
- [x] 눈을 감은채, 고개를 까딱까딱(졸면) 하면 좀 더 시끄러운 경고 알람 발생
- [x] CO2 농도가 3000 이상이면, 환기를 시키라는 알람 발생

<br>
<br>

## 사용 기술


<br>
<br>

1. OpenCV
- Video File & Camera에서 영상을 받아온다. 
- 받아온 영상에 대한 전반적인 처리 및 가공한다.

<br>
<br>

2. Grayscaling
- 시각적 특징을 구별하는 휘도(단위면적당 광도)를 증가시킨다.
- 영상 처리 속도에 영향을 주는 컬러 이미지보다 효율적으로 많은 작업에 이용 가능하다.
- 밤 또는 어두운 환경에서도 얼굴인식이 원활하게 될 수 있도록 gray_scale을 사용한다.

<br>
<br>

3. Dlib


<img width="270" alt="2" src="https://user-images.githubusercontent.com/78518132/121977744-a83ec400-cdc1-11eb-9d94-ff75d6724113.png"><img width="241" alt="1" src="https://user-images.githubusercontent.com/78518132/121977766-b2f95900-cdc1-11eb-84ce-7d9cd4b8136f.png">

- 라이브러리에 미리 학습된 데이터를 통하여 운전자의 얼굴을 인식한다.
- shape_predictor_68_face_landmarks.dat을 통하여 운전자 얼굴에 68개의 랜드마크 좌표를 얻는다.
- Eye Aspect Ratio(EAR)을 구하기 위한 공식을 사용한다.
- 눈의 비율을 실시간으로 측정가능 하도록 설계 한다.

<br>
<br>

4. Thread -> 처리시간 감소-module파일 수정

<img width="667" alt="3" src="https://user-images.githubusercontent.com/78518132/121977978-2bf8b080-cdc2-11eb-9c8e-8039a52f599b.png">

<br>
<br>

## 블록 다이어그램


<img width="672" alt="diagram" src="https://user-images.githubusercontent.com/78518132/119444453-3e08a580-bd66-11eb-9e07-9dbc5231ee96.png">
