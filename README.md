# udev rule 장치명 추출하기
usb로 연결된 장치의 장치id, 커널id, 벤더id, 모델id 등을 추출해주는 프로그램  
**마지막으로 검색 된 장치**만 사용됨: (마지막으로 usb 인식이 된 장치)

## github에서 실행 파일 getudev 다운로드
전체 소스코드 클론 할 필요 없이 getudev 파일만 다운 받아서 바로 실행 가능 하다..   
[깃허브 release 파일 다운로드](https://github.com/AMR-Labs/auto_udevrule/releases) 

> 이후 권한 때문에 실행이 안 된다면,   
> `chmod +x ./getudev` 해 준 후 실행해준다.(또는 getudev-ubuntu)    
> Rocky Linux 9, fedora 는 첫 번째 파일, Ubuntu 20.04는 getudev-ubuntu 을 다운 받는다. 


## 실행 옵션 -s, -m, -h   
- -s : 한번만 검색 후 종료   

- -m : 여러번 검색 가능, 
    0 은 list_file 목록 그대로 순차적으로 실행  
    1+ 0 외에 다른 숫자 입력시 리스트에서 고를 수 있는 모드.

- -d : 생성된 파일 및 symlink 장치를 지울 때 사용, 장치를 선택   

- -i : 프롬프트를 보면서 장치에 하나씩 입력 

- -h : 파라미터 설명


## 사용법
#### 준비,  
1. 만약 실행파일만 받아서 하는 경우라면 먼저 디렉토리를 만들어 준후 다운로드 받은 파일을 이동  
  ```
  mkdir ~/auto_udevrule
  mv ~/Downloads/getudev ~/auto_udevrule/   ## 또는 getudev-ubuntu 파일
  ```
2. 최초 실행을 (-s 옵션으로 실행) 해준다.   
  이후 ref 디렉토리가 만들어지고, 각 list_file, config.lua 파일이 생성이 된다.   
  
3. 프로그램을 재 실행 한다. 이하 *실행* 부분을 참고한다. 


#### 실행. (또는 깃 클론 한 경우)
1. 먼저 실행파일 있는 디렉토리에 있는 ref 디렉토리의 list_file을 선택한다.  
  여기의 파일들이 심링크 이름 및 /etc/udev/ 이하의 파일명으로 사용 되므로 원하는 이름으로 변경해준다.   
  (vi, gedit, vscode 등으로 편집해준다, default 는 수정 없이 사용)   
    * 아래의 [ref 디렉토리 설명 부분을 참고 하세요](#ref-디렉토리)  
2. config.lua 파일에서 kernel, serial 데이터를 사용 여부를 true/false 지정해준다.  
3. 특정 장치에 연결된 usb 케이블을 pc에 연결해준다.   
4. getudev 실행파일을 실행 시켜서 옵션을 선택한다.  (예 `./getudev -s`)   
5. 화면에 나온 원하는 장치의 번호를 누른 후 엔터   
6. 관리자 비밀번호를 넣어서 장치 관련 파일 업데이트가 된다.   

#### 확인. 
`ls -l /dev/tty*` 등으로 장치가 있는지 확인하기   


## 깃 클론 후 빌드하기
소스 코드 사용 시 
깃 클론 
```
git clone https://github.com/AMR-Labs/auto_udevrule.git
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
> 실행 파일만 받아서 사용하는 경우에는 필요하지 않다. 개발할 경우는 필요   
> 우분투 20 기준으로는 5.3, 우분투 22는 5.4 버전이 가능   


디렉토리 이동 후 빌드
```
cd ~/auto_udevrule
```

빌드
**(공통)** helper writer 빌드
```
g++ -std=c++17 -o helper_writer sub-src/helper_writer.cpp
```

메인 프로그램
```
g++ -std=c++17 -o getudev src/main.cpp src/usb_info_confirmer.cpp src/udev_maker.cpp src/lua_config.cpp src/manager.cpp src/time_checker.cpp src/sudo_manager.cpp src/sub_process_writer.cpp -I `pwd`/include -llua5.3 -ldl -DUBUNTU_20=true
```
> ubuntu 22 또는 fedora 에서는 -llua 이면 충분

**(옵션)** 라이브러리로 만들기 (without main)
```
g++ -std=c++17 -shared -fPIC -o libauto_udevrule.so.0.1.6 src/usb_checker.cpp src/udev_maker.cpp src/lua_config.cpp src/time_checker.cpp src/sudo_manager.cpp -I `pwd`/include -llua -ldl
```

**(옵션)** 라이브러리로 만들기 - ubuntu20.04 lua5.3 빌드 (without main) - 아래 우분투 용 빌드 참고
```
g++ -std=c++17 -shared -fPIC -o libauto_udevrule.so.0.1.6 src/usb_checker.cpp src/udev_maker.cpp src/lua_config.cpp src/time_checker.cpp src/sudo_manager.cpp -I `pwd`/include -llua5.3 -ldl
```

우분투 용 빌드 (20.04 - lua5.3)
```
g++ -std=c++17 -o getudev-ubuntu src/main.cpp src/usb_checker.cpp src/udev_maker.cpp src/lua_config.cpp src/manager.cpp src/time_checker.cpp -I `pwd`/include -llua5.3 -ldl
```
**IMPORTANT**: 일단 우분투 20일 경우에는 main.cpp/ lua_config.h 파일에서 *우분투 case 로 주석되어 있는 부분 해제해서   
lua5.3 디렉토리로 지정해서 빌드해줘야 한다.   
TODO: 추후 자동으로 할 수 있게 업데이트 예정   

> std::filesystem 추가로 인해서 c++17 버전으로 빌드   
> lua5.4 추가, 우분투 5.3


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


