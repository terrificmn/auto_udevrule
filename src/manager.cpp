#include "manager.hpp"

Manager::Manager(UdevMaker* udevMaker, const Mode& mode) 
                                    : ptrUdevMaker(udevMaker), mUsbInfoConfirmer(udevMaker), m_mode(mode) {
    //
}

void Manager::execute() {
    switch (m_mode) {
        case Mode::SINGLE_MODE : {
            this->singleMode();
            break;
        }

        case Mode::MULTI_MODE : {
            this->mulipleMode();
            break;
        }

        case Mode::INPUT_MODE : {
            this->inputMode();
            break;
        }

        case Mode::DELETE_MODE : {
            this->deleteMode();
            break;
        }

        default: break;
    }
}

std::string Manager::inputList(const std::string& str_print) {
    std::string str_input;

    std::cout << "== Please choose the device you want to " << str_print << std::endl;
    this->ptrUdevMaker->printList();
    std::cin >> str_input;

    if(this->ptrUdevMaker->getVSize() < stoi(str_input)) {
        std::cerr << "input number exceeded." << std::endl;
        return std::string();
    }

    /// input check
    bool res_num = this->mUsbInfoConfirmer.checkNumber(str_input);  // only 숫자
    if(!res_num) {
        std::cerr << "only number available" << std::endl;
        return std::string();
    }

    return str_input;
}

bool Manager::singleMode() {
    /// give options to the user
    std::string str_input = this->inputList("detect");
    if(str_input.empty()) {
        std::cerr << "error\n";
        return false;
    }
    
    /// detect usb device
    bool res = this->mUsbInfoConfirmer.detectUsb();
    if(!res) {
        std::cerr << "error\n";
        return false;
    }

    /// for the result
    UdevInfo& udevInfo = this->ptrUdevMaker->getUdevInfo();
    std::cout << "\n/// udevInfo ///\n";
    std::cout << "\tkernel: " << udevInfo.kernel << std::endl;
    std::cout << "\tproduct: " << udevInfo.product << std::endl;
    std::cout << "\tvendor: " << udevInfo.vendor << std::endl;
    std::cout << "\tserial: " << udevInfo.serial << std::endl;
    ///FYI: symlink can be assigned after makeUdevRule() is called.
    std::cout << "\tsymlink_name: Not decided yet" <<  /* udevInfo.symlink_name  << */ std::endl;

    ///FYI: for warning
    if(this->ptrUdevMaker->getSerialWarn()) {
        std::cerr << "[warn]serial info not found. Please change 'use_serial' to false in the config.lua" << std::endl;
        std::cerr << "[warn]device may not be found." << std::endl;
    }
    
    /// make a file under /etc/udev/... 
    int result = this->makeUdevRule(str_input);
    if(result != 0) {
        std::cerr << "\n== Failed to write the udev rule. ==\n\n";
        return false;
    }
    ///DEBUG
    std::cout << "\n== Copy complete!! ==\n\n";

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    int show_result = this->mUsbInfoConfirmer.showResult();
    if(show_result == 0) {
        std::cout << "Udev rules okay. Please check the result the below." << std::endl;
        std::cout << this->mUsbInfoConfirmer.getLsResult() << std::endl;
    }

    return true;
}


