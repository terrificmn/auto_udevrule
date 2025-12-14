# udev rule 장치명 추출하기
USB 장치의 디바이스id, 커널id, 벤더id, 모델id 등을 인식 후 udev rule 파일을 만들어서  
심링크로 해당 ttyUSB 를 연결 해주는 프로그램

지원 되는 타입 ttyUSB, ttyACM  
**마지막으로 검색 된 장치**만 연결: (마지막으로 usb 인식이 된 장치)  

TODO: 여러 장치 인식 기능 업데이트 예정

## 실행 옵션 -s, -m, -h   
-s : 한번만 검색 후 종료   
~~-m : 여러번 검색 가능,~~ *deprecated*  
-d : 생성된 파일 및 symlink 장치를 지울 때 사용, 장치를 선택   
-i : 프롬프트를 보면서 장치에 하나씩 입력  
-h : 파라미터 설명  
-v : 버전  


## 사용법
#### 준비,  
1. 실행 파일을 다운로드 후에 auto_udevrule 디렉토리를 만들고 이동  
  ```
  mkdir ~/auto_udevrule
  mv ~/Downloads/getudev ~/auto_udevrule/   ## 또는 getudev-ubuntu 파일
  ```
2. 최초 실행을 (-s 옵션으로 실행) 해준다.   
  이후 ref 디렉토리가 만들어지고, 각 list_file, config.lua 파일이 생성이 된다.   
  
3. 프로그램을 재 실행 한다. 이하 *실행* 부분을 참고한다. 


#### 실행. 
1. 먼저 실행파일 있는 디렉토리에 있는 ref 디렉토리의 list_file을 선택한다.  
  여기의 파일들이 심링크 이름 및 /etc/udev/ 이하의 파일명으로 사용 되므로 원하는 이름으로 변경해준다.   
  (vi, gedit, vscode 등으로 편집해준다, default 는 수정 없이 사용)   
    * 아래의 [ref 디렉토리 설명 부분을 참고 하세요](#ref-디렉토리)  
2. config.lua 파일에서 kernel, serial 데이터를 사용 여부를 true/false 지정해준다.  
3. 특정 장치에 연결된 usb 케이블을 pc에 연결해준다.   
4. getudev 실행파일을 실행 시켜서 옵션을 선택한다.  (예 `./getudev -s`)   
5. 화면에 나온 원하는 장치의 번호를 누른 후 엔터   
6. 관리자 비밀번호를 넣어서 장치 관련 파일 업데이트가 된다.   


## 빌드  
빌드가 따로 필요 할 경우, 클론 후 빌드 한다.   
깃 클론  
```
git clone https://github.com/terrificmn/auto_udevrule.git
```

### 의존성 패키지 설치
on Fedora
```
sudo dnf install lua-devel
```

on Ubuntu
```
sudo apt install liblua5.3-dev
```
> 우분투 20 / 22 에서도 5.3을 설치, 다만 빌드 시에 변수 설정을 해줘야 함(우분투 20)  

디렉토리 이동 후 빌드
```
cd ~/auto_udevrule
```

### g++ 빌드 
빌드하기 g++ 로 빌드를 한다.  
> 주로 Fedora 에서 작업 예정, Ubuntu 에서는 빌드 configure 시에 UBUNTU_DEV=true 로 사용  
Release 빌드만 Ubuntu 20 에서 cmake 로 빌드 하기.  

1. **(공통)** helper writer 빌드
```
g++ -std=c++17 -o helper_writer sub-src/helper_writer.cpp
```

2-1. 메인 프로그램
```
g++ -std=c++17 -o getudev src/main.cpp src/usb_info_confirmer.cpp src/udev_maker.cpp src/lua_config.cpp src/manager.cpp src/time_checker.cpp src/sudo_manager.cpp src/sub_process_writer.cpp -I `pwd`/include -llua -ldl
```

2-2, 우분투 용 빌드 (20 / 22) - development 일 경우에만 사용  
**중요**Release 빌드는 cmake 를 사용한다. - 크게 다른점, lua를 static으로 빌드
```
g++ -std=c++17 -o getudev src/main.cpp src/usb_info_confirmer.cpp src/udev_maker.cpp src/lua_config.cpp src/manager.cpp src/time_checker.cpp src/sudo_manager.cpp src/sub_process_writer.cpp -I `pwd`/include -llua5.3 -ldl -DUBUNTU_DEV=true
```
> UBUNTU_DEV=true 로 변수를 셋팅, 및 library 는 lua5.3

3-1. **(옵션)** 라이브러리로 만들기 (without main)
```
g++ -std=c++17 -shared -fPIC -o libauto_udevrule.so.1.1.0 src/usb_info_confirmer.cpp src/udev_maker.cpp src/lua_config.cpp src/time_checker.cpp src/sudo_manager.cpp src/sub_process_writer.cpp -I `pwd`/include -llua -ldl
```

3-2. **(옵션)** 라이브러리로 만들기 - ubuntu 20/22 lua5.3 빌드 (without main)  
```
g++ -std=c++17 -shared -fPIC -o libauto_udevrule.so.1.1.0 src/usb_info_confirmer.cpp src/udev_maker.cpp src/lua_config.cpp src/time_checker.cpp src/sudo_manager.cpp src/sub_process_writer.cpp -I `pwd`/include -llua5.3 -ldl -DUBUNTU_DEV=true
```
> UBUNTU_DEV=true 로 변수를 셋팅, 및 library 는 lua5.3  
TODO: lua 포함하지 않는 g++ 또는 cmake 빌드 업데이트 예정


## ref 디렉토리
(새로 업데이트 OnDec17 2023) 처음 실행파일만 있고 다른 파일이 없을 경우에는  
ref 디렉토리 및 list_file 을 만들어 준다. 최초 실행만 해주면 된다.   
최초 실행 시 리스트 파일이 없다면 만들어 준다.
```
./getudev -s
```

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

실행 후 파일명은 **90-zltech-motor** 식으로 만들어지고, 장치를 검색했을 경우 `/dev/ttyZltechMotor` 로 표시되게 된다.  


