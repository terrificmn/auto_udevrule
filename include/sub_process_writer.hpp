#ifndef SUB_PROCESS_WRITER_HPP
#define SUB_PROCESS_WRITER_HPP

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>

class SubProcessWriter {
public:
    SubProcessWriter();
    ~SubProcessWriter();

    bool startProcess(const std::string& sub_prog_path);
    bool writeLine(const std::string& data);
    bool writeContent(const std::string& data);
    int finishProcess();

private:
    int writePipeFd; // file descriptor 
    pid_t m_pid;
    bool is_gui_mode = true;

};

#endif //SUB_PROCESS_WRITER_HPP