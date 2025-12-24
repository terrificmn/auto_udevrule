#include "udev_maker.hpp"

/// @brief When policy kit uses for GUI(QT), set true. set false for CLI
/// @param policy_kit_use 
UdevMaker::UdevMaker(bool policy_kit_use) : is_policy_kit_needed(policy_kit_use) {
    if(!this->is_policy_kit_needed) {
        std::cout << "CLI mode" << std::endl;
    } else {
        std::cout << "QT Quick mode" << std::endl;
    }
    this->HELPER_WRITER_FULL_PATH = LuaConfig::config_path + "/bin/" + this->HELPER_WRITER_FILENAME;
    ///DEBUG
    std::cout << "helper_writer path: " << this->HELPER_WRITER_FULL_PATH << std::endl;
}

UdevMaker::~UdevMaker() { }

/// @brief to create a 'ref' dir and a 'list' file if 
/// @return 
bool UdevMaker::initialize() {
    std::fstream fsFile;
    std::string list_path = LuaConfig::config_path + "/ref";

    std::string list_file_path;
    bool res_dir = std::filesystem::exists(list_path);
    if(!res_dir) {
        std::cout << "list_path dir not found. " << list_path << std::endl;
        std::filesystem::create_directories(list_path);
    }
    list_file_path = list_path + "/list";

    bool res_list_path = std::filesystem::exists(list_file_path);
    if(!res_list_path) {
        this->createBasicList(list_file_path);
    }

    std::string helper_path = LuaConfig::config_path + "/bin";
    bool res_helper_path = std::filesystem::exists(helper_path);
    if(!res_helper_path) {
        if(!this->copyHelper(helper_path)) {
            std::cerr << "failed to move " << this->HELPER_WRITER_FILENAME << "! Need to move manually.\n";
        }
    }

    /// read list : preparation for symlink names and script file names.
    fsFile.open(list_file_path, std::ios::in);
    if(!fsFile.is_open()) {
        std::cerr << "can not open: " << list_file_path << std::endl;
        return false;
    }
    this->saveVector(&fsFile);
    std::cout << "Reading a config file for an udev list...";
    fsFile.close();
    std::cout << " Done\n";

    return true;
}

/// @brief it's decided when UdevMaker is initialized.
/// @return 
bool UdevMaker::getIsPolicyKitNeeded() {
    return this->is_policy_kit_needed;
}

/// @brief set symlink_name and udev_filename as well by user input (v_list_index)
/// @param v_list_index 
/// @return 
bool UdevMaker::setSymlink(int v_list_index, std::shared_ptr<TtyUdevInfo> shared_tty_udev_info) {
    if(!shared_tty_udev_info) {
        std::cerr << "shared_tty_udev_info is not initialized." << std::endl;
        return false;
    }
    // std::cout << "set symlink ...\n";

    // 넘버 자체를 +1 해서 받으므로 사이즈 만큼은 받는다. (0일 때 -1 되는 것 방지)
    if(v_list_index > this->v_device_list.size() || v_list_index == 0) {
        std::cerr << "exceed index number!" << std::endl;
        return false;
    }

    // v_list_index 는 처음에 인덱스에 +1 를 해줬으므로 다시 -1 해준다.
    shared_tty_udev_info->symlink_name = this->v_symlink_list.at(v_list_index -1);
    // this->udev_filename = "90-" + this->v_device_list[v_list_index -1] + this->symlink_suffix;
    this->udev_filename = prefix_udevrule_number + "-" + this->v_device_list[v_list_index -1] + this->symlink_suffix;
    /// ex: 90-esp-rules (completed filename)
    
    // std::cout << "symlink_name: " << this->udevInfo.symlink_name << "\n";

    return true;
}


