#ifndef HYBRIDZIP_TERMINAL_H
#define HYBRIDZIP_TERMINAL_H

#include <cstdint>
#include <argparse/argparse.h>

argparse::ArgumentParser build_parser() {
    argparse::ArgumentParser parser("hybridzip", "A unified compression platform");
    parser.add_argument()
            .names({"-p", "--peak-size"})
            .description("Set peak size for the process")
            .required(false);

    parser.add_argument()
            .names({"-v", "--verbose"})
            .description("Set verbosity level")
            .required(false);

    parser.enable_help();

    return parser;
}

uint64_t conv_to_bytes(const std::string &s) {
    if (s.ends_with("kb")) {
        return std::stoull(s.substr(0, s.length() - 2)) << 10;
    } else if (s.ends_with("mb")) {
        return std::stoull(s.substr(0, s.length() - 2)) << 20;
    } else if (s.ends_with("gb")) {
        return std::stoull(s.substr(0, s.length() - 2)) << 30;
    } else if (s.ends_with("b")) {
        return std::stoull(s.substr(0, s.length() - 1));
    } else {
        return 0;
    }
}


#endif
