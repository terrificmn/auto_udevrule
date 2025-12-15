#include "usb_info_confirmer.hpp"

UsbInfoConfirmer::UsbInfoConfirmer(UdevMaker* udevMaker) : ptrUdevMaker(udevMaker) { }

ResultData UsbInfoConfirmer::findNewDevice(const int& try_count, const Mode& mode) {
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
    if(mode == Mode::LAST_DETECT_MODE) {
        this->executeCmd(v_result_data, cmd_str, ResultType::LAST_RESULT_ONLY);
    } else if(mode == Mode::ALL_DETECT_MODE) {
        this->executeCmd(v_result_data, cmd_str, ResultType::WHOLE_RESULT);
        
        ///TEST
        v_result_data.push_back("[12524.612507] usb 1-9.2.1: pl2303 converter now attached to ttyUSB0");
        v_result_data.push_back("[12612.210855] pl2303 ttyUSB0: pl2303 converter now disconnected from ttyUSB0");
        v_result_data.push_back("[12613.500000] usb 1-9.2.3: pl2303 converter now attached to ttyUSB2");
        v_result_data.push_back("[12616.500000] usb 1-9.2.4: pl2303 converter now attached to ttyUSB3");
        v_result_data.push_back("[12613.572544] usb 1-9.2.1: pl2303 converter now attached to ttyUSB0");
        v_result_data.push_back("[12614.669504] ftdi_sio ttyUSB1: Unable to write latency timer: -32");
        v_result_data.push_back("[12614.670634] usb 1-9.2.2: FTDI USB Serial Device converter now attached to ttyUSB1");
        v_result_data.push_back("[12615.210855] pl2303 ttyUSB0: pl2303 converter now disconnected from ttyUSB0");
        v_result_data.push_back("[12615.612507] usb 1-9.2.1: pl2303 converter now attached to ttyUSB0");
        v_result_data.push_back("[    8.362335] usb 3-2.1: cp210x converter now attached to ttyUSB1");
        v_result_data.push_back("[    8.364675] usb 3-6.1: FTDI USB Serial Device converter now attached to ttyUSB0");
        v_result_data.push_back("[    8.365392] usb 3-2.2: cp210x converter now attached to ttyUSB2");
    }

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

    if(mode == Mode::LAST_DETECT_MODE) {
        resultData.result_str = std::move(v_result_data.at(0));
        this->findUsbNumber(resultData);

    } else {
        resultData.result_v = std::move(v_result_data);
        /// findUsbNumber() 는 하나 str만 처리하므로 더 테스트 필요
    }

    return resultData;
}

void UsbInfoConfirmer::findUsbNumber(ResultData& resultData) {
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
}

void UsbInfoConfirmer::checkValidDevices(ResultData& resultData) {
    if(!this->sh_un_tty_udev_info) {
        this->sh_un_tty_udev_info = std::make_shared<UnTtyUdevInfo>();
    }

    int result_size = resultData.result_v.size();
    for(int i=result_size; i > 0;i--) {
        std::string& result_str = resultData.result_v.at(i -1); // real index
        if(result_str.find("attached") != std::string::npos) {
            std::cout << "FOUND Device: ";

            /// findUsbNumber() 중 ;
            ///TODO: 변경 또는 업데이트 findUsbNumber()
            size_t find_index = result_str.find("ttyUSB");
            if(find_index != std::string::npos) {
                // std::cout << "found index: " << find_index << std::endl;
                std::string temp_usb_num = result_str.substr(find_index+6, 1); // extract the last index 
                // this->m_usb_num_str = resultData.result_str.substr(find_index+6, 1); // extract the last index 
                std::cout << "usb device id: " << temp_usb_num << std::endl;
                int usb_id;
                try {
                    usb_id = std::stoi(temp_usb_num);
                } catch(const std::exception& e) {
                    std::cerr << "error while converting to number: " << e.what() << std::endl;
                    continue;
                }
                
                TtyUdevInfo ttyUdevInfo;
                std::string kernel_id = this->getKernelId(result_str);
                if(!kernel_id.empty()) {
                    std::cout << "Okay. Found the USB device" << std::endl;
                    ttyUdevInfo.kernel = kernel_id;

                } else {
                    std::cout << "Not Found the device for ttyUSB." << std::endl;
                }
                
                auto [it, inserted] = this->sh_un_tty_udev_info->emplace(usb_id, ttyUdevInfo);
                if(!inserted) {
                    std::cout << usb_id << " already found. it will be skipped." << std::endl;
                }
                
                //// the same but it will do double lookup (find() and operator[])
                // auto it = this->sh_un_tty_udev_info->find(usb_id);
                // if(it != this->sh_un_tty_udev_info->end()) {
                //     std::cout << usb_id << " already found. it will be skipped." << std::endl;
                //     continue;
                    
                // } else {
                //     /// need to dereference
                //     (*this->sh_un_tty_udev_info)[usb_id] = ttyUdevInfo;
                // }
            }
        }

    }

    std::cout << "-----------------------------------\n";
    for(auto tty: *this->sh_un_tty_udev_info) {
        std::cout << "usb: " << tty.first << std::endl;
        std::cout << "\t" << "kernel: " << tty.second.kernel << std::endl;
    }
}