bool Manager::mulipleMode() {
    // bool is_auto = false;
    // std::cout << "== Please select the mode" << std::endl;
    // std::cout << "==>auto mode 0 : you can enter 0 to find it in order in a list." << std::endl;
    // std::cout << "==>manual mode 1+ : you can enter any number except 0." << std::endl;
    // std::cin >> str_input;
    
    // /// input check
    // bool res_num = usbChecker.checkNumber(str_input);  // only 숫자
    // if(!res_num) {
    //     std::cerr << "only number available" << std::endl;
    //     return 1;
    // }

    // if(str_input == "0") {
    //     std::cout << "auto seletect" << std::endl;
    //     is_auto = true;
    // }

    // for(int i=0; i < udevMaker.getVSize(); ++i ) {
    //     if(is_auto) {
    //         std::cout << "== Please plug in another device(usb cable)." << std::endl;
    //         std::cout << "If plugged already, please press any key.. It will be continued to dectect the usb device.." << std::endl;
    //         std::cout << "OR you can just hit ^c to exit.." << std::endl;
    //         std::cin >> str_input;  // just use for blocking

    //         bool res = usbChecker.detectUsb(); // default 1  // single use
    //         if(!res) {
    //             std::cerr << "error\n";
    //             return 1;
    //         }
    //         // detection 이후 
    //         udevMaker.setSymlink(i+1);  // 리스트를 1부터 출력해서 처리했으므로... 위의 입력과는 관계 없음. (순차적으로 만듬)
            

    //     } else {  // 입력 받은 INPUT 으로 처리 (싱글과 유사)
    //         // 결국 싱글의 반복
    //         std::cout << "== Please choose the device you want to detect.." << std::endl;
    //         std::cout << "OR you can just hit ^c to exit.." << std::endl;
    //         udevMaker.printList();
    //         std::cin >> str_input;
    //         /// input check
    //         bool res_num = usbChecker.checkNumber(str_input);  // only 숫자
    //         if(!res_num) {
    //             std::cerr << "only number available" << std::endl;
    //             return 1;
    //         }

    //         bool res = usbChecker.detectUsb(); // default 1  // single use
    //         if(!res) {
    //             std::cerr << "error\n";
    //             return 1;
    //         }

    //         udevMaker.setSymlink(std::stoi(str_input)); //  입력 받은 값으로 symlink 이름 만들기
    //     }

    //     // FOR 문에서 공통 처리 부분
    //     std::fstream fsScript;
    //     bool open_res = udevMaker.openFile(&fsScript, "temp_script.sh", Type::WRITE);
    //     if(!open_res) {
    //         std::cerr << "file open failure" << std::endl;
    //         return 1;
    //     }

    //     udevMaker.makeScript(&fsScript);
    //     fsScript.close();

    //     if(udevMaker.copyUdev()) {
    //         std::cout << "\n== Copy complete!! ==\n\n";
    //     }
            
    // } // end for loop

    std::cout << "Not surpported yet." << std::endl;
    return true;
}



/// @brief a wrapper function for createUdevruleFile()
/// @param input_str 
/// @return 
int Manager::makeUdevRule(const std::string& input_str) {
    // detection 이후 
    this->ptrUdevMaker->setSymlink(std::stoi(input_str));

    /// temp_script 만든 후에 복사하는 방식. 작동ok  -- stdin 으로 sub program 실행으로 변경 -on May12,2025
    // {
    //     std::fstream fsScript;
    //     bool open_res = this->ptrUdevMaker->openFile(&fsScript, "temp_script.sh", Type::WRITE);
    //     if(!open_res) {
    //         std::cerr << "file open failure" << std::endl;
    //         return false;
    //     }
    //     this->ptrUdevMaker->makeScript(&fsScript);
    //     fsScript.close();
    //     if(this->ptrUdevMaker->copyUdev()) {
    //         std::cout << "\n== Copy complete!! ==\n\n";
    //     }
    // }
    

    // 또는 직접 /etc쪽에 만들어주기 - permission 때문에 stdin 방식으로 해결
    return this->ptrUdevMaker->createUdevruleFile();
}

bool Manager::inputMode() {
    ///test 
    ///TODO: 테스트 완료 시 삭제
    this->inputSymlinkInManualMode();
    return false;

    std::string str_input = this->inputList("type manually");
    if(str_input.empty()) {
        return false;
    }

    std::cout << "::input_mode::" << std::endl;
    bool res = this->ptrUdevMaker->inputDevInfo();
    if(res) {
        std::cout << "input OK!\n";
        this->ptrUdevMaker->assignInfoByInput();  // instead of detectUsb()
    } else {
        std::cout << "aborted by user\n";
        return false;
    }

    bool result = this->makeUdevRule(str_input);
    if(!result) {
        return false;
    }

    return true;
}

/// @brief Set symlink name manaully in GUI, Not plan to use this in CLI
void Manager::inputSymlinkInManualMode() {
    std::cout << "Please type symlink name. *Do not add 'tty'" << std::endl;
    std::string user_input;
    std::cin >> user_input;
    // std::cout << "you enter: " << user_input << std::endl;

    this->ptrUdevMaker->setSymlinkNameByType(user_input);
}

bool Manager::deleteMode() {
    std::string str_input = this->inputList("remove");
    if(str_input.empty()) {
        return false;
    }

    std::string udev_filename;
    bool result = this->ptrUdevMaker->getUdevFilename(&udev_filename, std::stoi(str_input));
    if(!result) {
        std::cerr << "Can't get the udev filename." << std::endl;
        return false;
    }

    // std::cout << "Now trying to remove is " << udev_filename << std::endl;

    // 또는 직접 /etc쪽에 만들어주기 - permission 때문에 stdin 방식으로 해결
    int fin_result = this->ptrUdevMaker->deleteUdevruleFile(str_input);

    if(fin_result != 0) {
        std::cerr << "\n== Failed to delete the udev rule. ==\n\n";
        std::cout << "error code: " << fin_result << std::endl;
        return false;
    }

    std::cout << "Please re-launch the program if you want to use it again. Thank you." << std::endl;
    return true;
}
