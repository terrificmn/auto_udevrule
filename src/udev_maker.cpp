#include "udev_maker.h"

UdevMaker::UdevMaker() { }

UdevMaker::~UdevMaker() { }


bool UdevMaker::setSymlink(int v_list_index) {
    std::cout << "create... script.. test...\n";

    // 넘버 자체를 +1 해서 받으므로 사이즈 만큼은 받는다. (0일 때 -1 되는 것 방지)
    if(v_list_index > this->v_device_list.size() || v_list_index == 0) {
        std::cerr << "exceed index number!" << std::endl;
        return false;
    }

    // v_list_index 는 처음에 인덱스에 +1 를 해줬으므로 다시 -1 해준다.
    this->m_symlink_name = this->v_symlink_list.at(v_list_index -1);
    this->udev_filename = "90-" + this->v_device_list[v_list_index -1] + this->symlink_suffix;

    std::cout << this->m_symlink_name << "\n";

    return true;
}


bool UdevMaker:: inputSymlink() {
    std::cout << "choose symlink name" << std::endl;
    std::cout << "1. motor \n2. lidar \n3. faduino-upload" << std::endl;
    std::cin >> this->m_symlink_name;

    std::cout <<  "symlink_name: " << this->m_symlink_name << std::endl;
    return true;
}


bool UdevMaker::openFile2() {
    std::fstream fsFile;
    fsFile.open(this->file_path, std::ios::out);  // ios::out write
    if (fsFile.is_open()) {
        std::string str = "SUBSYSTEM==\"tty\", KERNELS==\"" + this->m_kennel + "\", ATTRS{idVendor}==\"" + this->m_vendor;
        str += "\", ATTRS{idProduct}==\"" + this->m_product + "\", MODE:=\"0666\", GROUP:=\"dialout\", SYMLINK+=\"" + this->m_symlink_name + "\"";
        std::cout << str << std::endl;
        // for(std::vector<double> current_pose : vec_waypoints) {
        //     for(int i=0; i < current_pose.size(); i++) {
        //         wpFile << std::to_string(current_pose.at(i)) << ",";
        //     }
        // }
        fsFile << str;
        
    } else {
        std::cout << "failed to save file\n";
        return false;
    }

    fsFile.close();
    std::cout << file_path << " file is saved" << std::endl;

    return true;
}




bool UdevMaker::openFile(std::fstream* fs, std::string filename, Type type_enum) {
    std::string path = this->file_path + "/" + filename;
    
    if(type_enum == Type::READ) {
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


void UdevMaker::makeScript(std::fstream* fs) {

    std::string str = "SUBSYSTEM==\"tty\", KERNELS==\"" + this->m_kennel + "\", ATTRS{idVendor}==\"" + this->m_vendor + "\", ";
    str.append("ATTRS{idProduct}==\"" + this->m_product + "\", MODE:=\"0666\", GROUP:=\"dialout\", SYMLINK+=\"" + this->m_symlink_name + "\"");
    
    std::cout << str << std::endl;
    // for(std::vector<double> current_pose : vec_waypoints) {
    //     for(int i=0; i < current_pose.size(); i++) {
    //         wpFile << std::to_string(current_pose.at(i)) << ",";
    //     }
    // }
    *fs << str;
}


bool UdevMaker::copyUdev() {
    // test 이후 udev_path 로 바꿔주기 - target부분
    /// test
    std::string cmd = "cp " + this->file_path + "/temp_script.sh " + this->file_path + "/" + this->udev_filename;
    /// to udev_path
    // std::string cmd = "cp " + this->file_path + "/temp_script.sh " + this->udev_path + "/" + this->udev_filename;
    
    std::cout << "cmd: " << cmd << std::endl;
    bool res = std::system(cmd.c_str());
    if(res != 0) {
        return false;
    }

    cmd = "sudo udevadm control --reload-rules; sudo udevadm trigger";
    res = std::system(cmd.c_str());
    if(res != 0) {
        return false;
    }

    //final
    return true;
}


int UdevMaker::getVSize() {
    return this->v_device_list.size();
}