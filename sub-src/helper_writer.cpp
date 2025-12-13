#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

const char* ALLOWED_PATH = "/etc/udev/rules.d/";
const char* DELETE_MODE = "DEL";
const char* CREATE_MODE = "NEW";

bool isPathAllowed(const std::string& path) {
    std::filesystem::path p(path);
    try {
        // weakly_canonical; resloves symlinks and normalizes path; '..', 맨 뒤의 '/' 등을 제거
        std::filesystem::path canonical_path = std::filesystem::weakly_canonical(p);
        // std::cout << "canonical_path: " << canonical_path << std::endl;
        if(!canonical_path.is_absolute()) {
            std::cerr << "[Helper Writer] Path must be absoulte. Recieved: " << path << std::endl;
            return false;
        }

        // CRITICAL security check
        std::string canonical_path_str = canonical_path.string();
        /// FYI: result should 0. because reverse find should return 0 if ALLOWED_PATH is found from back.
        /// if ALLOWED_PATH is not found at all, it returns npos. But 0 should be the first index. so It's useless if it's not 0.
        /// 문자열 자체가 다 맞아야 함.
        if(canonical_path_str.rfind(ALLOWED_PATH, 0) != 0) {
            std::cerr << "[Helper Writer] Path error: " << canonical_path_str << " Not allowed." << std::endl;
            return false;
        }
        // It works, though
        // int res = canonical_path_str.find(ALLOWED_PATH, 0);
        // if(res == std::string::npos) {
        //     std::cerr << "[Helper Writer] Not found " << ALLOWED_PATH << "in a " << canonical_path_str << " Not allowed." << std::endl;
        //     return false;
        // }
        // std::cerr << "[Helper Writer] Path error: " << canonical_path_str << " Not allowed." << std::endl;

        // absoultely not allowed theh base path itselt!
        if(canonical_path_str == ALLOWED_PATH) {
            std::cerr << "[Helper Writer] Can not delete the base dir: Not allowed." << std::endl;
            return false;
        }

    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "[Helper Writer] Filesystem error." << std::endl;
        return false;
    }

    return true;
}

bool deleteFile(const std::string& target_path) {
    bool result = isPathAllowed(target_path);
    if(!result) {
        return false;
    }

    try {
        if(!std::filesystem::exists(target_path)) {
            std::cerr << "[Helper Writer] file does not exist.." << std::endl;
            return false;
        }

        if(!std::filesystem::is_regular_file(target_path)) {
            std::cerr << "[Helper Writer] only regular file can be deleted." << std::endl;
            return false;
        }

        if(std::filesystem::remove(target_path)) {
            std::cout << "[Helper Writer] " << target_path << " has been removed." << std::endl;
        }

    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "[Helper Writer] error during removal: " << e.what() << std::endl;
        return false;

    } catch (const std::exception& e) {
        std::cerr << "[Helper Writer] exception error: " << e.what() << std::endl;
        return false;
    }

    return true;
}


bool createFile(const std::string& target_path) {
    /// TODO:추가하가
    // **CRITICAL SECURITY VALIDATION (Simplified for this example):**
    bool result = isPathAllowed(target_path);
    if(!result) {
        return false;
    }

    std::string udev_content;
    std::string line;
    while(std::getline(std::cin, line)) {
        udev_content += line + "\n";
    }

    /// write the received conent to the target path
    std::ofstream outFile(target_path, std::ios::out);
    
    if(!outFile.is_open()){
        std::cerr << "[Helper Writer] failed to open target path for writing." << std::endl;
        perror("System error");
        return false;
    }

    outFile << udev_content;
    if(outFile.fail()) {
        std::cerr << "[Helper Writer] failed to write contents to file." << std::endl;
        outFile.close();
        return false;
    }

    outFile.close();

    return true;
}



int main() {
    std::string received_target_path;

    if(!std::getline(std::cin, received_target_path)) {
        std::cerr << "[Helper Writer] failed to read target path from stdin." << std::endl;
        return 1;
    }

    if(received_target_path.empty()) {
        std::cerr << "[Helper Writer] received empty path." << std::endl;
        return 2;
    }

    std::string type_str;
    if(!std::getline(std::cin, type_str)) {
        std::cerr << "[Helper Writer] failed to read the type_str from stdin." << std::endl;
        return 3;
    }

    std::string mode;
    // std::cout << "[Helper Writer] type: " << type_str << std::endl;
    if(type_str == DELETE_MODE) {
        bool res = deleteFile(received_target_path);
        if(!res) return 4;

    } else if(type_str == CREATE_MODE) {
        bool res = createFile(received_target_path);
        if(!res) return 4;

    } else {
        std::cerr << "[Helper Writer] not allowed another mode from stdin." << std::endl;
        return 4;
    }

    std::cout << "[Helper Writer] Done :)" << std::endl;
    return 0;
}