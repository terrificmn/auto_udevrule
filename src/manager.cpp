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
    bool res = this->detectUsb();
    if(!res) {
        std::cerr << "error\n";
        return false;
    }

    /// for the result
    if(!this->ttyUdevInfo) {
        std::cerr << "No shared TtyUdevInfo found." << std::endl;
        return false;
    }
    std::cout << "\n/// udevInfo ///\n";
    std::cout << "\tkernel: " << this->ttyUdevInfo->kernel << std::endl;
    std::cout << "\tproduct: " << this->ttyUdevInfo->product << std::endl;
    std::cout << "\tvendor: " << this->ttyUdevInfo->vendor << std::endl;
    std::cout << "\tserial: " << this->ttyUdevInfo->serial << std::endl;
    ///FYI: symlink can be assigned after makeUdevRule() is called.
    std::cout << "\tsymlink_name: Not decided yet" <<  /* udevInfo.symlink_name  << */ std::endl;
    
    ///FYI: for warning
    if(this->ptrUdevMaker->getSerialWarn(this->ttyUdevInfo) ) {
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
    int show_result = this->mUsbInfoConfirmer.showResult(this->ttyUdevInfo);
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


bool Manager::detectUsb() {
    ResultData resultData;
    bool is_acm_detected = false; //default

    /// step 1. find new device - USB, ACM
    for(int i=0; i<2; i++) {
        resultData = this->mUsbInfoConfirmer.findNewDevice(i);
        // std::cout << "return result string: " << resultData.result_str << std::endl;

        /// 1. time check
        bool detected_time_result = false;
        std::string detected_t_str = this->mUsbInfoConfirmer.getDetectedTime(resultData.result_str);
        if(!detected_t_str.empty()) {
            TimeChecker timeC;
            timeC.dmesgToRealTime(stod(detected_t_str));
            /// timeout --> compareDmesgTime
            detected_time_result = timeC.compareDmesgTime(stod(detected_t_str));
        }
        
        std::string cmd;
        if(i == 0) { // first try
             /// 2-1, check if the device number exists
            if(resultData.found_device_num != -1) {
                std::string cmd = "ls /dev/ttyUSB" + std::to_string(resultData.found_device_num);
                bool res = this->mUsbInfoConfirmer.executeSimpleCmd(cmd);
                if(!res) {
                    std::cout << "ttyUSB" << resultData.found_device_num << "not connected." << std::endl;
                    continue; /// still have one left
                }
            }

            /// 2-2. find kernel id
            if(this->mUsbInfoConfirmer.getKernelId(resultData.result_str).empty() == false && detected_time_result == true) {
                std::cout << "Okay. Found the USB device" << std::endl;
                break;
            } else {
                std::cout << "Not Found the device for ttyUSB." << std::endl;
            }
        
        } else if(i == 1) { // second try
            /// 3-1, check if the device number exists
            if(resultData.found_device_num != -1) {
                std::string cmd = "ls /dev/ttyACM" + std::to_string(resultData.found_device_num);
                bool res = this->mUsbInfoConfirmer.executeSimpleCmd(cmd);
                if(!res) {
                    std::cout << "ttyACM" << resultData.found_device_num << " is not connected." << std::endl;
                    return false;
                }
            }
            /// 3-2. find kernel id
            // if(this->getKernelIdForAcm(resultData.result_str).empty() == false && detected_time_result == true) { // the last try
            if(this->mUsbInfoConfirmer.getKernelIdForAcm(resultData.result_str).empty() == false) { // the last try
                std::cout << "Okay. Found the ACM device" << std::endl;
                is_acm_detected = true;
                break;
            } else {
                std::cout << "Not Found the device for ttyACM." << std::endl;
                std::cout << "USB might be disconnected. Try it again.\n";
                return false;
            }
        }
        std::cout << std::endl;
    } // for loop end
    

    this->ttyUdevInfo = std::make_shared<TtyUdevInfo>();

    /// step2. get Kenrnel id if it's found.  --> a bit different method to find the kernel id between usb and acm
    if(is_acm_detected) {
        // std::cout << "ACM device detected!" << std::endl;    
        ttyUdevInfo->kernel = this->mUsbInfoConfirmer.getKernelIdForAcm(resultData.result_str);
    
    } else {
        ttyUdevInfo->kernel = this->mUsbInfoConfirmer.getKernelId(resultData.result_str);
    }

    std::cout << "usb id: " << this->mUsbInfoConfirmer.getUsbId() << std::endl;
    std::cout << "kernel id: " << ttyUdevInfo->kernel << std::endl;

    /// step3. get vender_id, model_id, and serial_id
    std::vector<std::string> v_udev_str;
    v_udev_str = this->mUsbInfoConfirmer.findUdevInfo(is_acm_detected);

    std::string res_vender_id, res_model_id, res_serial_id;
    for(int i=0; i< v_udev_str.size(); ++i) {
        // res_vender_id = UsbInfoConfirmer.getVenderId(v_udev_str[i]);
        if(this->mUsbInfoConfirmer.getVenderId(v_udev_str[i]).empty() == false) {
            res_vender_id = v_udev_str[i];
        } 
        if(this->mUsbInfoConfirmer.getModelId(v_udev_str[i]).empty() == false) {
            res_model_id = v_udev_str[i];
        }
        if(this->mUsbInfoConfirmer.getSerialId(v_udev_str[i]).empty() == false) {
            res_serial_id = v_udev_str[i];
        }
        /// loop until all strings are found.
        if(!res_vender_id.empty() && !res_model_id.empty() && !res_serial_id.empty()) {
            // std::cout << "break!" << std::endl;
            break;
        }
    }

    // std::cout << "vendor_id : " << res_vender_id << std::endl;
    // std::cout << "model_id : " << res_model_id << std::endl;
    // std::cout << "serial_id : " << res_serial_id << std::endl;

    /// step4. save value
    this->ttyUdevInfo->vendor = this->mUsbInfoConfirmer.getIdsAterRegex(res_vender_id);
    this->ttyUdevInfo->product = this->mUsbInfoConfirmer.getIdsAterRegex(res_model_id);
    this->ttyUdevInfo->serial = this->mUsbInfoConfirmer.getIdsAterRegex(res_serial_id);
    std::cout << "extracted vendor_id : " << this->ttyUdevInfo->vendor  << std::endl;
    std::cout << "extracted model_id : " << this->ttyUdevInfo->product  << std::endl;
    std::cout << "extracted serial_id : " << this->ttyUdevInfo->serial << std::endl;

    return true;
}

/// @brief a wrapper function for createUdevruleFile()
/// @param input_str 
/// @return 
int Manager::makeUdevRule(const std::string& input_str) {
    // detection 이후 
    this->ptrUdevMaker->setSymlink(std::stoi(input_str), this->ttyUdevInfo);

    if(this->ptrUdevMaker->getIsPolicyKitNeeded()) {   
        int res = this->ptrUdevMaker->createUdevRuleFileWithFork(this->ttyUdevInfo);
        if(res == 0) {
            std::vector<std::string> cmd_result_data;
            std::string cmd = "udevadm control --reload-rules; udevadm trigger";
            this->mUsbInfoConfirmer.executeCmd(cmd_result_data, cmd, ResultType::EXECUTE_ONLY);
            return 0;
        }
        return res;

    } else {
        // 또는 직접 /etc쪽에 만들어주기 - permission 때문에 stdin 방식으로 해결
        return this->ptrUdevMaker->createUdevRuleFile(this->ttyUdevInfo);
    }
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

    if(!this->ttyUdevInfo) {
        this->ttyUdevInfo = std::make_shared<TtyUdevInfo>();
    }
    std::cout << "::input_mode::" << std::endl;
    bool res = this->ptrUdevMaker->inputDevInfo(this->ttyUdevInfo);
    if(res) {
        std::cout << "input OK!\n";\
    } else {
        std::cout << "aborted by user\n";
        return false;
    }

    int result = this->makeUdevRule(str_input);
    if(result != 0) {
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

    this->ptrUdevMaker->setSymlinkNameByType(user_input, this->ttyUdevInfo);
}

bool Manager::deleteMode() {
    std::string str_input = this->inputList("remove");
    if(str_input.empty()) {
        return false;
    }

    std::string udev_filename;
    int input_num;
    try {
        input_num = std::stoi(str_input);

    } catch(const std::exception& e) {
        std::cerr << "Exception error: " << e.what() << std::endl;
        return false;
    }
    bool result = this->ptrUdevMaker->getUdevFilename(&udev_filename, input_num);
    if(!result) {
        std::cerr << "Can't get the udev filename." << std::endl;
        return false;
    }
    if(!this->ttyUdevInfo) {
        this->ttyUdevInfo = std::make_shared<TtyUdevInfo>();
    }
    // std::cout << "Now trying to remove is " << udev_filename << std::endl;
    this->ptrUdevMaker->setSymlink(input_num, this->ttyUdevInfo);

    // 또는 직접 /etc쪽에 만들어주기 - permission 때문에 stdin 방식으로 해결
    int fin_result = this->ptrUdevMaker->deleteUdevruleFile();

    if(fin_result != 0) {
        std::cerr << "\n== Failed to delete the udev rule. ==\n\n";
        std::cout << "error code: " << fin_result << std::endl;
        return false;
    }

    std::cout << "Please re-launch the program if you want to use it again. Thank you." << std::endl;
    return true;
}
