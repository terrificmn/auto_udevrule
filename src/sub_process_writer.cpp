#include "sub_process_writer.hpp"

SubProcessWriter::SubProcessWriter(bool write_mode) : writePipeFd(-1), m_pid(-1), is_write_mode(write_mode) {
    ///currently only for GUI mode
}
SubProcessWriter::~SubProcessWriter() {
    this->finishProcess();
}

bool SubProcessWriter::startProcess(const std::string& sub_prog_path) {
    if(!this->is_gui_mode) {
        std::cout << "Only GUI mode is available, now" << std::endl;
        return false;
    }

    ////IMPORTANT: child -> read,  parent -> write
    int pipefd[2];

    if(pipe(pipefd) == -1) {
        std::cerr << "pipe error: -1" << std::endl;
        return false;
    }

    this->m_pid = fork();

    if(this->m_pid == -1) {
        close(pipefd[0]);
        close(pipefd[1]);
        std::cerr << "pipe error after fork: -1" << std::endl;
        return false;
    }

    /// for child process
    if(this->m_pid == 0) {
        close(pipefd[1]); // child does not need to write, so close

        // redirect stdin to read from pipe
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);

        const char* program_exec = "pkexec"; /// add "sudo" if needed later
        
        // prepare arguments
        const char* args[] = {
            program_exec,
            sub_prog_path.c_str(),
            nullptr
        };

        // execute without shell wrapper
        execvp(program_exec, const_cast<char* const*>(args));

        // if execvp fails
        _exit(127);
    }

    /// parent process
    close(pipefd[0]); // parent done not read
    /// IMPORTANT: keep write end for sending data
    this->writePipeFd = pipefd[1];

    return true;
}

bool SubProcessWriter::writeLine(const std::string& data) {
    if(this->writePipeFd == -1) {
        std::cerr << "writePipeFd is not valid." << std::endl;
        return false;
    }

    ///FYI: '\n' 을 붙여야지 Heler writer 에서 입력이 완료
    std::string line = data + '\n';
    ssize_t written = write(this->writePipeFd, line.c_str(), line.length());

    if(written == -1 || written != static_cast<ssize_t>(line.length()) ) {
        std::cerr << "written wrong." << std::endl;
        return false;
    }

    return true;
}

bool SubProcessWriter::writeContent(const std::string& data) {
    if(this->writePipeFd == -1) {
        std::cerr << "writePipeFd is not valid." << std::endl;
        return false;
    }

    ssize_t written = write(this->writePipeFd, data.c_str(), data.length());

    ///unlike the fprintf (buffer), flush() is not necessary
    if(written == -1 || written != static_cast<ssize_t>(data.length()) ) {
        std::cerr << "written wrong." << std::endl;
        return false;
    }

    return true;
}

int SubProcessWriter::finishProcess() {
    ///FYI: startProcess() 에서 writePipeFd 에 pipefd[1] 
    if(this->writePipeFd != -1) {
        close(this->writePipeFd);
        this->writePipeFd = -1; ///reset
    }

    /// If m_pid(child) is already reset, then return
    if(this->m_pid == -1) {
        return -1;
    }

    /// wait for child
    int status;
    waitpid(this->m_pid, &status, 0);

    std::string verb;
    /// defaule
    if(this->is_write_mode) {
        verb = "written.";
    } else {
        verb = "removed.";
    }
    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        if (exit_code == 0) {
            ///DEBUG
            std::cout << "Helper Writer process executed successfully. The requested file has been " << verb << std::endl;
        } else {
            std::cerr << "Helper Writer process exited with error code: " << exit_code << std::endl;
            std::cerr << "Check Helper Writer's stderr output (if any was printed to the same terminal)." << std::endl;
            ///FYI: Non-zero (1-255)
        }
        return exit_code;
    } else {
        std::cerr << "Helper Writer process did not terminate normally (e.g., killed by a signal)." << std::endl;
        return -6;
    }
}