/// @brief set symlink name and symlink filename by user's input instead of using a list file under ref
/// @param user_input 
void UdevMaker::setSymlinkNameByType(const std::string& user_input, std::shared_ptr<TtyUdevInfo> shared_tty_udev_info) {
    if(user_input.empty()) {
        std::cerr << "empty string is not allowed." << std::endl;
        return;
    }
    if(!shared_tty_udev_info) {
        std::cerr << "shared_tty_udev_info is not valid." << std::endl;
        return;
    }

    std::string temp_verified_str = this->veritySymlinkName(user_input);
    std::cout << "verified_temp_str: " << temp_verified_str << std::endl;

    std::string temp_str = this->makeSymlinkFilename(user_input);
    std::cout << "temp_str: " << temp_str << std::endl;

    /// TODO: may need a function that converts the Camel case for user input
    shared_tty_udev_info->symlink_name = "tty" + user_input;
    std::cout << "Now udevInfo symlink_name: " << shared_tty_udev_info->symlink_name << std::endl;

    this->udev_filename = prefix_udevrule_number + "-" + temp_str + this->symlink_suffix;
    /// ex: 90-esp-rules (completed filename)
    std::cout << "udev_filename: " << this->udev_filename << std::endl;
}

std::string UdevMaker::getSymlinkNameFromList(int list_index) {
    // 넘버 자체를 +1 해서 받으므로 사이즈 만큼은 받는다. (0일 때 -1 되는 것 방지)
    if(list_index > this->v_device_list.size() || list_index == 0) {
        std::cerr << "exceed index number!" << std::endl;
        return "";
    }

    // list_index 는 처음에 인덱스에 +1 를 해줬으므로 다시 -1 해준다.
    try {
        std::string rtn = this->v_symlink_list.at(list_index -1);
        std::cout << "symlink name: " << rtn << std::endl;
        return rtn;

    } catch(const std::exception& e) {
        std::cerr << "Exception Error: " << e.what() << std::endl;
        return "";
    }
}

/// @brief make a symlink file name. '-' is going to be inserted between words like hello-world by user's input: HelloWorld or hello_world
/// @param user_input_str 
/// @return 
std::string UdevMaker::makeSymlinkFilename(const std::string& user_input_str) {
    std::string temp_str;
    bool is_first_cap = false;
    for(int i=0; i < user_input_str.size(); ++i) {
        if(user_input_str[i] == '-' || user_input_str[i] == '_') {
            temp_str += '-';
        } else if(user_input_str[i] == std::toupper(user_input_str[i]) ) {
            // std::cout << "Capital letter found.\n";
            ///FYI: only first time to lower the letter
            if(!is_first_cap && i == 0) { 
                temp_str += std::tolower(user_input_str[i]);
            } else {
                temp_str += + '-';
                temp_str += std::tolower(user_input_str[i]);
            }
            is_first_cap = true;
        } else {
            temp_str += user_input_str[i];
        }
    }

    return std::move(temp_str);
}


std::string UdevMaker::veritySymlinkName(const std::string& user_input_str) {
    std::string temp_str;
    bool need_to_check_cap = false;
    for(int i=0; i < user_input_str.size(); ++i) {
        if( (i == 0 || need_to_check_cap == true) && user_input_str[i] != std::toupper(user_input_str[i]) ) {
            temp_str += std::toupper(user_input_str[i]);
            need_to_check_cap = false; // reset
        } else if(user_input_str[i] == '-' || user_input_str[i] == '_') {
            // do nothing
            need_to_check_cap = true;
        } else {
            temp_str += user_input_str[i];
        }
    }

    return temp_str;
}

bool UdevMaker::openFile(std::fstream* fs, std::string filename, Type type_enum) {
    std::string path = this->file_path + "/" + filename;
    
    if(type_enum == Type::READ) {
        fs->open(path, std::ios::in);

    } else if(type_enum == Type::DELETE) {
        /// udev rule filename has a prefix and suffix word like 90-(device name).rules
        path = udev_path + "/" + this->prefix_udevrule_number + "-" + filename + this->symlink_suffix;
        // std::cout << "delete path: " << path << std::endl;
        fs->open(path, std::ios::in);

    } else { //write
        fs->open(path, std::ios::out);
    }

    if(fs->is_open()) {
        return true;
    }

    return false;
}        


