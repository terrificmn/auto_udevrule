#include <iostream>
#include <string>
#include "usb_checker.h"
#include "udev_maker.h"
#include "device_enum.h"

void helpMsg();

int main(int argc, char** argv) {
    bool is_multi_mode = false;

    /// parameter part
    if(argc == 2) {
        std::string argv_str = argv[1];
        if(argv_str == "-s") {
            // std::cout << argv[1] << std::endl;
            std::cout << "single mode selected" << std::endl;

        } else if(argv_str == "-m") {
            // std::cout << argv[1] << std::endl; // multiple use (like 10 times)
            std::cout << "multi mode selected" << std::endl;
            is_multi_mode = true;

        } else if(argv_str == "-h" || argv_str == "--help" ) {
            helpMsg();
            return 1;

        } else {
            std::cout << "wrong parameter: only accept -s or -m" << std::endl;
            return 1;
        }

    } else {
        helpMsg();
        return 1;
    }

    /// usb check part
    UdevMaker udevMaker;
    UsbChecker usbChecker(&udevMaker);
    std::string str_input;
    std::fstream fsFile;

    /// read list : preparation for symlink names and script file names.
    bool open_res = udevMaker.openFile(&fsFile, "list_file", Type::READ); //or out
    if(!open_res) {
        std::cerr << "file open failure [list_file]" << std::endl;
        udevMaker.createBasicList();
        fsFile.close();
        std::cout << "Please execute the program again~.\n";
        return 1;
    }
        // 
    std::cout << "Reading a config file for an udev list...";
    udevMaker.saveVector(&fsFile);
    fsFile.close();
    std::cout << " Done\n";


    if(!is_multi_mode) {
        std::cout << "== Please choose the device you want to detect.." << std::endl;
        udevMaker.printList();
        std::cin >> str_input;

        /// input check
        bool res_num = usbChecker.checkNumber(str_input);  // only 숫자
        if(!res_num) {
            std::cerr << "only number available" << std::endl;
            return 1;
        }

        bool res = usbChecker.detectUsb(); // default 1  // single use
        // bool res =true; // 강제로 설정 --- 테스트 후 삭제하기
        if(!res) {
            std::cerr << "error\n";
            return 1;
        }

        // detection 이후 
        udevMaker.setSymlink(std::stoi(str_input));
        std::fstream fsScript;
        bool open_res = udevMaker.openFile(&fsScript, "temp_script.sh", Type::WRITE);
        if(!open_res) {
            std::cerr << "file open failure" << std::endl;
            return 1;
        }

        udevMaker.makeScript(&fsScript);
        fsScript.close();

        // 또는 직접 /etc쪽에 만들어주기
        // 테스트 주석 처리
        if(udevMaker.copyUdev()) {
            std::cout << "\n== Copy complete!! ==\n\n";
        }
    
    } else { // multi_mode
        bool is_auto = false;
        std::cout << "== Please select the mode" << std::endl;
        std::cout << "==>auto mode 0 : you can enter 0 to find it in order in a list." << std::endl;
        std::cout << "==>manual mode 1+ : you can enter any number except 0." << std::endl;
        std::cin >> str_input;
        
        /// input check
        bool res_num = usbChecker.checkNumber(str_input);  // only 숫자
        if(!res_num) {
            std::cerr << "only number available" << std::endl;
            return 1;
        }

        if(str_input == "0") {
            std::cout << "auto seletect" << std::endl;
            is_auto = true;
        }

        for(int i=0; i < udevMaker.getVSize(); ++i ) {
            if(is_auto) {
                std::cout << "== Please plug in another device(usb cable)." << std::endl;
                std::cout << "If plugged already, please press any key.. It will be continued to dectect the usb device.." << std::endl;
                std::cout << "OR you can just hit ^c to exit.." << std::endl;
                std::cin >> str_input;  // just use for blocking

                bool res = usbChecker.detectUsb(); // default 1  // single use
                if(!res) {
                    std::cerr << "error\n";
                    return 1;
                }
                // detection 이후 
                udevMaker.setSymlink(i+1);  // 리스트를 1부터 출력해서 처리했으므로... 위의 입력과는 관계 없음. (순차적으로 만듬)
                

            } else {  // 입력 받은 INPUT 으로 처리 (싱글과 유사)
                // 결국 싱글의 반복
                std::cout << "== Please choose the device you want to detect.." << std::endl;
                std::cout << "OR you can just hit ^c to exit.." << std::endl;
                udevMaker.printList();
                std::cin >> str_input;
                /// input check
                bool res_num = usbChecker.checkNumber(str_input);  // only 숫자
                if(!res_num) {
                    std::cerr << "only number available" << std::endl;
                    return 1;
                }

                bool res = usbChecker.detectUsb(); // default 1  // single use
                if(!res) {
                    std::cerr << "error\n";
                    return 1;
                }

                udevMaker.setSymlink(std::stoi(str_input)); //  입력 받은 값으로 symlink 이름 만들기
            }

            // FOR 문에서 공통 처리 부분
            std::fstream fsScript;
            bool open_res = udevMaker.openFile(&fsScript, "temp_script.sh", Type::WRITE);
            if(!open_res) {
                std::cerr << "file open failure" << std::endl;
                return 1;
            }

            udevMaker.makeScript(&fsScript);
            fsScript.close();

            // 또는 직접 /etc쪽에 만들어주기
            if(udevMaker.copyUdev()) {
                std::cout << "\n== Copy complete!! ==\n\n";
            }
                
        } // end for loop

    }


    return 0;
}



void helpMsg() {
    std::cout << "help: add parameter -s or -m" << std::endl;
    std::cout << "-s: single use" << std::endl;
    std::cout << "-m: multiple use" << std::endl;
    std::cout << "-h: help message" << std::endl;
}