bool UsbInfoConfirmer::devicesExist(ResultData& resultData) {
    if(!this->sh_un_tty_udev_info) {
        std::cerr << "shared_ptr sh_un_tty_dev_info not initialized" << std::endl;
        return false;
    }

    int failure_count = 0;
    int total_usbs = this->sh_un_tty_udev_info->size();
    std::cout << "****size: " << this->sh_un_tty_udev_info->size() << std::endl;

    for(auto tty: *this->sh_un_tty_udev_info) {
        // std::cout << "usb: " << tty.first << std::endl;
        std::string usb_number = std::to_string(tty.first);
        std::string cmd = "ls /dev/ttyUSB" + usb_number;
        bool res = this->executeSimpleCmd(cmd);
        if(!res) {
            std::cout << "ttyUSB" << usb_number << " not connected." << std::endl;
            (*this->sh_un_tty_udev_info)[tty.first].is_connected_now = false;
            failure_count++;
            continue; /// still have one left
        } else {
            (*this->sh_un_tty_udev_info)[tty.first].is_connected_now = true;
        }
    }

    std::cout << "-----------------------------------\n";
    for(auto tty: *this->sh_un_tty_udev_info) {
        std::cout << "usb: " << tty.first << std::endl;
        std::cout << "\t" << "kernel: " << tty.second.kernel << std::endl;
        std::cout << "\t" << "is_connected_now: "  << std::boolalpha << tty.second.is_connected_now << std::endl;
    }

    if(failure_count == total_usbs) {
        return false;
    }
    return true;
}


void UsbInfoConfirmer::findUdevInfosWrapper(const bool& is_acm_detected) {
    if(!this->sh_un_tty_udev_info) {
        std::cerr << "shared_ptr not initialized." << std::endl;
        return;
    }

    for(auto& udev : *this->sh_un_tty_udev_info) {
        std::vector<std::string> v_udev_str;
        if(!udev.second.is_connected_now) {
            std::cerr << udev.first << " is not connected right now. aborting to find the udev info" << std::endl;
            continue;
        }

        //// TODO: is_connected_now 가 true 일 경우 아래 테스트 필요!
        v_udev_str = this->findUdevInfo(udev.first); // overload function

        std::string res_vender_id, res_model_id, res_serial_id, res_vender_db_id;
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
            if(!this->getVenderDb(v_udev_str[i]).empty()) {
                res_vender_db_id = v_udev_str[i];
            }
            /// loop until all strings are found.
            if(!res_vender_id.empty() && !res_model_id.empty() && !res_serial_id.empty() && !res_vender_db_id.empty()) {
                // std::cout << "break!" << std::endl;
                break;
            }
        } // second loop

        std::cout << "vendor_id : " << res_vender_id << std::endl;
        std::cout << "model_id : " << res_model_id << std::endl;
        std::cout << "serial_id : " << res_serial_id << std::endl;
        std::cout << "vendor_database : " << res_vender_db_id << std::endl;

        /// step4. save value
        udev.second.vendor = this->getIdsAterRegex(res_vender_id);
        udev.second.product = this->getIdsAterRegex(res_model_id);
        udev.second.serial = this->getIdsAterRegex(res_serial_id);
        udev.second.vendor_db = this->getIdsAterRegex(res_vender_db_id);
        std::cout << "extracted vendor_id : " <<  udev.second.vendor << std::endl;
        std::cout << "extracted model_id : " << udev.second.product  << std::endl;
        std::cout << "extracted serial_id : " << udev.second.serial << std::endl;
        std::cout << "extracted vendor_db_id : " << udev.second.vendor_db << std::endl;
    } // first for loop end

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


