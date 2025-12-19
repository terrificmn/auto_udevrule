#ifndef USB_INFO_CONFIRMER_HPP
#define USB_INFO_CONFIRMER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <unistd.h>
#include <sys/wait.h>
#include <optional>
#include "device_enum.hpp"
#include "udev_maker.hpp"
#include "time_checker.hpp"

#include <regex>
#include <algorithm>

/// @brief Custom Deleter struct: called by std::unique_ptr<FILE, PipeCloser>
struct PipeCloser {
    void operator()(FILE* fp) const {
        if(fp) {
            /// FILE 있으면 close
            pclose(fp);
        }
    }
};

class UsbInfoConfirmer {
public:
    UsbInfoConfirmer(UdevMaker* udevMaker);
    ResultData findNewDevice(const int& try_count, const Mode& mode);
    void findUsbNumber(ResultData& resultData);
    void checkValidDevices(int try_count, ResultData& resultData);
    bool devicesExist(int try_count, ResultData& resultData);
    void findUdevInfosWrapper(const bool& is_acm_detected);
    std::vector<std::string> findUdevInfo(const bool& is_acm_detected);
    std::vector<std::string> findUdevInfo(const std::string& tty_device);
    std::string getUsbId();

    void executeCmd(std::vector<std::string>& cmd_result_data, const std::string& cmd_str, int process_result_type);
    bool executePopen(const std::string& cmd_str);
    bool executeSimpleCmd(const std::string& cmd_str);

    std::string regexWrapper(std::string& last_message, std::string reg_str);
    std::string getDetectedTime(std::string& last_message);
    std::string getKernelIdForAcm(std::string& last_message);
    std::string getKernelId(std::string& last_message);
    std::string getVenderId(std::string& last_message);
    std::string getModelId(std::string& last_message);
    std::string getSerialId(std::string& last_message);
    std::string getValueFromProductKey(std::string& last_message, const RegUdevType& reg_udev_type);
    void removeCharacter(std::string& str, char remove_char);

    std::string getIdsAterRegex(std::string& last_message);
    bool checkNumber(std::string& input_msg);
    
    void makeCopyUdevInfoByVendor();
    void addDummy();
    std::optional<std::vector<TtyUdevInfo>> getTtyUdevInfoVec(const std::string& product_category_name);
    void updateMapCheckList(const std::string product_category_name, int index);
    void updateStatusMapCheckList(const std::string product_category_name, MapStatus map_status);
    MapStatus getStatusFromMapChecklist(const std::string product_category_name);
    int getSymlinkIndexFromMapChecklist(const std::string product_category_name, int idx);

    int showResult(std::shared_ptr<TtyUdevInfo> shared_tty_udev_info);
    std::string getLsResult();

protected:
    using UnTtyUdevInfo = std::unordered_map<std::string, TtyUdevInfo>;
    std::shared_ptr<UnTtyUdevInfo> sh_un_tty_udev_info;
    std::map<std::string, std::vector<TtyUdevInfo>> v_udev_by_pc;

    std::map<std::string, MapCheckList> map_check_list;

private:
    UdevMaker* ptrUdevMaker = nullptr;

    std::string m_dmesg_base_cmd = "dmesg | grep tty";
    std::string m_udev_base_cmd = "udevadm info /dev/tty";
    std::string m_usb_num_str = "-1";
    std::string ls_rule_result;

    bool is_map_created = false;
};

#endif //USB_INFO_CONFIRMER_HPP