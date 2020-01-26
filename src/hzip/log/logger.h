#ifndef HYBRIDZIP_LOGGER_H
#define HYBRIDZIP_LOGGER_H

#include <iostream>
#include <string>
#include <chrono>
#include <fstream>
#include <hzip/utils/boost_utils.h>
#include <hzip/utils/errors.h>
#include <thread>
#include <sstream>
#include <mutex>

namespace LOGTYPE {
    enum __LOG_TYPE {
        INFO = 0,
        WARNING = 1,
        CRITICAL = 2,
        DEBUG = 3
    };
}

using namespace LOGTYPE;

class HZLogger {
private:
    static char *ofile;
    static std::ofstream ofs;
    static FILE *stream;
    static bool log_time;
    static std::map<std::thread::id, HZERR::ERROR_TYPE> errStatMap;
    static bool log_tid;
    static bool log_debug;
    static bool log_stat;
    static std::thread::id main_thread_id;
    static std::mutex disp_mutex;

    static void write_err(std::string data) {
        fprintf(stream, "%s", data.c_str());
    }

    static std::string get_tid() {
        auto id = std::this_thread::get_id();
        if (id == main_thread_id) {
            return "main";
        } else {
            std::stringstream str;
            str << id;
            return str.str();
        }
    }

public:
    static void setFileStream(char *_ofile = nullptr) {
        ofile = _ofile;
        if (ofile != nullptr) {
            ofs.open(ofile, std::ios_base::app);
        }
    }

    static void start() {
        if (log_stat) {
            log(CRITICAL, "Logger is running already.");
            return;
        }

        log_stat = true;
        main_thread_id = std::this_thread::get_id();
        auto log_time_p = log_time;
        log_time = false;


        if (log_debug) {
            log(INFO, "Environment: \033[1;32mDEVELOPMENT\033[0m");
            log(WARNING, "You are running in development mode. Do not use it for production.");
        } else {
            log(INFO, "Environment: \033[4m\033[1;34mPRODUCTION\033[0m");
        }


        log_time = log_time_p;
    }

    static void stop() {
        if (!log_stat) {
            log(CRITICAL, "No running logger found.");
            return;
        }

        log(WARNING, "Logging stopped.");
        log_stat = false;
    }

    static void logTime(bool opt) {
        log_time = opt;
    }

    static void logThreadId(bool opt) {
        log_tid = opt;
    }

    static void logDebug(bool opt) {
        log_debug = opt;
    }

    static void setErrStatus(HZERR::ERROR_TYPE error_code) {
        errStatMap[std::this_thread::get_id()] = error_code;
    }

    static HZERR::ERROR_TYPE getErrStatus() {
        return errStatMap[std::this_thread::get_id()];
    }

    static void setLogStream(FILE *_stream) {
        stream = _stream;
    }

    static void log(LOGTYPE::__LOG_TYPE logType, std::string data) {
        if (!log_stat || (logType == DEBUG && !log_debug)) return;



        char *timeStr = strdup("");
        if (log_time) {
            auto timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            timeStr = ctime(&timenow);
            timeStr[strlen(timeStr) - 1] = '\0';
        }

        disp_mutex.lock();
        if (log_tid) {
            write_err("\033[1;35m[Thread-");
            write_err(get_tid());
            write_err("]\033[0m ");
            ofs << "[Thread-" << gettid() << "] ";
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
            case DEBUG: {
                write_err("\033[1;36m[DEBUG]\033[0;36m ");
                if (ofs.is_open()) {
                    ofs << "[DEBUG] ";
                }
                break;
            };
        }
        write_err(timeStr);
        write_err(" ");
        write_err(data);
        write_err("\n");
        write_err("\033[0m");
        if (ofs.is_open()) {
            ofs << timeStr << " " << data.c_str() << std::endl;
        }
        disp_mutex.unlock();

    }

};

#endif
