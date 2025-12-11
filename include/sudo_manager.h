#ifndef SUDO_MANAGER_H
#define SUDO_MANAGER_H

#include <iostream>

class SudoManager {
public:
    SudoManager();
    bool hasSudoPrivilege();

private:
    bool m_is_password_set = false;
    bool has_sudo_privilege = false;

private:
    bool checkIfSudoPrivilege();


};

#endif // SUDO_MANAGER_H