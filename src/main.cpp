#include <signal.h>
#include <loguru/loguru.hpp>
#include <dotenv.h>
#include <rainman/rainman.h>
#include <hzip/api/api.h>

std::function<void()> _hzapi_graceful_shutdown;

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

void check_env(cpp_dotenv::dotenv &dotenv) {
    const char *required_vars[] = {
            "HZIP_API_THREADS",
            "HZIP_PROCESSOR_THREADS",
            "HZIP_API_KEY",
            "HZIP_API_TIMEOUT",
            "HZIP_API_PORT"
    };

    for (auto var : required_vars) {
        if (dotenv[var].empty()) {
            LOG_F(ERROR, "hzip: %s was not set in environment", var);
            exit(0);
        }
    }
}

void set_signal_handlers() {
    auto signal_handler = [](int signum) {
        LOG_F(ERROR, "hzip: Signal captured %d", signum);
        _hzapi_graceful_shutdown();
        exit(signum);
    };

    signal(SIGINT, signal_handler);
    signal(SIGSEGV, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGABRT, signal_handler);
}

int main(int argc, const char **argv) {
    setup_logger();
    set_unhandled_exception_handler();

    auto mgr = new rainman::memmgr;

    cpp_dotenv::env.load_dotenv();
    auto &dotenv = cpp_dotenv::env;

    check_env(dotenv);

    auto api = new hzapi::hz_api;
    rinitptrfrom(mgr, api);

    _hzapi_graceful_shutdown = [api]() {
        LOG_F(WARNING, "Gracefully shutting down.");
        api->shutdown();
    };

    set_signal_handlers();

    api->limit(std::stoi(dotenv["HZIP_API_THREADS"]))
            ->process(std::stoi(dotenv["HZIP_PROCESSOR_THREADS"]))
            ->protect(dotenv["HZIP_API_KEY"])
            ->timeout(timeval{.tv_sec=std::stoi(dotenv["HZIP_API_TIMEOUT"])})
            ->start("127.0.0.1", std::stoi(dotenv["HZIP_API_PORT"]));
}