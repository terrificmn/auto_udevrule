#include "usb_info_confirmer.hpp"

UsbInfoConfirmer::UsbInfoConfirmer(UdevMaker* udevMaker) : ptrUdevMaker(udevMaker) { }

ResultData UsbInfoConfirmer::findNewDevice(const int& try_count) {
    std::vector<std::string> v_result_data;
    ResultData resultData;

    std::string cmd_str;
    if(try_count == 0) { // FIND_USB_DMESG
        /// FYI:이전 버전에서는 sudo 없이 사용이 되었으나, dmesg명령어 사용시 permit error 발생 -- on Mar26, 2024
        // TODO: 커널 6.7.4 기준으로는 sudo가 필요
        resultData.tty_device = TTYDevice::USB;
        cmd_str = this->m_dmesg_base_cmd + "USB";

    } else if(try_count == 1) { //CmdCase::FIND_ACM_DMESG;
        resultData.tty_device = TTYDevice::ACM;
        cmd_str = this->m_dmesg_base_cmd + "ACM";
    }

    // std::cout << "cmd_str: " << cmd_str << std::endl;
    /// TODO: 최초는 DMESG_SEARCH 이지만, UDEVINFO 찾을 경우에 대해서 업데이트 필요
    this->executeCmd(v_result_data, cmd_str, ResultType::LAST_RESULT_ONLY);

    // 리턴 받은 값으로 비교 후 마지막 값을 ttyUSB 인지 확인 후 마지막 장치 번호를 리턴해준다.
    // std::cout << "--------------------------------------\n";
    // std::cout << "v_result_data size: " << v_result_data.size() << std::endl;
    // for(auto& result_str : v_result_data) {
    //     std::cout << result_str << std::endl;
    // }
    // std::cout << "--------------------------------------\n";


    if(v_result_data.size() == 0) {
        resultData.result_str = std::string(); //empty
        // std::cout << "No result found. ";
        return resultData;
    }

    resultData.result_str = std::move(v_result_data.at(0));

    /// usb device 넘버 찾기 - 끝자리만 찾음.
    if(resultData.tty_device == TTYDevice::USB) {
        size_t find_index = resultData.result_str.find("ttyUSB");
        if(find_index != std::string::npos) {
            this->m_usb_num_str = resultData.result_str.substr(find_index+6, 1); // extract the last index 
            std::cout << "usb device id: " << this->m_usb_num_str << std::endl;
        }
        
    } else if(resultData.tty_device == TTYDevice::ACM) {
        size_t find_index = resultData.result_str.find("ttyACM");
        if(find_index != std::string::npos) {
            this->m_usb_num_str = resultData.result_str.substr(find_index+6, 1);
            std::cout << "usb device id: " << this->m_usb_num_str << std::endl;
        }
    } else {
        this->m_usb_num_str = "-1";
    }

    /// to assign to found device number
    try {
        resultData.found_device_num = stoi(this->m_usb_num_str);
    } catch (const std::exception& e) {
        resultData.found_device_num = -1;
        std::cerr << "Exception error. " << e.what() << std::endl;
    }

    return resultData;
}

std::vector<std::string> UsbInfoConfirmer::findUdevInfo(const bool& is_acm_detected) {
    std::string usb_id = this->getUsbId();
    // std::cout << "detected id: " << usb_id << std::endl; // acm 포함
    
    std::vector<std::string> v_udev_result_data;
    std::string concanated_cmd;
    if(is_acm_detected) {
        concanated_cmd = this->m_udev_base_cmd + "ACM" + usb_id;
    } else {
        concanated_cmd = this->m_udev_base_cmd + "USB" + usb_id;
    }

    this->executeCmd(v_udev_result_data, concanated_cmd, ResultType::WHOLE_RESULT);

    return std::move(v_udev_result_data);
}