void UdevMaker::saveVector(std::fstream* fs) {
    std::string aline;
    while(getline(*fs, aline)) {
        if(aline[0] != '#') {   // # 주석으로 처리 
            // std::cout << aline << std::endl;
            this->v_device_list.push_back(aline);
            /// - 이후는 camel case 방식으로 변경
            std::string temp_str;
            for(int i=0; i < aline.size(); ++i) {
                if(aline[i] == '-') {
                    i++;
                    temp_str += std::toupper(aline[i]);  // 대문자
                } else {
                    temp_str += aline[i];
                }
            }
            // 첫 글자도 대문자
            temp_str[0] = std::toupper(temp_str[0]);
            // prefix 
            temp_str = this->symlink_prefix + temp_str;
            this->v_symlink_list.push_back(temp_str);
            // std::cout << "converted : " << temp_str << std::endl; 
        }

    }
}

void UdevMaker::printList() {
    int size = this->v_device_list.size();
    if(size == 0) {
        return;
    }

    for(int i=0; i< size; ++i) {
        std::cout << i+1 << " : " << this->v_device_list[i] << std::endl;  // 1로부터 보여주기
    }

}

std::vector<std::string> UdevMaker::getDeviceList() {
    return this->v_device_list;
}

/// @brief make content(udev) using fstream
/// @param fs 
void UdevMaker::makeScript(std::fstream* fs) {
    std::string str;

    // this->makeContent(str);
    
    *fs << str;
}

bool UdevMaker::getSerialWarn(std::shared_ptr<TtyUdevInfo> shared_tty_udev_info) {
    // std::cout << "current use count :" << shared_tty_udev_info.use_count() << std::endl;
    if(!shared_tty_udev_info) {
        return false;
    }
    if(LuaConfig::use_serial == true && shared_tty_udev_info->serial.empty() && !shared_tty_udev_info->kernel.empty()) {
        return true;
    }
    return false;
}

void UdevMaker::makeContent(std::string& udev_str, std::shared_ptr<TtyUdevInfo> shared_tty_udev_info) {
    if(!shared_tty_udev_info) {
        std::cerr << "shared_tty_udev_info is not valid." << std::endl;
        return;
    }

    /// static LuaConfig's variable already set.
    if(LuaConfig::use_kernel) {
        udev_str = "SUBSYSTEM==\"tty\", KERNELS==\"" + shared_tty_udev_info->kernel + "\", ATTRS{idVendor}==\"" + shared_tty_udev_info->vendor_id + "\", ";
    } else {
        udev_str = "SUBSYSTEM==\"tty\", ATTRS{idVendor}==\"" + shared_tty_udev_info->vendor_id + "\", ";
    }

    if(LuaConfig::use_serial) {
        udev_str.append("ATTRS{serial}==\"" +  shared_tty_udev_info->serial  + "\", ");
    }
    udev_str.append("ATTRS{idProduct}==\"" + shared_tty_udev_info->product + "\", MODE:=\"0666\", GROUP:=\"dialout\", SYMLINK+=\"" + shared_tty_udev_info->symlink_name + "\"");
    
    ///DEBUG
    std::cout << "\nscript's content:\n";
    std::cout << udev_str << std::endl;
    // for(std::vector<double> current_pose : vec_waypoints) {
    //     for(int i=0; i < current_pose.size(); i++) {
    //         wpFile << std::to_string(current_pose.at(i)) << ",";
    //     }
    // }
}


