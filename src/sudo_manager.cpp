#include "sudo_manager.hpp"

SudoManager::SudoManager() {
    this->has_sudo_privilege = this->checkIfSudoPrivilege();
}

bool SudoManager::checkIfSudoPrivilege() {
    int res = std::system("sudo -n true 2>/dev/null");
    return (res == 0);
}

bool SudoManager::hasSudoPrivilege()  {
    return this->has_sudo_privilege;
}