/// @brief TEST is needed. Not used currently.
/// @param cmd_str 
/// @return 
bool UsbInfoConfirmer::executePopen(const std::string& cmd_str) {
    char var[256];
    std::array<char, 128> buffer;
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd_str.c_str(), "r"), pclose);
    if(!pipe) {
        std::cerr << "popen() failed!" << std::endl;
    }

    while(fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        // cmd_result_data.push_back(var);
        result = buffer.data();
    }
    
    if(!result.empty()) {
        std::cout << "ls result: " << result << std::endl;
        
    }
    return true;
}

bool UsbInfoConfirmer::executeSimpleCmd(const std::string& cmd_str) {
    int outPipe[2]; //stdout
    int errPipe[2]; //stderr
    if(pipe(outPipe) == -1) {
        std::cerr << "stdout pipe error: -1" << std::endl;
        return false;
    }
    if(pipe(errPipe) == -1) {
        std::cerr << "stderr pipe error: -1" << std::endl;
        return false;
    }

    pid_t pid = fork();

    if(pid == -1) {
        close(outPipe[0]);
        close(outPipe[1]);
        close(errPipe[0]);
        close(errPipe[1]);
        std::cerr << "pipe error: -1. all closed" << std::endl;
        return false;
    }
    
    /// child process
    if(pid == 0) {
        ///FYI: 2개의 stdout, stderr 를 확인할 수 있게 준비. popen 으로는 stderr를 확인할 수 없음.(아예 불가능한것은 아님)
        close(outPipe[0]); /// Read close
        dup2(outPipe[1], STDOUT_FILENO);

        close(errPipe[0]);
        dup2(errPipe[1], STDERR_FILENO);

        close(outPipe[1]);
        close(errPipe[1]);

        /// 여기는 sudo가 필요 없어서 괜찮을 듯 함.
        execl("/bin/sh", "sh", "-c", cmd_str.c_str(), (char*)0);

        // child 종료
        _exit(1);
    }

    /// parent process
    close(outPipe[1]); // write close
    close(errPipe[1]); // write close

    char buffer[256];
    bool is_error = false;
    /// for stdout
    while(read(outPipe[0], buffer, sizeof(buffer)) > 0) {
        std::cout << "[stdout]: " << buffer;
    }

    /// for stderr
    std::string err_str;
    while(read(errPipe[0], buffer, sizeof(buffer)) > 0) {
        std::cout << "[stderr]: " << buffer;
        err_str = buffer;
    }

    if(err_str.find("cannot access") != std::string::npos) {
        // std::cout << "found cannot access\n";
        is_error = true;
    }

    close(outPipe[0]);
    close(errPipe[0]);

    waitpid(pid, nullptr, 0);

    if(is_error) {
        return false;
    }
    return true;
}