/// @brief copy a script file to /etc/...
/// @return 
bool UdevMaker::copyUdev() {
    /// test: ref 디렉토리에 그대로 복사함 
    // std::string cmd = "cp " + this->file_path + "/temp_script.sh " + this->file_path + "/" + this->udev_filename;
    /// to udev_path : /etc/ 이하 디렉토리로 복사하게 됨.
    std::string cmd = "sudo cp " + this->file_path + "/temp_script.sh " + this->udev_path + "/" + this->udev_filename;
    
    // std::cout << "cmd: " << cmd << std::endl;
    bool res = std::system(cmd.c_str());
    if(res != 0) {
        return false;
    }

    if(!this->executeUdevadmControl()) {
        std::cerr << "falied to udevadm control" << std::endl;
        return false;
    }
    
    //final
    std::cout << "udevadm reload rules... done.\n";
    return true;
}

bool UdevMaker::executeUdevadmControl() {
    std::string cmd;
    if(!this->is_policy_kit_needed) {
        cmd = "sudo udevadm control --reload-rules; sudo udevadm trigger";
    } else {
        cmd = "pkexec bash -c \"udevadm control --reload-rules; udevadm trigger\"";
        ///FYI: Not work like the below. command should be inserted together in a bash command.
        // cmd = "pkexec bash -c \"udevadm control --reload-rules\";pkexec bash -c \"udevadm trigger\""; 
    }
    bool res = std::system(cmd.c_str());
    if(res != 0) {
        return false;
    }
    return true;
}

int UdevMaker::createUdevRuleFile(std::shared_ptr<TtyUdevInfo> shared_tty_udev_info) {
    if(!shared_tty_udev_info) {

        return -1;
    }
    std::cout << "Now.. writing begins..\n";
    // std::string helper_writer_path = this->HELPER_WRITER_FULL_PATH;
    std::string cmd = "sudo " + this->HELPER_WRITER_FULL_PATH;
    std::string target_udev_path = this->udev_path + "/" + this->getUdevRuleFilename();
    ////DEBUG
    std::cout << "Atempting to write to " << target_udev_path << std::endl;
    /// alreay sudo executed 
    // std::cout << "You will likely be prompted for your sudo password." << std::endl;

    /// process 
    /// 1. open file
    /// 2. send the path
    /// 3. send "NEW" for making a file.
    /// 4. send the actual content

    /// 1.
    FILE* pipeFile = popen(cmd.c_str(), "w");
    if(!pipeFile) {
        perror("Helper Writer failed");
        return -2;
    }

    // 2.
    // send udev path : //FYI: '\n' 을 붙여야지 Heler writer 에서 입력이 완료
    if(fprintf(pipeFile, "%s\n", target_udev_path.c_str()) < 0) {
        perror("Failed to write the target path");
        return -3;
    }
    fflush(pipeFile);

    /// 3.
    /// "NEW" keyword 로 보내야지 create mode 작동하게 만듬
    if(fprintf(pipeFile, "%s\n", "NEW") < 0) {
        perror("Failed to write the mode");
        return -4;
    }
    fflush(pipeFile);

    /// 4.
    // send the actual content
    std::string udev_content;
    this->makeContent(udev_content, shared_tty_udev_info);

    if(fprintf(pipeFile, "%s", udev_content.c_str()) < 0) {
        perror("Failed to write content to Helper Writer's stdin");
        return -5;
    }
    fflush(pipeFile);

    int result_status = pclose(pipeFile);
    if(result_status == -1) {
        perror("pclose failed");
        return -1;
    } else {
        if (WIFEXITED(result_status)) {
            int exit_code = WEXITSTATUS(result_status);
            if (exit_code == 0) {
                ///DEBUG
                std::cout << "Helper Writer process executed successfully. File should be written." << std::endl;
            } else {
                std::cerr << "Helper Writer process exited with error code: " << exit_code << std::endl;
                std::cerr << "Check Helper Writer's stderr output (if any was printed to the same terminal)." << std::endl;
                ///FYI: Non-zero (1-255)
                return exit_code;
            }
        } else {
            std::cerr << "Helper Writer process did not terminate normally (e.g., killed by a signal)." << std::endl;
            return -6;
        }
    }

    if(!this->executeUdevadmControl()) {
        std::cerr << "falied to udevadm control" << std::endl;
        return -7;
    }

    return 0; // finally!
}

