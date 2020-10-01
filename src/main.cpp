#include <iostream>
#include <loguru/loguru.hpp>
#include <rainman/rainman.h>
#include <terminal.h>

void setup_logger(int argc, const char **argv) {
#ifndef DEBUG
    loguru::g_preamble_file = false;
#endif
    char **args_copy = new char*[argc+1];

    for (int i = 0; i < argc; i++) {
        args_copy[i] = new char[strlen(argv[i])];
        strcpy(args_copy[i], argv[i]);
    }

    int argc_copy = argc;

    loguru::init(argc_copy, args_copy);
}

void set_unhandled_exception_handler() {
    std::set_terminate([]() {
        LOG_F(ERROR, "hzip: encountered unhandled exception");
        abort();
    });
}

void init() {

}

int main(int argc, const char **argv) {
    setup_logger(argc, argv);
    set_unhandled_exception_handler();
    auto parser = build_parser();

    auto err = parser.parse(argc, argv);
    if (err) {
        LOG_F(WARNING, "args: %s", err.what().c_str());
    }

    auto mgr = new rainman::memmgr;

    if (parser.exists("p")) {
        mgr->set_peak(conv_to_bytes(parser.get<std::string>("p")));
    }

    init();

    LOG_F(INFO, "hzip: started hybridzip server");

    return 0;
}