#include "manager.hpp"

Manager::Manager(UdevMaker* udevMaker, const Mode& mode) 
                                    : ptrUdevMaker(udevMaker), mUsbInfoConfirmer(udevMaker), m_mode(mode) {
    //
}

void Manager::execute() {
    switch (m_mode) {
        case Mode::LAST_DETECT_MODE : {
            this->singleMode();
            break;
        }

        case Mode::ALL_DETECT_MODE : {
            this->allDetectMode();
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

    try {
        if(this->ptrUdevMaker->getVSize() < stoi(str_input)) {
            std::cerr << "input number exceeded." << std::endl;
            return std::string();
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception error: " << e.what() << std::endl;
        return "";
    }

    /// input check
    bool res_num = this->mUsbInfoConfirmer.checkNumber(str_input);  // only 숫자
    if(!res_num) {
        std::cerr << "only number available" << std::endl;
        return std::string();
    }

    return str_input;
}

std::string Manager::inputProductCategory() {
    std::string str_input;

    std::cout << "== Please choose number for the product category." << std::endl;
    int pc_size = LuaConfig::luaParam.v_product_category.size();

    for(int i=0; i < pc_size; ++i) {
        std::cout << i << " : " << LuaConfig::luaParam.v_product_category.at(i).alias << std::endl; 
    }
    std::cin >> str_input;

    int input_num;
    try {
        input_num = stoi(str_input);
    } catch(const std::exception &e) {
        std::cerr << "Exception error: " << e.what() << ", use only number." << std::endl;
        return "";
    }

    try {
        return LuaConfig::luaParam.v_product_category.at(input_num).alias;
    } catch(const std::exception& e) {
        std::cerr << "Exception error: " << e.what() << std::endl;
        std::cerr << "input number exceeded." << std::endl;
        return "";
    }
}

/// @brief block std::cin
/// @param input_check 
/// @return 
bool Manager::inputReMakeOrNot(InputCheck input_check) {
    std::string str_input;

    if(input_check == InputCheck::RE_MAKE_UDEV) {
        std::cout << "Please check real settings. Are you sure to re-make udev rules? (y or n)" << std::endl;
    } else if(input_check == InputCheck::RE_SYMLINK) {
        std::cout << "== Please check if symlink name is right or not." << std::endl;
        std::cout << "== Type 'y' if you want to make a re-symlink again."  << std::endl;
        std::cout << "== Type 'n' to move on the next device."  << std::endl;
    } else if(input_check == InputCheck::RE_MAKE_UDEV_AGAIN) {
        std::cout << "Are you sure to switch the udevrules again? ('y'or'n') " << std::endl;
    } else if(input_check == InputCheck::RE_MAKE_CANCEL) {
        std::cout << "== Type 'y' if you want to make a re-symlink manaully again."  << std::endl;
        std::cout << "== Type 'n' to move back the previous."  << std::endl;
    }

    std::cin >> str_input;
    if(str_input == "y" || str_input == "Y") {
        return true;
    } else if(str_input == "n" || str_input == "N") {
        return false;
    }

    return false;
}


bool Manager::singleMode() {
    /// give options to the user
    std::string str_input = this->inputList("detect");
    if(str_input.empty()) {
        std::cerr << "inputList error\n";
        return false;
    }
    
    /// detect usb device
    bool res = this->detectUsb();
    if(!res) {
        std::cerr << "detectUsb error\n";
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
    std::cout << "\vvendor_id: " << this->ttyUdevInfo->vendor_id << std::endl;
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


bool Manager::allDetectMode() {
    std::cout << "all detect mode" << std::endl;

    bool res = this->detectUsbs();
    if(!res) {
        std::cerr << "detectUsbs error\n";
        ///TODO: test onlY
        // return false;
    }

    ///1. 전체를 makeUdevRule 실행하거나

    ///2. 그룹별로 진행
    // 유저 반응에 따라 
    std::cout << "\n\nTEST DUMMY INFO" << std::endl;
    while(true) {
        /// 1. 새로 만들어진 map을 통해서 진행 (product_category (vendor or model 정보)), 첫 시도만 만듬
        this->mUsbInfoConfirmer.makeCopyUdevInfoByVendor();

        /// 2. 원하는 vendor db 로 순차적으로 진행
        // makeCopyUdevInfoByVendor() 의 마지막 프린트 참고

        /// 기존 - 시리얼 관련 워닝 메세지 프린트
        // / 기존 - makeUdevRule 실행    
        // /// make a file under /etc/udev/... 
        int result = this->makeUdevRuleByProductCategory();
        if(result != 0) {
            std::cerr << "\n== Aborted or Failed to write the udev rule ==\n\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }
        std::cout << "\n== Copy complete!! ==\n\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }

    return false;
}


bool Manager::detectUsb() {
    ResultData resultData;
    bool is_acm_detected = false; //default

    /// step 1. find new device - USB, ACM
    for(int i=0; i<2; i++) {
        resultData = this->mUsbInfoConfirmer.findNewDevice(i, Mode::LAST_DETECT_MODE);
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
    this->ttyUdevInfo->vendor_id = this->mUsbInfoConfirmer.getIdsAterRegex(res_vender_id);
    this->ttyUdevInfo->product = this->mUsbInfoConfirmer.getIdsAterRegex(res_model_id);
    this->ttyUdevInfo->serial = this->mUsbInfoConfirmer.getIdsAterRegex(res_serial_id);
    std::cout << "extracted vendor_id : " << this->ttyUdevInfo->vendor_id  << std::endl;
    std::cout << "extracted model_id : " << this->ttyUdevInfo->product  << std::endl;
    std::cout << "extracted serial_id : " << this->ttyUdevInfo->serial << std::endl;

    return true;
}


bool Manager::detectUsbs() {
    std::cout << "TEST detectUsbssss()\n";
    ResultData resultData;
    bool is_acm_detected = false; //default
    int failure_cnt =0;

    /// step 1. find new device - USB, ACM
    for(int i=0; i<2; i++) {
        ///vector
        resultData = this->mUsbInfoConfirmer.findNewDevice(i, Mode::ALL_DETECT_MODE);
        // std::cout << "return result string: " << resultData.result_str << std::endl;
        // auto& v_data = resultData.result_v;
        // if(!v_data.empty()) {
        //     for(auto& str : v_data) {
        //         std::cout << str << std::endl;
        //     }
        // }

        /// checkValidDevices 에서 usb_id 와 kernel_id 확인됨, shared_ptr<UnTtyUdevInfo> sh_un_tty_udev_info 도 만들어짐
        this->mUsbInfoConfirmer.checkValidDevices(i, resultData);
        
        bool res = this->mUsbInfoConfirmer.devicesExist(i, resultData);
        if(!res) {
            failure_cnt++;
            if(failure_cnt == 2) {
                std::cout << "All devices are not connected..." << std::endl;
                return false;
            }
        }
        
    } // for loop end

     /// step3. get vender_id, model_id, and serial_id
    this->mUsbInfoConfirmer.findUdevInfosWrapper(is_acm_detected);
    
    /// step4. now all information complete
    ///TODO: 

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

int Manager::makeUdevRuleByProductCategory() {
    std::string product_category_name = this->inputProductCategory();
    if(product_category_name.empty()) {
        return 1;
    }

    auto opt = this->mUsbInfoConfirmer.getTtyUdevInfoVec(product_category_name);
    if(!opt) {
        std::cout << "Not found by \"" << product_category_name << "\"" << std::endl;
        return 1;
    }

    // std::cout << "\n\nTEST\n";
    // this->mUsbInfoConfirmer.updateStatusMapCheckList(product_category_name, MapStatus::MAP_OK);
    // this->mUsbInfoConfirmer.updateStatusMapCheckList(product_category_name, MapStatus::MAP_OK);
    // this->mUsbInfoConfirmer.updateStatusMapCheckList(product_category_name, MapStatus::MAP_OK);
    // return 1;

    std::vector<TtyUdevInfo>& v_tty_udev = opt.value();
    int v_tty_udev_size = v_tty_udev.size();
    std::cout << "Total ** " << v_tty_udev_size << " ** ttyUdevInfos have been found." << std::endl;
    while(true) {
        if(v_tty_udev_size == 1) {
            std::cout << "Single!" << std::endl;
            this->singleProcess(v_tty_udev, product_category_name);
        } else if(v_tty_udev_size == 2) {
            std::cout << "SWAP!" << std::endl;
            this->swapProcess(v_tty_udev, product_category_name);
        } else if(v_tty_udev_size > 2) {
            std::cout << "DO STEP BY STEP!" << std::endl;
            this->stepByStepProcess(v_tty_udev, product_category_name);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        if(!this->inputReMakeOrNot(InputCheck::RE_MAKE_UDEV)) {
            ///cancel or okay by user
            this->mUsbInfoConfirmer.updateStatusMapCheckList(product_category_name, MapStatus::MAP_OK);
            break;
        }

        /// only for swap
        if(v_tty_udev_size == 2) {
            std::cout << "swap again!" << std::endl;
            this->mUsbInfoConfirmer.updateStatusMapCheckList(product_category_name, MapStatus::SWAP_INDEX);
            this->mUsbInfoConfirmer.updateMapCheckList(product_category_name, 0);
        }
    } // while

    return 1;
}

int Manager::singleProcess(std::vector<TtyUdevInfo>& v_tty_udev, const std::string& product_category_name) {
    int final_result = -1;
    int success_cnt = 1; //fix
    auto& v = v_tty_udev.at(0); //fix
    /// ttyUdevInfo 새로 shared_ptr로 생성.
    if(!this->ttyUdevInfo) {
        // std::cout << "shared ptr ttyUdevInfo not initialized yet.\n";
        this->ttyUdevInfo = std::make_shared<TtyUdevInfo>(v);
    } else {
        this->ttyUdevInfo.reset();
        this->ttyUdevInfo = std::make_shared<TtyUdevInfo>(v);
    }
    // first, print
    std::cout << "\n( " << 1 << " ) shared_ptr: ttyUdevInfo\n";
    std::cout << "\tis_conneted: " << std::boolalpha << this->ttyUdevInfo->is_connected_now << std::endl;
    std::cout << "\tcurrent tty device num: " << this->ttyUdevInfo->tty_number << std::endl;
    std::cout << "\tkernel_id: " << this->ttyUdevInfo->kernel << std::endl;
    std::cout << "\tvendor_id: " << this->ttyUdevInfo->vendor_id << std::endl;
    std::cout << "\tproduct_id: " << this->ttyUdevInfo->product << std::endl;
    std::cout << "\tvendor: " << this->ttyUdevInfo->vendor << std::endl;
    std::cout << "\tmodel: " << this->ttyUdevInfo->model << std::endl;
    std::cout << "------------------------\n";

    // MapStatus pre_map_status = this->mUsbInfoConfirmer.getStatusFromMapChecklist(product_category_name);
    // if(pre_map_status == MapStatus::MAP_OK) {
    //     std::cout << product_category_name <<" was already done." << std::endl;
    //     if(this->inputReMakeOrNot(InputCheck::RE_MAKE_UDEV_AGAIN)) {
    //         this->mUsbInfoConfirmer.updateStatusMapCheckList(product_category_name, MapStatus::SWAP_INDEX);    
    //     } else {
    //         /// 한번 더 체크
    //         if(!this->inputReMakeOrNot(InputCheck::RE_MAKE_CANCEL)) {
    //             std::cout << "Cancel" << std::endl;
    //             return 1;
    //         }
    //         this->mUsbInfoConfirmer.updateStatusMapCheckList(product_category_name, MapStatus::MAP_DEFAULT);
    //         this->mUsbInfoConfirmer.clearMapCheckListSymlink(product_category_name);
    //     }
    // }
    
    /// input list check 
    std::string str_input;
    int input_num;
    MapStatus mas_status = this->mUsbInfoConfirmer.getStatusFromMapChecklist(product_category_name);

    str_input = this->inputList("add");
    if(str_input.empty()) {
        return 1;
    }
    try {
        input_num = std::stoi(str_input);
    } catch(const std::exception& e) {
        std::cerr << "Exception error: " << e.what() << std::endl;
        return 1;
    }
    /// sylmink name 정보 업데이트
    this->mUsbInfoConfirmer.clearMapCheckListSymlink(product_category_name);
    this->mUsbInfoConfirmer.updateMapCheckList(product_category_name, input_num);
    
    ///FYI: for warning
    if(this->ptrUdevMaker->getSerialWarn(this->ttyUdevInfo) ) {
        std::cerr << "[warn]serial info not found. Please change 'use_serial' to false in the config.lua" << std::endl;
        std::cerr << "[warn]device may not be found." << std::endl;
    }
    
    ///FYTL 꼭 getSymlinkIndexFromMapChecklist 에서 set(위) 및 get 을 안해도 될 듯 하나, 일단 기존 형식으로 맞춤
    input_num = this->mUsbInfoConfirmer.getSymlinkIndexFromMapChecklist(product_category_name, 0);
    if(input_num == -1) {
        std::cerr << "Couldn't find the symlink index." << std::endl;
        return 1;
    }
    std::cout << "Symlink index from MapCheckList: " << input_num << std::endl;
    this->ptrUdevMaker->getSymlinkNameFromList(input_num);
    ///TODO: 시리얼 정보 없을 경우 커널 정보로 넣어주기
    this->ptrUdevMaker->setSymlink(input_num, this->ttyUdevInfo);

    ///TODO: test 후 주석해제
    // if(this->ptrUdevMaker->getIsPolicyKitNeeded()) {   
    //     final_result = this->ptrUdevMaker->createUdevRuleFileWithFork(this->ttyUdevInfo);
    //     if(final_result == 0) {
    //         std::vector<std::string> cmd_result_data;
    //         std::string cmd = "udevadm control --reload-rules; udevadm trigger";
    //         this->mUsbInfoConfirmer.executeCmd(cmd_result_data, cmd, ResultType::EXECUTE_ONLY);
    //         success_cnt--;
    //     }

    // } else {
    //     // 또는 직접 /etc쪽에 만들어주기 - permission 때문에 stdin 방식으로 해결
    //     // return this->ptrUdevMaker->createUdevRuleFile(this->ttyUdevInfo);
    //     final_result = this->ptrUdevMaker->createUdevRuleFile(this->ttyUdevInfo);
    //     if(final_result == 0) {
    //         success_cnt--;
    //     }
    // }

    success_cnt--;

    if(success_cnt == 0) {
        return final_result;
    }
    return 1;
}

int Manager::swapProcess(std::vector<TtyUdevInfo>& v_tty_udev, const std::string& product_category_name) {
    int i=0;
    int final_result = -1;
    int success_cnt = v_tty_udev.size();
    for(auto& v : v_tty_udev) {
        /// ttyUdevInfo 새로 shared_ptr로 생성.
        if(!this->ttyUdevInfo) {
            // std::cout << "shared ptr ttyUdevInfo not initialized yet.\n";
            this->ttyUdevInfo = std::make_shared<TtyUdevInfo>(v);
        } else {
            this->ttyUdevInfo.reset();
            this->ttyUdevInfo = std::make_shared<TtyUdevInfo>(v);
        }
        // first, print

        std::cout << "\n( " << i+1 << " ) shared_ptr: ttyUdevInfo\n";
        std::cout << "\tis_conneted: " << std::boolalpha << this->ttyUdevInfo->is_connected_now << std::endl;
        std::cout << "\tcurrent tty device num: " << this->ttyUdevInfo->tty_number << std::endl;
        std::cout << "\tkernel_id: " << this->ttyUdevInfo->kernel << std::endl;
        std::cout << "\tvendor_id: " << this->ttyUdevInfo->vendor_id << std::endl;
        std::cout << "\tproduct_id: " << this->ttyUdevInfo->product << std::endl;
        std::cout << "\tvendor: " << this->ttyUdevInfo->vendor << std::endl;
        std::cout << "\tmodel: " << this->ttyUdevInfo->model << std::endl;
        std::cout << "------------------------\n";

        MapStatus pre_map_status = this->mUsbInfoConfirmer.getStatusFromMapChecklist(product_category_name);
        if(pre_map_status == MapStatus::MAP_OK) {
            std::cout << product_category_name <<" was already done." << std::endl;
            if(this->inputReMakeOrNot(InputCheck::RE_MAKE_UDEV_AGAIN)) {
                this->mUsbInfoConfirmer.updateStatusMapCheckList(product_category_name, MapStatus::SWAP_INDEX);    
            } else {
                /// 한번 더 체크
                if(!this->inputReMakeOrNot(InputCheck::RE_MAKE_CANCEL)) {
                    std::cout << "Cancel" << std::endl;
                    return 1;
                }
                this->mUsbInfoConfirmer.updateStatusMapCheckList(product_category_name, MapStatus::MAP_DEFAULT);
                this->mUsbInfoConfirmer.clearMapCheckListSymlink(product_category_name);
            }
        }
        
        /// input list check 
        std::string str_input;
        int input_num;
        MapStatus mas_status = this->mUsbInfoConfirmer.getStatusFromMapChecklist(product_category_name);

        if(mas_status != MapStatus::SWAP_INDEX) {
            str_input = this->inputList("add");
            if(str_input.empty()) {
                return 1;
            }
            try {
                input_num = std::stoi(str_input);
            } catch(const std::exception& e) {
                std::cerr << "Exception error: " << e.what() << std::endl;
                return 1;
            }
            /// sylmink name 정보 업데이트
            this->mUsbInfoConfirmer.updateMapCheckList(product_category_name, input_num);
        }
        
        ///FYI: for warning
        if(this->ptrUdevMaker->getSerialWarn(this->ttyUdevInfo) ) {
            std::cerr << "[warn]serial info not found. Please change 'use_serial' to false in the config.lua" << std::endl;
            std::cerr << "[warn]device may not be found." << std::endl;
        }
        
        /// auto 면 setSymlink 이름 다시 정해주기
        if(mas_status == MapStatus::SWAP_INDEX) {
            /// swap 된 정보 받아오기
            input_num = this->mUsbInfoConfirmer.getSymlinkIndexFromMapChecklist(product_category_name, i);
            if(input_num == -1) {
                std::cerr << "Couldn't find the symlink index." << std::endl;
                continue;
            }
            std::cout << "Symlink index from MapCheckList: " << input_num << std::endl;
            this->ptrUdevMaker->getSymlinkNameFromList(input_num);
        }
        ///TODO: 시리얼 정보 없을 경우 커널 정보로 넣어주기
        this->ptrUdevMaker->setSymlink(input_num, this->ttyUdevInfo);

        ///TODO: test 후 주석해제
        // if(this->ptrUdevMaker->getIsPolicyKitNeeded()) {   
        //     final_result = this->ptrUdevMaker->createUdevRuleFileWithFork(this->ttyUdevInfo);
        //     if(final_result == 0) {
        //         std::vector<std::string> cmd_result_data;
        //         std::string cmd = "udevadm control --reload-rules; udevadm trigger";
        //         this->mUsbInfoConfirmer.executeCmd(cmd_result_data, cmd, ResultType::EXECUTE_ONLY);
        //         success_cnt--;
        //     }

        // } else {
        //     // 또는 직접 /etc쪽에 만들어주기 - permission 때문에 stdin 방식으로 해결
        //     // return this->ptrUdevMaker->createUdevRuleFile(this->ttyUdevInfo);
        //     final_result = this->ptrUdevMaker->createUdevRuleFile(this->ttyUdevInfo);
        //     if(final_result == 0) {
        //         success_cnt--;
        //     }
        // }

        i++;
        success_cnt--;
    } // For loop ends here

    if(success_cnt == 0) {
        return final_result;
    }
    return 1;
}

int Manager::stepByStepProcess(std::vector<TtyUdevInfo>& v_tty_udev, const std::string& product_category_name) {
    std::cout << "Step by Step\n";
    int i=0;
    int final_result = -1;
    int success_cnt = v_tty_udev.size();
    while(v_tty_udev.size() > 0) {
        /// ttyUdevInfo 새로 shared_ptr로 생성.
        auto& v = v_tty_udev.at(i);
        if(!this->ttyUdevInfo) {
            // std::cout << "shared ptr ttyUdevInfo not initialized yet.\n";
            this->ttyUdevInfo = std::make_shared<TtyUdevInfo>(v);
        } else {
            this->ttyUdevInfo.reset();
            this->ttyUdevInfo = std::make_shared<TtyUdevInfo>(v);
        }
        // first, print

        std::cout << "\n( " << i+1 << " ) shared_ptr: ttyUdevInfo\n";
        std::cout << "\tis_conneted: " << std::boolalpha << this->ttyUdevInfo->is_connected_now << std::endl;
        std::cout << "\tcurrent tty device num: " << this->ttyUdevInfo->tty_number << std::endl;
        std::cout << "\tkernel_id: " << this->ttyUdevInfo->kernel << std::endl;
        std::cout << "\tvendor_id: " << this->ttyUdevInfo->vendor_id << std::endl;
        std::cout << "\tproduct_id: " << this->ttyUdevInfo->product << std::endl;
        std::cout << "\tvendor: " << this->ttyUdevInfo->vendor << std::endl;
        std::cout << "\tmodel: " << this->ttyUdevInfo->model << std::endl;
        std::cout << "------------------------\n";

        /// input list check 
        std::string str_input;
        int input_num;
        MapStatus map_status = this->mUsbInfoConfirmer.getStatusFromMapChecklist(product_category_name);

        if(map_status != MapStatus::SWAP_INDEX) {
            str_input = this->inputList("add");
            if(str_input.empty()) {
                continue;
            }
            try {
                input_num = std::stoi(str_input);
            } catch(const std::exception& e) {
                std::cerr << "Exception error: " << e.what() << std::endl;
                return 1;
            }
            /// sylmink name 정보 업데이트
            this->mUsbInfoConfirmer.updateMapCheckList(product_category_name, input_num);
        }
        
        ///FYI: for warning
        if(this->ptrUdevMaker->getSerialWarn(this->ttyUdevInfo) ) {
            std::cerr << "[warn]serial info not found. Please change 'use_serial' to false in the config.lua" << std::endl;
            std::cerr << "[warn]device may not be found." << std::endl;
        }
        
        ///TODO: 시리얼 정보 없을 경우 커널 정보로 넣어주기
        this->ptrUdevMaker->setSymlink(input_num, this->ttyUdevInfo);

        ///TODO: test 후 주석해제
        // if(this->ptrUdevMaker->getIsPolicyKitNeeded()) {   
        //     final_result = this->ptrUdevMaker->createUdevRuleFileWithFork(this->ttyUdevInfo);
        //     if(final_result == 0) {
        //         std::vector<std::string> cmd_result_data;
        //         std::string cmd = "udevadm control --reload-rules; udevadm trigger";
        //         this->mUsbInfoConfirmer.executeCmd(cmd_result_data, cmd, ResultType::EXECUTE_ONLY);
        //         success_cnt--;
        //     }

        // } else {
        //     // 또는 직접 /etc쪽에 만들어주기 - permission 때문에 stdin 방식으로 해결
        //     // return this->ptrUdevMaker->createUdevRuleFile(this->ttyUdevInfo);
        //     final_result = this->ptrUdevMaker->createUdevRuleFile(this->ttyUdevInfo);
        //     if(final_result == 0) {
        //         success_cnt--;
        //     }
        // }

        if(!this->inputReMakeOrNot(InputCheck::RE_SYMLINK)) {
            i++;
            success_cnt--;
            if(i == v_tty_udev.size()) {
                std::cout << "Now stepbyStepProcess complete!" << std::endl;
                ///TODO: 이제 실제 symlink 연결해주는 코드 추가하기
                break;
            }

        } else {
            std::cout << "make a re-symlink again" << std::endl;
        }
    } // While Loop ends here

    if(success_cnt == 0) {
        return final_result;
    }
    return 1;
}



bool Manager::inputMode() {
    ///FYI: use this in GUI mode only
    // this->inputSymlinkInManualMode();

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
    bool fin_result = this->removeUdevRule(input_num);
    if(fin_result != 0) {
        std::cerr << "\n== Failed to delete the udev rule. ==\n\n";
        std::cout << "error code: " << fin_result << std::endl;
        return false;
    }

    std::cout << "Please re-launch the program if you want to use it again. Thank you." << std::endl;
    return true;
}

int Manager::removeUdevRule(int input_num) {
    this->ptrUdevMaker->setSymlink(input_num, this->ttyUdevInfo);

    if(this->ptrUdevMaker->getIsPolicyKitNeeded()) {   
        int res = this->ptrUdevMaker->deleteUdevRuleFileWithFork(this->ttyUdevInfo);
        if(res == 0) {
            std::vector<std::string> cmd_result_data;
            std::string cmd = "udevadm control --reload-rules; udevadm trigger";
            this->mUsbInfoConfirmer.executeCmd(cmd_result_data, cmd, ResultType::EXECUTE_ONLY);
            return 0;
        }
        return res;

    } else {
        // 또는 직접 /etc쪽에 만들어주기 - permission 때문에 stdin 방식으로 해결
        return this->ptrUdevMaker->deleteUdevruleFile();
    }
    
}