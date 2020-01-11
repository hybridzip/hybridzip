#ifndef HYBRIDZIP_LOGGER_H
#define HYBRIDZIP_LOGGER_H

#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <hzip/utils/boost_utils.h>

namespace hzlog {
    enum LOG_TYPE {
        INFO = 0,
        WARNING = 1,
        CRITICAL = 2
    };
}

using namespace hzlog;

class Logger {
private:
    char *ofile;
    std::ofstream ofs;
    FILE *stream;
    bool log_time;
    std::map<int, int> errStatMap;

    void write_err(std::string data) {
        fprintf(stream, "%s", data.c_str());
    }

public:
    Logger(char *_ofile = nullptr, bool _log_time = false) {
        stream = stderr;
        ofile = _ofile;
        log_time = _log_time;
        if (ofile != nullptr) {
            ofs.open(ofile, std::ios_base::app);
        }
    }

    void setErrStatus(int error_code) {
        errStatMap[getpid()] = error_code;
    }

    void setLogStream(FILE* _stream) {
        stream = _stream;
    }

    void log(LOG_TYPE logType, std::string data) {
        char *timeStr = strdup("");
        if (log_time) {
            auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            timeStr = ctime(&timenow);
            timeStr[strlen(timeStr) - 1] = '\0';
        }
        switch (logType) {
            case INFO: {
                write_err("\033[1;34m[INFO]\033[0;34m ");
                if (ofs.is_open()) {
                    ofs << "[INFO] ";
                }
                break;
            }
            case WARNING: {
                write_err("\033[1;33m[WARNING]\033[0;33m ");
                if (ofs.is_open()) {
                    ofs << "[WARNING] ";
                }
                break;
            }
            case CRITICAL: {
                write_err("\033[1;31m[CRITICAL]\033[0;31m ");
                if (ofs.is_open()) {
                    ofs << "[CRITICAL] ";
                }
                break;
            }
        }
        write_err(timeStr);
        write_err(" ");
        write_err(data);
        write_err("\n");
        write_err("\033[0m");
        if (ofs.is_open()) {
            ofs << timeStr << " " << data.c_str() << std::endl;
        }
    }

};

#endif
