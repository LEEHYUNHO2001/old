import time
import MZ
from scipy.spatial import distance as dist
from imutils.video.pivideostream import PiVideoStream
from imutils.video import FPS
from imutils import face_utils
import numpy as np
from playsound import playsound
from picamera.array import PiRGBArray
from picamera import PiCamera
import pygame 
import sys
import serial
import time

import argparse
import imutils
import time
import dlib
import cv2
import mh_z19

count = 0
wow=0
dan =0
check_p=999

def sound_alarm(path):
    playsound.playsound(path)
    
def eye_aspect_ratio(eye):
    A = dist.euclidean(eye[1], eye[5])
    B = dist.euclidean(eye[2], eye[4])
    C = dist.euclidean(eye[0], eye[3])
    ear = (A+B) / (2.0 * C)
    return ear

ap = argparse.ArgumentParser()
ap.add_argument("-p", "--shape-predictor", required=True, help="path to facial landmark predictor")
ap.add_argument("-a", "--alarm", type=str, default="", help="path alarm .WAV file")
ap.add_argument("-w", "--webcam", type=int, default=0, help="index of webcam on system")
args = vars(ap.parse_args())

EYE_AR_THRESH = 0.3
EYE_AR_CONSEC_FRAMES = 15
EYE_AR_CONSEC_FRAMES1 = 7

jear_THRESH =0.5
jear_CONSEC_FRAMES = 15

COUNTER = 0
ALARM_ON = False

print("[INFO] loading facial landmark predictor...")
detector = dlib.get_frontal_face_detector()
predictor = dlib.shape_predictor(args["shape_predictor"])

(lStart, lEnd) = face_utils.FACIAL_LANDMARKS_IDXS["left_eye"]
(rStart, rEnd) = face_utils.FACIAL_LANDMARKS_IDXS["right_eye"]
(jStart, jEnd) = face_utils.FACIAL_LANDMARKS_IDXS["jaw"] #

print("[INFO] starting video stream thread...")

vs = PiVideoStream().start()
time.sleep(1.0)

pygame.mixer.init()  ##
alarm1 = pygame.mixer.Sound("Caution.wav")  # sleep
alarm2 = pygame.mixer.Sound("Danger.wav")  # sleep
alarm3 = pygame.mixer.Sound("CO2.wav")  # co2 leak

while True:
    
    frame = vs.read()
    frame = imutils.resize(frame, width=300)
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    rects = detector(gray, 0)
    
    for rect in rects:
        shape = predictor(gray, rect)
        shape = face_utils.shape_to_np(shape)
        jaw = shape[jStart:jEnd] #
        jawHull = cv2.convexHull(jaw) #
        cv2.drawContours(frame, [jawHull], -1, (0, 255, 0), 1) #
        jear = jaw #
        
        
        leftEye = shape[lStart:lEnd]
        rightEye = shape[rStart:rEnd]
        leftEAR = eye_aspect_ratio(leftEye)
        rightEAR = eye_aspect_ratio(rightEye)
        ear = (leftEAR + rightEAR) / 2.0
        leftEyeHull = cv2.convexHull(leftEye)
        rightEyeHull = cv2.convexHull(rightEye)
        cv2.drawContours(frame, [leftEyeHull], -1, (0, 255, 0), 1)
        cv2.drawContours(frame, [rightEyeHull], -1, (0, 255, 0), 1)
        check_n = shape[28][1]
        
                                       
        if ear < EYE_AR_THRESH:
            COUNTER += 1
            cv2.putText(frame, "close = {}".format(COUNTER), (200, 50),
                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)
            if (COUNTER % 15) == EYE_AR_CONSEC_FRAMES1:
                alarm1.stop()
                if (dan == 1) :
                    dan = 0
                    continue
                else :
                    alarm1.play()
                
                cv2.putText(frame, "Caution", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)
                
  
            if COUNTER > EYE_AR_CONSEC_FRAMES1 :
                if check_n > check_p + 2 : 
                    wow += 1
                    cv2.putText(frame, "count = {}".format(wow), (10, 60),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)
                    if wow >= 8 :
                        dan = 1
                        alarm1.stop()
                        alarm2.play()     
                        cv2.putText(frame, "Dangerous!", (10, 40),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)
                        wow = 0

        else:
            COUNTER = 0
            ALARM_ON = False
        
        
        cccc = mh_z19.read_all()
        cc=cccc['co2']
        
        cv2.putText(frame, "CO2 : {}".format(cc), (10, 220),
            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)   ##
        
        if cc > 3000 :
            alarm3.play()   
            cv2.putText(frame, "CO2 Leak", (10, 200),
                cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2)   ##
                
            
        cv2.putText(frame, "EAR: {:.2f}".format(ear), (200, 30),
            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 2) 
        check_p = shape[28][1]
        
    cv2.imshow("Frame", frame)
    key = cv2.waitKey(1) & 0xFF
 
    if key == ord("q"):
        break
        
cv2.destroyAllWindows()

vs.stop()