std::vector<std::string> UsbInfoConfirmer::findUdevInfo(int usb_id) {
    std::vector<std::string> v_udev_result_data;
    std::string usb_id_str;
    try {
        usb_id_str = std::to_string(usb_id);

    } catch(const std::exception& e) {
        std::cerr << "Exception error while converting " << e.what() << std::endl;
        return {};
    }

    std::string concanated_cmd;
    concanated_cmd = this->m_udev_base_cmd + "USB" + usb_id_str;
    // if(is_acm_detected) {
    //     concanated_cmd = this->m_udev_base_cmd + "ACM" + usb_id;
    // } else {
    //     concanated_cmd = this->m_udev_base_cmd + "USB" + usb_id;
    // }

    this->executeCmd(v_udev_result_data, concanated_cmd, ResultType::WHOLE_RESULT);

    return std::move(v_udev_result_data);
}

/// @brief TEST is needed. Not used currently, but it's ready to use.
/// @param cmd_str 
/// @return 
bool UsbInfoConfirmer::executePopen(const std::string& cmd_str) {
    std::array<char, 128> buffer;
    std::string result;

    std::unique_ptr<FILE, PipeCloser> pipe(popen(cmd_str.c_str(), "r"));
    if(!pipe) {
        std::cerr << "popen() failed!" << std::endl;
    }

    while(fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data(); // append
        //// result = buffer.data(); // (It replaces each time. maybe saving for the last result. )
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
/// Data can be read line by line or as many as buffer's size on process_result_type
/// @param cmd_result_data 
/// @param cmd_str 
/// @param process_result_type 
void UsbInfoConfirmer::executeCmd(std::vector<std::string>& cmd_result_data, const std::string& cmd_str, int process_result_type) {
    // std::cout << "command: " << cmd_str << std::endl;
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
    if(process_result_type == ResultType::LAST_RESULT_ONLY) {
        char buffer[256];
        ssize_t count;
        std::string result;
        
        while((count = read(pipefd[0], buffer, sizeof(buffer) -1)) > 0) {
            buffer[count] = '\0'; // add null-terminate : 한번 읽을 때 바이트 수 만큼 읽어오는데, 마지막 인데스에 null-terminate 를 해줌
            result += buffer;
            // std::cout << "-- read once\n";
            // std::cout << "--result: " << buffer <<  std::endl;
        }

        /// Last result count -1 --> error
        if(count == -1) {
            perror("read");
        }
        // std::cout << "result size: " << result.size() << ", result: \n" << result << std::endl;

        if(!result.empty()) {
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

    } else if(process_result_type == ResultType::WHOLE_RESULT) {
        // std::cout << "WHOLE_RESULT: " << result << std::endl;
        /// 기존 방법은 버퍼 만큼 받아오고 \0이 들어가게 되므로 처리하기 곤란..
        ///  File descriptor 로 변환에서 사용
        FILE* stream = fdopen(pipefd[0], "r");
        if(!stream) {
            close(pipefd[0]);
            return;
        }
        std::vector<std::string> lines;
        char line_buff[1024];

        /// fgets read line by line
        while(fgets(line_buff, sizeof(line_buff), stream) != nullptr) {
            std::string line(line_buff);
            // remove trailing newline if present
            if(!line.empty() && line.back() == '\n') {
                line.pop_back();
            }

            lines.push_back(line);
        }
        fclose(stream);

        cmd_result_data = std::move(lines);
    
    } else if(process_result_type == ResultType::EXECUTE_ONLY) {
        char buffer[256];
        ssize_t count;
        std::string result;
        
        while((count = read(pipefd[0], buffer, sizeof(buffer) -1)) > 0) {
            buffer[count] = '\0'; // add null-terminate : 한번 읽을 때 바이트 수 만큼 읽어오는데, 마지막 인데스에 null-terminate 를 해줌
            result += buffer;
            // std::cout << "-- read once\n";
            // std::cout << "--result: " << buffer <<  std::endl;
        }

        /// Last result count -1 --> error
        if(count == -1) {
            perror("read");
        }
        std::cout << "EXECUTE_ONLY result: " << result << std::endl;
        /// FYI:cmd_result_data is not used in this case.
    }
 
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

std::string UsbInfoConfirmer::getVenderDb(std::string& last_message) {
    // verder_database 넘버 추출
    // VENDOR_FROM_DATABASE
    std::string reg_str = "(ID_VENDOR_FROM_DATABASE=)";

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

void UsbInfoConfirmer::makeCopyUdevInfoByVendor() {
    if(!this->sh_un_tty_udev_info) {
        std::cerr << "shared_ptr sh_un_tty_dev_info not initialized" << std::endl;
        return;
    }

    for(auto& udev : *this->sh_un_tty_udev_info) {
        // udev.first
        if(!udev.second.is_connected_now) {
            std::cout << "usb " << udev.first << " skipped." << std::endl;
            continue;
        }
        if(udev.second.vendor_db.find(LuaConfig::luaParam.lidar2d_main_vendor) != std::string::npos) {
            std::cout << LuaConfig::luaParam.lidar2d_main_vendor << " vendor matched. " << udev.second.vendor_db << std::endl;
            ///FYI: unorder_map은 not allow to have duplicated keys , map 도 마찬가지지만, vector로 추가하는 것은 가능
            this->v_udev_by_vendor[LuaConfig::luaParam.lidar2d_main_vendor].push_back(udev.second);
            this->v_udev_by_vendor[LuaConfig::luaParam.lidar2d_main_vendor].push_back(udev.second); /// For test
            continue;
        } else {
            std::cout << "Not fount vendor." << std::endl;
        }

        if(udev.second.vendor_db.find(LuaConfig::luaParam.lidar2d_bottom_vendor) != std::string::npos) {
            std::cout << LuaConfig::luaParam.lidar2d_bottom_vendor << " vendor matched. " << udev.second.vendor_db << std::endl;
            ///FYI: unorder_map은 not allow to have duplicated keys , map 도 마찬가지지만, vector로 추가하는 것은 가능
            this->v_udev_by_vendor[LuaConfig::luaParam.lidar2d_bottom_vendor].push_back(udev.second);
            this->v_udev_by_vendor[LuaConfig::luaParam.lidar2d_bottom_vendor].push_back(udev.second); /// For test
            continue;
        } else {
            std::cout << "Not fount vendor." << std::endl;
        }

        if(udev.second.vendor_db.find(LuaConfig::luaParam.loadcell_vendor) != std::string::npos) {
            std::cout << LuaConfig::luaParam.loadcell_vendor << " vendor matched. " << udev.second.vendor_db << std::endl;
            ///FYI: unorder_map은 not allow to have duplicated keys , map 도 마찬가지지만, vector로 추가하는 것은 가능
            this->v_udev_by_vendor[LuaConfig::luaParam.loadcell_vendor].push_back(udev.second);
            this->v_udev_by_vendor[LuaConfig::luaParam.loadcell_vendor].push_back(udev.second); /// For test
            continue;
        } else {
            std::cout << "Not fount vendor." << std::endl;
        }

        // if(udev.second.vendor_db.find(LuaConfig::luaParam.) != std::string::npos) {
        //     std::cout << "vendor matched. " << udev.second.vendor_db << std::endl;
        //     ///FYI: unorder_map은 not allow to have duplicated keys , map 도 마찬가지지만, vector로 추가하는 것은 가능
        //     this->v_udev_by_vendor[LuaConfig::luaParam.loadcell_vendor].push_back(udev.second);
        //     this->v_udev_by_vendor[LuaConfig::luaParam.loadcell_vendor].push_back(udev.second); /// For test
        //     continue;
        // } else {
        //     std::cout << "Not fount vendor." << std::endl;
        // }
    }

    // check
    auto& vecs = this->v_udev_by_vendor[LuaConfig::luaParam.lidar2d_main_vendor];
    if(vecs.size() > 0) {
        std::cout << "[" << LuaConfig::luaParam.lidar2d_main_vendor << "] info below" << std::endl;
    }
    for(auto& v : vecs) {
        std::cout << "\tis_conneted: " << std::boolalpha << v.is_connected_now << std::endl;
        std::cout << "\tvnedor_id: " << v.vendor << std::endl;
        std::cout << "------------\n";
    }
    
}

int UsbInfoConfirmer::showResult(std::shared_ptr<TtyUdevInfo> shared_tty_udev_info) {
    if(!shared_tty_udev_info) {
        return 1;
    }
    ///DEBUG
    std::cout << "symlink name " << shared_tty_udev_info->symlink_name << std::endl;
    // std::cout << "rule file name: " << this->ptrUdevMaker->getUdevRuleFilename() << std::endl;

    std::string cmd = "ls -l /dev/" + shared_tty_udev_info->symlink_name;
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
                this->ls_rule_result = show_str;
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
