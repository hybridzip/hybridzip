#include "logger.h"

char *HZLogger::ofile = nullptr;
std::ofstream HZLogger::ofs = std::ofstream();
FILE *HZLogger::stream = stderr;
bool HZLogger::log_time = false;
std::map<std::thread::id, HZERR::ERROR_TYPE> HZLogger::errStatMap = std::map<std::thread::id, HZERR::ERROR_TYPE>();
bool HZLogger::log_tid = false;
bool HZLogger::log_debug = false;
bool HZLogger::log_stat = false;
std::thread::id HZLogger::main_thread_id = std::thread::id();
std::mutex HZLogger::disp_mutex = std::mutex();