/// @brief execute cmd ; Device has dectected then we can get the device name but It can't be decided at this point if the device is active or not.
/// @param cmd_result_data 
/// @param cmd_str 
/// @param process_result_type 
void UsbInfoConfirmer::executeCmd(std::vector<std::string>& cmd_result_data, const std::string& cmd_str, int process_result_type) {
    std::cout << "command: " << cmd_str << std::endl;
    int pipefd[2];
    if(pipe(pipefd) == -1) {
        std::cerr << "pipe error: -1" << std::endl;
        return;
    }

    pid_t pid = fork();

    if(pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        std::cerr << "pipe error after fork: -1" << std::endl;
        return;
    }

    /// for child process
    if(pid == 0) {
        close(pipefd[0]); // close read end

        // redirect stdout to pipe
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        /// for test ONLY
        const char* program_exec = this->ptrUdevMaker->getIsPolicyKitNeeded() ? "pkexec" : "sudo";
        
        // prepare arguments
        const char* args[] = {
            program_exec,
            "bash",
            "-c",
            cmd_str.c_str(),
            nullptr
        };

        // execute without shell wrapper
        execvp(program_exec, const_cast<char* const*>(args));

        // if execvp fails
        _exit(1);
    }

    /// for parent process
    close(pipefd[1]); // close write end
    std::cout << "parent processing...\n";

    char buffer[256];
    ssize_t count;
    std::string result;

    while((count = read(pipefd[0], buffer, sizeof(buffer) -1)) > 0) {
        buffer[count] = '\0'; // add null-terminate : 한번 읽을 때 바이트 수 만큼 읽어오는데, 마지막 인데스에 null-terminate 를 해줌
        ///TODO: need TEST for WHOLE_RESULT -- 어차피 result 는 만들어야 한다.
        // if(process_result_type == ResultType::WHOLE_RESULT) {
        //     // 결과가 많이 필요하므로 UDEVADM 일 경우는 계속 push_back()
        //     // cmd_result_data.push_back(buffer);
        //     result += buffer;
        // }
        result += buffer;
        // std::cout << "-- read once\n";
    }

    /// Last result count -1 --> error
    if(count == -1) {
        perror("read");
    }

    // std::cout << "result size: " << result.size() << ", result: \n" << result << std::endl;
    /// 처리 후

    if(!result.empty() && process_result_type == ResultType::LAST_RESULT_ONLY) {
        /// DMESG_SEARCH 일 경우에는 마지막 결과만 만들어서 넘김. var has the last result.
        /// 마지막 읽은 buffer, 256 사이즈를 안넘기면 result와 같은 결과 일 수 있다.
        size_t pos = result.rfind('\n');
        if(pos != std::string::npos) {
            /// meaning there is '\n' at the end of the line
        }

        std::string last_line;
        if(pos == result.size() -1) { /// meaning '\n's pos is at end of the line.
            // exclude literally the last '\n' and re-rfind from there again.
            size_t re_pos = result.rfind('\n', pos-1);
            if(re_pos != std::string::npos) {
                std::cout << "the second \\n(reversed) found at : " << re_pos << std::endl;
                last_line = result.substr(re_pos+1);
                std::cout << "Last line:\n" << last_line;
            }
        } else {
            std::cout << "last \\0(\\n) at index: " << pos << std::endl;
            last_line = result.substr(pos+1);
            std::cout << "Last line:\n" << last_line;
        }

        cmd_result_data.push_back(last_line);
    }
    // else if(!result.empty() && process_result_type == ResultType::WHOLE_RESULT) {
    //     //??
    // }
    
    close(pipefd[0]);

    /// wait for child
    int status;
    waitpid(pid, &status, 0);
    std::cout << "Parent done:)" << std::endl;
}


std::string UsbInfoConfirmer::getUsbId() {
    return this->m_usb_num_str;
}