int UdevMaker::createUdevRuleFileWithFork(std::shared_ptr<TtyUdevInfo> shared_tty_udev_info) {
    SubProcessWriter subProcessWriter;

    /// process 
    /// 1. open file
    /// 2. send the path
    /// 3. send "NEW" for making a file.
    /// 4. send the actual content
    std::string target_udev_path = this->udev_path + "/" + this->getUdevRuleFilename();
    std::cout << "Atempting to write to " << target_udev_path << std::endl;

    bool res = subProcessWriter.startProcess(this->HELPER_WRITER_FULL_PATH);
    if(!res) {
        return 1;
    }

     // 2. send udev path : //FYI: '\n' 을 붙여야지 Heler writer 에서 입력이 완료
    if(!subProcessWriter.writeLine(target_udev_path)) {
        std::cerr << "Failed to write the target path" << std::endl;
        return -3;
    }
    ///FYI: no need to use fflush rather than fprintf with popen

    /// 3. "NEW" keyword 로 보내야 create mode 가 작동함
    if(!subProcessWriter.writeLine("NEW")) {
        std::cerr << "Failed to write the mode" << std::endl;
        return -4;
    }

    /// 4. send the actual content
    std::string udev_content;
    this->makeContent(udev_content, shared_tty_udev_info);

    if(!subProcessWriter.writeContent(udev_content)) {
        std::cerr << "Failed to write content to Helper Writer's stdin" << std::endl;
        return -5;
    }

    return subProcessWriter.finishProcess();
}


int UdevMaker::deleteUdevRuleFileWithFork(std::shared_ptr<TtyUdevInfo> shared_tty_udev_info) {
    SubProcessWriter subProcessWriter(false);  /// false for removal 

    /// process 
    /// 1. open a file
    /// 2. send the path
    /// 3. send "DEL" for removal

    std::string target_del_path = this->udev_path + "/" + this->getUdevRuleFilename();
    std::cout << "Atempting to write to " << target_del_path << " to remove a file." << std::endl;

    bool res = subProcessWriter.startProcess(this->HELPER_WRITER_FULL_PATH);
    if(!res) {
        return 1;
    }

     // 2. send udev path : //FYI: '\n' 을 붙여야지 Heler writer 에서 입력이 완료
    if(!subProcessWriter.writeLine(target_del_path)) {
        std::cerr << "Failed to write the target path" << std::endl;
        return -3;
    }
    ///FYI: no need to use fflush rather than fprintf with popen

    /// 3. "DEL" keyword 로 보내야 delete mode 가 작동
    if(!subProcessWriter.writeLine("DEL")) {
        std::cerr << "Failed to delete" << std::endl;
        return -4;
    }

    return subProcessWriter.finishProcess();
}

