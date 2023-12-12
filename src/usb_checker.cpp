#include "usb_checker.h"

UsbChecker::UsbChecker(UdevMaker* udevMaker) : ptrUdevMaker(udevMaker) { }

std::string UsbChecker::findNewDevice() {
    // std::cout << dmesg_cmd << std::endl;
    
    std::vector<std::string> v_dmesg_data;
    this->getCmdResult(v_dmesg_data, DMESG);

    // 리턴 받은 값으로 비교 후 마지막 값을 ttyUSB 인지 확인 후 마지막 장치 번호를 리턴해준다.

    return v_dmesg_data.at(0);
}


std::vector<std::string> UsbChecker::findUdevInfo() {
    std::vector<std::string> v_udev_data;
    this->getCmdResult(v_udev_data, UDEVADM);

    return v_udev_data;
}


void UsbChecker::getCmdResult(std::vector<std::string>& cmd_data, int cmd_type) {
    FILE *fp;
    char var[100];

    std::string cmd_str;
    if(cmd_type == Cmd::UDEVADM) { 
        cmd_str = this->m_udev_cmd + this->getUsbId(); // 최초 DMESG 일 경우 마지막 인덱스로 장치 넘버를 구해놓음
    } else {
        cmd_str = this->m_dmesg_cmd;
    }

    fp = popen(cmd_str.c_str(), "r");
    while(fgets(var, sizeof(var), fp) != NULL) {
        // std::cout << var; // 프린트 확인 시
        if(cmd_type == Cmd::UDEVADM) {   // 결과가 많이 필요하므로 UDEVADM 일 경우는 계속 push_back()
            cmd_data.push_back(var);
        }
    }

    /// DMESG 일 경우에는 마지막 결과만 만들어서 넘김
    if(cmd_type == Cmd::DMESG) { // DMESG
        cmd_data.push_back(var);
        
        /// usb device 넘버 찾기 - 끝자리만 찾음.
        for(int i=0; i < sizeof(var); ++i) {
            if(var[i] == '\0') {
                this->m_usb_num_str = var[i - 2]; // '/0' 만큼 뺴주기
                // std::cout << "found last index: " << i << std::endl;
                // std::cout << "usb device id: " << this->m_usb_num_str << std::endl;
                break;
            }
        }
    }

    pclose(fp);
}


std::string UsbChecker::getUsbId() {
    return this->m_usb_num_str;
}

std::string UsbChecker::regexWrapper(std::string& last_message, std::string reg_str) {
    std::regex reg(reg_str);
    std::smatch mat;

    // std::regex_search(last_message, mat, reg);  // reg matching 할 때 마지막 인자를 string으로 주면 안됨
    bool result = std::regex_search(last_message, mat, reg);
    std::string return_str;

    if(result) {
        // std::cout << "result : " << result << std::endl;
        for(auto x: mat) {
            // std::cout << "reg found " << std::endl;
            return_str = x;
        }
    }

    return return_str;
}


std::string UsbChecker::getKernelId(std::string& last_message) {
    std::string reg_str = "(usb \\d-.+:)";
    std::string return_data;
    std::string result = this->regexWrapper(last_message, reg_str);

    if(result.empty()) {
        return return_data;
    }

    /// 단순하게 정규식 이후 usb 로 시작하는 것 제거, 마지막 인덱스도 제외; 다시 스트링 만들어서 리턴함
    for(int i=3; i < result.size() -1; ++i) {
        return_data += result[i];
    }

    removeCharacter(return_data, ' ');
    return return_data;
}


std::string UsbChecker::getVenderId(std::string& last_message) {
    // vendor id 뽑기
    // ID_VENDOR_ID=
    std::string reg_str = "(ID_VENDOR_ID=)";
    
    std::string return_data = this->regexWrapper(last_message, reg_str);

    return return_data;
}


std::string UsbChecker::getModelId(std::string& last_message) {
    // model id 추출
    // ID_MODEL_ID
    std::string reg_str = "(ID_MODEL_ID=)";

    std::string return_data = this->regexWrapper(last_message, reg_str);

    return return_data;
}


/// @brief  vendor id or model id can be extracted here
/// @param last_message 
/// @return 
std::string UsbChecker::getIdsAterRegex(std::string& last_message) {
    // 실제 model id 추출
    std::string return_data;
    size_t last_index = last_message.find_first_of('=');
    // std::cout << last_index << std::endl;
    // std::cout << last_message.at(last_index) << std::endl;
    for(int i=last_index+1; i < last_message.size(); ++i) {
        // std::cout << last_message.at(i);
        return_data += last_message.at(i);
    }
    std::cout << std::endl;

    // newline이 포함되어 있어서 제거 해준다.
    removeCharacter(return_data, '\n');
    return return_data;
}


void UsbChecker::removeCharacter(std::string& str, char remove_char) {
    str.erase(std::remove(str.begin(), str.end(), remove_char), str.cend());
}


bool UsbChecker::checkNumber(std::string& input_msg) {
    std::string reg_str = "[0-9]";
    std::string return_data;
    std::string result_str = this->regexWrapper(input_msg, reg_str);
    if(result_str.empty()) {
        return false;
    }
    return true;
}

bool UsbChecker::detectUsb() {

    std::string var = this->findNewDevice();

    std::cout << "return var: " << var << std::endl;
    if(this->getKernelId(var).empty()) {
        std::cout << "USB might be disconnected. Try it again.\n";
        return false;
    } 

    ptrUdevMaker->m_kennel = this->getKernelId(var);
    std::cout << "kernel id: " << ptrUdevMaker->m_kennel << std::endl;
    std::cout << "usb id: " <<  this->getUsbId() << std::endl;

    std::vector<std::string> v_str = this->findUdevInfo();
    std::string res_vender_id, res_model_id;
    for(int i=0; i< v_str.size(); ++i) {
        // res_vender_id = usbChecker.getVenderId(v_str[i]);
        if(this->getVenderId(v_str[i]).empty() == false) {
            res_vender_id = v_str[i];
        } 
        if(this->getModelId(v_str[i]).empty() == false) {
            res_model_id = v_str[i];
        }

        if(!res_vender_id.empty() && !res_model_id.empty()) {
            // std::cout << "break!" << std::endl;
            break;
        }
    }

    // std::cout << "vendor_id : " << res_vender_id << std::endl;
    // std::cout << "model_id : " << res_model_id << std::endl;

    std::cout << "extracted vendor_id : " << this->getIdsAterRegex(res_vender_id) << std::endl;
    std::cout << "extracted model_id : " << this->getIdsAterRegex(res_model_id) << std::endl;

    ptrUdevMaker->m_vendor = this->getIdsAterRegex(res_vender_id);
    ptrUdevMaker->m_product = this->getIdsAterRegex(res_model_id);

    return true;
}