std::string UsbInfoConfirmer::regexWrapper(std::string& last_message, std::string reg_str) {
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

/// @brief find kernel id from string. example "usb 1-10.3: ...."
/// @param last_message 
/// @return 
std::string UsbInfoConfirmer::getKernelId(std::string& last_message) {
    /// 
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

    this->removeCharacter(return_data, ' ');
    return return_data;
}

/// @brief Retrieve time which at is detected or detached. 
/// @param last_message 
/// @return 
std::string UsbInfoConfirmer::getDetectedTime(std::string& last_message) {
    std::string reg_str = "\\d.+?\\]"; /// 처음 리눅스 시간만 가져오기 마지막 ] 까지 
    std::string return_data;
    std::string result = this->regexWrapper(last_message, reg_str);

    if(result.empty()) {
        return return_data;
    }

    // std::cout << "regex result: " << result << std::endl;

    /// 정규식 이후 ']' 까지 포함 되어 있으므로 ] 빼주기
    return_data = std::move(result.substr(0, result.size()-1));
    // std::cout << "new regex result: " << return_data << std::endl;

    return std::move(return_data);
}

std::string UsbInfoConfirmer::getKernelIdForAcm(std::string& last_message) {
    /// 
    std::string reg_str = "\\d-.+?:";
    std::string return_data;
    std::string result = this->regexWrapper(last_message, reg_str);

    if(result.empty()) {
        return return_data;
    }

    // std::cout << "regex result: " << result << std::endl;

    /// 정규식 이후 : 까지 포함 되어 있으므로 : 빼주기
    return_data = std::move(result.substr(0, result.size()-1));
    // std::cout << "new regex result: " << return_data << std::endl;

    return return_data;
}

std::string UsbInfoConfirmer::getVenderId(std::string& last_message) {
    // vendor id 뽑기
    // ID_VENDOR_ID=
    std::string reg_str = "(ID_VENDOR_ID=)";
    
    std::string return_data = this->regexWrapper(last_message, reg_str);

    return return_data;
}


std::string UsbInfoConfirmer::getModelId(std::string& last_message) {
    // model id 추출
    // ID_MODEL_ID
    std::string reg_str = "(ID_MODEL_ID=)";

    std::string return_data = this->regexWrapper(last_message, reg_str);

    return return_data;
}

std::string UsbInfoConfirmer::getSerialId(std::string& last_message) {
    // serial 넘버 추출
    // ID_SERIAL_SHORT
    std::string reg_str = "(ID_SERIAL_SHORT=)";

    std::string return_data = this->regexWrapper(last_message, reg_str);

    return return_data;
}


/// @brief  vendor id or model id can be extracted here
/// @param last_message 
/// @return 
std::string UsbInfoConfirmer::getIdsAterRegex(std::string& last_message) {
    // 실제 model id 추출
    std::string return_data;
    size_t last_index = last_message.find_first_of('=');
    // std::cout << last_index << std::endl;
    // std::cout << last_message.at(last_index) << std::endl;
    for(int i=last_index+1; i < last_message.size(); ++i) {
        // std::cout << last_message.at(i);
        return_data += last_message.at(i);
    }
    // std::cout << "Id found." << std::endl;

    // newline이 포함되어 있어서 제거 해준다.
    this->removeCharacter(return_data, '\n');
    return return_data;
}


void UsbInfoConfirmer::removeCharacter(std::string& str, char remove_char) {
    str.erase(std::remove(str.begin(), str.end(), remove_char), str.cend());
}


bool UsbInfoConfirmer::checkNumber(std::string& input_msg) {
    std::string reg_str = "[0-9]";
    std::string return_data;
    std::string result_str = this->regexWrapper(input_msg, reg_str);
    if(result_str.empty()) {
        return false;
    }
    return true;
}

int UsbInfoConfirmer::showResult() {
    ///DEBUG
    std::cout << "symlink name " << this->ptrUdevMaker->udevInfo.symlink_name << std::endl;
    // std::cout << "rule file name: " << this->ptrUdevMaker->getUdevRuleFilename() << std::endl;

    std::string cmd = "ls -l /dev/" + this->ptrUdevMaker->udevInfo.symlink_name;
    // std::cout << "symlink cmd: " << cmd << std::endl;

    FILE *fp;
    char var[100];

    fp = popen(cmd.c_str(), "r");
    while(fgets(var, sizeof(var), fp) != NULL) {
        // std::cout << var; // 프린트 확인 시
    }

    std::string show_str = var;
    if(show_str.empty()) {
        std::cerr << "something goes wrong: can't find the udev rule file." << std::endl;
        return -2;
    }
    int result = pclose(fp);
    if(result == -1) {
        perror("pclose failed");
        return -1;
    } else {
        if (WIFEXITED(result)) {
            int exit_code = WEXITSTATUS(result);
            if (exit_code == 0) {
                /// set 
                this->ls_rule_result = std::move(show_str);
            } else {
                std::cerr << "The final result exited with error code: " << exit_code << std::endl;
                ///FYI: exit_code Non-zero (1-255)
                return exit_code;
            }
        } else {
            std::cerr << "The command(show result) did not terminate normally (e.g., killed by a signal)." << std::endl;
            return -3;
        }
    }

    return 0;
}

std::string UsbInfoConfirmer::getLsResult() {
    return this->ls_rule_result;
}

bool UsbInfoConfirmer::detectUsb() {
    ResultData resultData;
    bool is_acm_detected = false; //default

    /// step 1. find new device - USB, ACM
    for(int i=0; i<2; i++) {
        resultData = this->findNewDevice(i);
        // std::cout << "return result string: " << resultData.result_str << std::endl;

        /// 1. time check
        bool detected_time_result = false;
        std::string detected_t_str = this->getDetectedTime(resultData.result_str);
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
                bool res = this->executeSimpleCmd(cmd);
                if(!res) {
                    std::cout << "ttyUSB" << resultData.found_device_num << "not connected." << std::endl;
                    continue; /// still have one left
                }
            }

            /// 2-2. find kernel id
            if(this->getKernelId(resultData.result_str).empty() == false && detected_time_result == true) {
                std::cout << "Okay. Found the USB device" << std::endl;
                break;
            } else {
                std::cout << "Not Found the device for ttyUSB." << std::endl;
            }
        
        } else if(i == 1) { // second try
            /// 3-1, check if the device number exists
            if(resultData.found_device_num != -1) {
                std::string cmd = "ls /dev/ttyACM" + std::to_string(resultData.found_device_num);
                bool res = this->executeSimpleCmd(cmd);
                if(!res) {
                    std::cout << "ttyACM" << resultData.found_device_num << " is not connected." << std::endl;
                    return false;
                }
            }
            /// 3-2. find kernel id
            // if(this->getKernelIdForAcm(resultData.result_str).empty() == false && detected_time_result == true) { // the last try
            if(this->getKernelIdForAcm(resultData.result_str).empty() == false) { // the last try
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
    
    /// step2. get Kenrnel id if it's found.  --> a bit different method to find the kernel id between usb and acm
    if(is_acm_detected) {
        // std::cout << "ACM device detected!" << std::endl;    
        ptrUdevMaker->udevInfo.kernel = this->getKernelIdForAcm(resultData.result_str);
    
    } else {
        ptrUdevMaker->udevInfo.kernel = this->getKernelId(resultData.result_str);
    }

    // std::cout << std::endl;
    // std::cout << "kernel id: " << ptrUdevMaker->udevInfo.kernel << std::endl;
    // std::cout << "usb id: " <<  this->getUsbId() << std::endl;

    /// step3. get vender_id, model_id, and serial_id
    std::vector<std::string> v_udev_str;
    v_udev_str = this->findUdevInfo(is_acm_detected);

    std::string res_vender_id, res_model_id, res_serial_id;
    for(int i=0; i< v_udev_str.size(); ++i) {
        // res_vender_id = UsbInfoConfirmer.getVenderId(v_udev_str[i]);
        if(this->getVenderId(v_udev_str[i]).empty() == false) {
            res_vender_id = v_udev_str[i];
        } 
        if(this->getModelId(v_udev_str[i]).empty() == false) {
            res_model_id = v_udev_str[i];
        }
        if(this->getSerialId(v_udev_str[i]).empty() == false) {
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
    ptrUdevMaker->udevInfo.vendor = this->getIdsAterRegex(res_vender_id);
    ptrUdevMaker->udevInfo.product = this->getIdsAterRegex(res_model_id);
    ptrUdevMaker->udevInfo.serial = this->getIdsAterRegex(res_serial_id);

    // std::cout << "extracted vendor_id : " << ptrUdevMaker->udevInfo.vendor << std::endl;
    // std::cout << "extracted model_id : " << ptrUdevMaker->udevInfo.product  << std::endl;
    // std::cout << "extracted serial_id : " << ptrUdevMaker->udevInfo.serial << std::endl;

    return true;
}