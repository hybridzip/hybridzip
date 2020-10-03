#include <iostream>
#include <loguru/loguru.hpp>
#include <rainman/rainman.h>
#include <dotenv.h>
#include <terminal.h>
#include <hzip/api/api.h>

void setup_logger() {
#ifndef DEBUG
    loguru::g_preamble_file = false;
#endif
}

void set_unhandled_exception_handler() {
    std::set_terminate([]() {
        LOG_F(ERROR, "hzip: Encountered unhandled exception");
        abort();
    });
}

int main(int argc, const char **argv) {
    setup_logger();
    set_unhandled_exception_handler();

    auto mgr = new rainman::memmgr;

    cpp_dotenv::env.load_dotenv();
    auto& dotenv = cpp_dotenv::env;

    hz_api api;
    rinitfrom(mgr, api);

    api.limit(std::stoi(dotenv["HZIP_API_THREADS"]))
    ->process(std::stoi(dotenv["HZIP_PROCESSOR_THREADS"]))
    ->protect(dotenv["HZIP_API_KEY"])
    ->timeout(timeval{.tv_sec=std::stoi(dotenv["HZIP_API_TIMEOUT"])})
    ->start("127.0.0.1", std::stoi(dotenv["HZIP_API_PORT"]));
}