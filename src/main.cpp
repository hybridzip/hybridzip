#include <csignal>
#include <loguru/loguru.hpp>
#include <rainman/rainman.h>
#include <hzip_network/api/api.h>
#include <hzip_core/config.h>

std::function<void()> _hzapi_graceful_shutdown;

void setup_logger() {
#ifndef DEBUG
    loguru::g_preamble_file = false;
#endif
}

void set_unhandled_exception_handler() {
    std::set_terminate([]() {
        LOG_F(ERROR, "hybridzip: Encountered unhandled exception");
        abort();
    });
}

void set_signal_handlers() {
    auto signal_handler = [](int signum) {
        LOG_F(ERROR, "hybridzip: Signal captured: %s", strsignal(signum));
        _hzapi_graceful_shutdown();
        exit(signum);
    };

    auto signal_ignore = [](int signum) {
        LOG_F(ERROR, "hybridzip: Signal captured: %s", strsignal(signum));
        LOG_F(WARNING, "hybridzip: Ignoring signal: %s", strsignal(signum));
    };

    signal(SIGINT, signal_handler);
    signal(SIGSEGV, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGABRT, signal_handler);
    signal(SIGFPE, signal_handler);
    signal(SIGPIPE, signal_ignore);
}

int main(int argc, const char **argv) {
    setup_logger();
    set_unhandled_exception_handler();

    Config::configure();

    rainman::Allocator().peak_size(Config::host_max_memory);
    LOG_F(INFO, "hybridzip: Max memory usage set to %lu bytes", rainman::Allocator().peak_size());

    auto api = new hzapi::Api(Config::api_threads);

    _hzapi_graceful_shutdown = [api]() {
        LOG_F(WARNING, "Gracefully shutting down.");
        api->shutdown();
    };

    set_signal_handlers();

    api->process(Config::processor_threads)
            ->protect(Config::api_key)
            ->timeout(timeval{.tv_sec=Config::api_timeout})
            ->start("127.0.0.1", Config::api_port);
}