int UdevMaker::deleteUdevruleFile() {
    std::cout << "Now.. delete begins..\n";
    ///FYI: use the helper_writer with sudo for the security 
    std::string cmd = "sudo " + this->HELPER_WRITER_FULL_PATH;
    std::string target_del_path = this->udev_path + "/" + this->getUdevRuleFilename();
    std::cout << "Atempting to write to " << target_del_path << std::endl;

    /// already sudo executed 
    // std::cout << "You will likely be prompted for your sudo password." << std::endl;

    /// process 
    /// 1. open a file
    /// 2. send the path
    /// 3. send "DEL" for removal

    /// 1.
    FILE* pipeFile = popen(cmd.c_str(), "w");
    if(!pipeFile) {
        perror("Helper Writer failed");
        return -2;
    }
    /// 2.
    // send udev path : //FYI: '\n' 을 붙여야지 Heler writer 에서 입력이 완료
    if(fprintf(pipeFile, "%s\n", target_del_path.c_str()) < 0) {
        perror("Failed to write the target path");
        return -3;
    }
    fflush(pipeFile); // imporant: fflush make the target_del_path send through pipeFile
    /// 3.
    /// "DEL" keyword 로 create mode 작동하게 만듬
    if(fprintf(pipeFile, "%s\n", "DEL") < 0) {
        perror("Failed to delete mode");
        return -4;
    }
    fflush(pipeFile);

    int result_status = pclose(pipeFile);
    if(result_status == -1) {
        perror("pclose failed");
        return -1;
    } else {
        if (WIFEXITED(result_status)) {
            int exit_code = WEXITSTATUS(result_status);
            if (exit_code == 0) {
                std::cout << "Helper Writer process executed successfully. File should be removed." << std::endl;
            } else {
                std::cerr << "Helper Writer process exited with error code: " << exit_code << std::endl;
                std::cerr << "Check Helper Writer's stderr output (if any was printed to the same terminal)." << std::endl;
                ///FYI: Non-zero (1-255)
                return exit_code;
            }
        } else {
            std::cerr << "Helper Writer process did not terminate normally (e.g., killed by a signal)." << std::endl;
            return -6;
        }
    }

    if(!this->executeUdevadmControl()) {
        std::cerr << "falied to udevadm control" << std::endl;
        return -7;
    }

    return 0; // finally!
}


bool UdevMaker::getUdevFilename(std::string* return_filename, int list_index) {
    if(list_index == 0) { 
        return false;
    }
    
    // user 편의상 1부터 입력을 받으므로, 다시 -1 시켜준다.
    list_index--;
    std::cout << "device name: " << this->v_device_list[list_index] << std::endl;
    *return_filename = v_device_list[list_index];

    return true;
}


/// @brief get a complete filename which is used in the /etc/udev/...  
/// setSymlink() should be called before this function. If not, empty str will return
/// @return 
std::string& UdevMaker::getUdevRuleFilename() {
    return this->udev_filename;
}

int UdevMaker::getVSize() {
    return this->v_device_list.size();
}

void UdevMaker::createBasicList(std::string list_full_path) {
    std::string list_path;
    if(list_full_path.empty()) {
        // for old version
        const std::filesystem::path current_dir = std::filesystem::current_path();
        const std::filesystem::path ref_path = current_dir / "ref";
        if(std::filesystem::is_directory(ref_path)) {
            std::cerr << "It seems there is the 'ref' directory already but 'list_file' does not exist...\n";
        } else {
            std::string mkdir_path = "mkdir -p " + ref_path.string();
            // std::cout << "mkdir path : " << mkdir_path << std::endl; 
            std::system(mkdir_path.c_str());
            std::cout << "ref directory created.\n";
        }

        std::cout << "Now.. the initial set-up begins..\n";
        list_path = "ref/list_file";

    } else {
        list_path = std::move(list_full_path);
    }
    std::fstream fs;

    fs.open(list_path, std::ios::out);
    if(fs.is_open()) {
        fs << "## a list of devices\n";
        fs << "## It will be used for the file name and symlink name.\n";
        fs << "## If any device newly needs, then add its name the below.\n";
        fs << "faduino-upload\n";
        fs << "faduino-com\n";
        fs << "amrbd\n";
        fs << "amrbd-debug\n";
        fs << "front-lidar1\n";
        fs << "rear-lidar\n";
        fs << "front-aux-lidar\n";
        fs << "rear-aux-lidar\n";
        fs << "scale1\n";
        fs << "scale2\n";
        fs << "scale3\n";
        fs << "tfluna\n";
    }
    fs.close();
    std::cout << "A list_file has been created.\n";
    std::cout << "Each device name in the list_file will be used for the symlink name and file name.\n";
    std::cout << "So you can modify or add some devices in the list_file if you need.\n";
}

