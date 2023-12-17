# udev rule 장치명 추출하기
usb로 연결된 장치의 장치id, 커널id, 벤더id, 모델id 등을 추출해주는 프로그램

> 마지막으로 검색 된 장치만 사용됨 

빌드하기
```
g++ -std=c++17 -o getudev src/main.cpp src/usb_checker.cpp src/udev_maker.cpp -I include
```

> std::filesystem 추가로 인해서 c++17 버전으로 빌드


실행 
```
./getudev
```

## github에서 getudev 다운
전체 소스코드 다운 할 필요 없이   
(테스트는 안해봤지만), getudev 파일 다운 받아서 바로 실행 가능 할 듯..  
권한 때문에 실행이 안된다면, `chmod +x ./getudev` 해 준 후 실행   



## 옵션 -s, -m, -h   
- -s : 한번만 검색 후 종료   

- -m : 여러번 검색 가능, 
    0 은 list_file 목록 그대로 순차적으로 실행  
    1+ 0 외에 다른 숫자 입력시 리스트에서 고를 수 있는 모드.

- -h : 파라미터 설명

## ref 디렉토리
(새로 업데이트 OnDec17 2023) 처음 실행파일만 있고 다른 파일이 없을 경우에는  
ref 디렉토리 및 list_file 을 만들어 준다. 최초 실행만 해주면 된다.  

ref/list_file 에서 리스트 목록을 불러오므로 실행 파일과 `ref/list_file` 은 같이 있어야 함.   

이제 이 목록에서의 이름은 /etc/udev/... 에 들어갈 파일이름으로 저장이 된다.  

(**필수**) 항상 리스트에서는 한 줄에 하나씩 넣어주고, - 로 구분해서 넣어준다.   
이유는 **-** 를 구분해서 심링크 이름으로 카멜케이스 형식으로 만들어준다.   

주석처리가 필요할 경우에는 # 으로 주석처리 한다. 

예:
``` 
## 주석처리
zltech-motor
faduino-upload
```

실행 후 파일명은 90-zltech-motor 식으로 만들어지고, 장치를 검색했을 경우 `/dev/ttyZltechMotor` 로 표시되게 된다.  


