#include "Logger.h"

namespace MyLogger {
    void LogHRESULT(HRESULT hr, const char* message, const char* file, int line) {
        std::ofstream logFile("logfile.txt", std::ios::app);
        if (logFile.is_open()) {
            logFile << "File: " << file << ", Line: " << line << ", Message: " << message << ", HRESULT: 0x" << std::hex << hr << std::dec << std::endl;
            logFile.close();
        }
        else {
            std::cerr << "Error opening log file." << std::endl;
        }
    }
}