bool UdevMaker::copyHelper(const std::string& helper_path) {
    if(std::filesystem::create_directories(helper_path)) {
        std::cout << helper_path << " created successfully." << std::endl;
    }

    const std::filesystem::path current_dir = std::filesystem::current_path();
    const std::filesystem::path current_helper_writer_path = current_dir / this->HELPER_WRITER_FILENAME;

    bool res_path = std::filesystem::exists(current_helper_writer_path);
    if(!res_path) {
        std::cout << this->HELPER_WRITER_FILENAME << " Not Found." << std::endl;
        return false;
    } else {
        
        std::cout << this->HELPER_WRITER_FILENAME << " Found." << std::endl;
        // std::filesystem::path dest_dir = std::filesystem::path(LuaConfig::config_path) / ".local" / "share" / "auto_udevrule" / "bin";
        try {
            std::string helper_full_path = helper_path + "/" + this->HELPER_WRITER_FILENAME;
            if(std::filesystem::copy_file(current_helper_writer_path, std::filesystem::path(helper_full_path), std::filesystem::copy_options::overwrite_existing) ) {
                std::cout << this->HELPER_WRITER_FILENAME << " Copy to " << helper_full_path << std::endl;
            }

        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error copying file: " << e.what() << std::endl;
        }
    }

    return true;
}

void UdevMaker::createConfigLua() {
    std::fstream fs;
    const std::filesystem::path current_dir = std::filesystem::current_path();
    const std::filesystem::path config_ref_path = current_dir / "ref";
    if(std::filesystem::is_directory(config_ref_path)) {
        std::cerr << "It seems there is the 'ref' directory already but 'config.lua' does not exist...\n";
    } else {
        std::string mkdir_path = "mkdir -p " + config_ref_path.string();
        // std::cout << "mkdir path : " << mkdir_path << std::endl; 
        std::system(mkdir_path.c_str());
        std::cout << "config directory created.\n";
    }

    std::cout << "Now.. the initial set-up for a lua file, begins..\n";

    fs.open("ref/config.lua", std::ios::out);
    if(fs.is_open()) {
        fs << "-- 단순하게 순서대로 파라미터로 사용, 주석 참고(key)\n";
        fs << "configParams(\n";
        fs << "    false, --use_kernel\n";
        fs << "    true, --use_serial\n";
        fs << "    10.0 --timeout_sec : for dectection\n";
        fs << ")\n";
        fs << "\n-- TODO: dictionary 같은 키:value로 구성 하기\n";
    }
    fs.close();
    std::cout << "A config.lua has been created.\n";
}


/// @brief get user's input (kernel, vendor & product) step by step.
/// save data to un_dev_info (unordered_map<string>)
/// @return 
bool UdevMaker::inputDevInfo(std::shared_ptr<TtyUdevInfo> shared_tty_udev_info) {
    // instead of detectUsb()
    std::cout << "Please insert the data..\n";
    std::cout << "kernel (eg. 1-1): ";
    std::cin >> shared_tty_udev_info->kernel;
    
    std::cout << "vendor id (eg. 10c4): ";
    std::cin >> shared_tty_udev_info->vendor_id;
    
    std::cout << "product id (eg. ea60): ";
    std::cin >> shared_tty_udev_info->product;

    std::cout << "input kernel : " << shared_tty_udev_info->kernel << std::endl;
    std::cout << "input vendor_id : " << shared_tty_udev_info->vendor_id << std::endl;
    std::cout << "input product : " << shared_tty_udev_info->product << std::endl;
    
    std::cout << "Are these correct? (y / n) ?";
    std::string fin_input;
    std::cin >> fin_input;

    if(fin_input == "y" || fin_input == "Y") {
        return true;
    }

    shared_tty_udev_info.reset();
    return false;
}
