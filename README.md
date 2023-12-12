# udev rule 장치명 추출하기
usb로 연결된 장치의 장치id, 커널id, 벤더id, 모델id 등을 추출해주는 프로그램

> 마지막으로 검색 된 장치만 사용됨 

빌드하기
```
g++ -std=c++14 -o getudev src/main.cpp src/usb_checker.cpp src/udev_maker.cpp -I include
```


실행 
```
./getudev
```


## 옵션 -s, -m, -h   
- -s : 한번만 검색 후 종료   

- -m : 여러번 검색 가능, 
    0 은 list_file 목록 그대로 순차적으로 실행  
    1+ 0 외에 다른 숫자 입력시 리스트에서 고를 수 있는 모드.

- -h : 파라미터 설명

## ref 디렉토리
ref/list_file 에서 리스트 목록을 불러오므로 실행 파일과 `ref/list_file` 은 같이 있어야 함.   
리스트에서는 한 줄에 하나씩 넣어주고, - 로 구분해서 넣어준다.  
주석처리가 필요할 경우에는 # 으로 주석처리 한다. 

예:
``` 
## 주석처리
zltech-motor
faduino-upload
```


