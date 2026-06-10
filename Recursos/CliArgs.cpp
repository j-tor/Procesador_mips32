#include <iostream>
#include <format>
#include "CliArgs.hpp"

void CliArgs::printUsage()
{
    std::cout
        << "Usage: " << program << " --program <program path> --font <font path> --data <data path>"
        << "\n"
        << "Options:\n"
        << "  --program <program path>      (Required) Specify the binary program file in machine language.\n"
        << "  --font <font path>            (Required) Specify the MIF file with the font.\n"
        << "  --data <data path>            (Required) Specify the data file.\n"
        << "\n";
}

std::optional<std::string> CliArgs::parse()
{
    for (int i = 0; i < argc_; i++) {
        std::string cmd = argv_[i];

        if (cmd == "--program") {
            if (i + 1 == argc_) {
                return std::string("Option '--program' requires a file path");
            }
            programPath_ = argv_[i + 1];
            i++;
        } else if (cmd == "--font") {
            if (i + 1 == argc_) {
                return std::string("Option '--font' requires a file path");
            }
            fontPath_ = argv_[i + 1];
            i++;
        } else if (cmd == "--data") {
            if (i + 1 == argc_) {
                return std::string("Option '--data' requires a file path");
            }
            dataPath_ = argv_[i + 1];
            i++;
        } else {
            return std::format("Invalid option '{}'", cmd);
        }
    }

    if (programPath_.empty()) {
        return std::string("Option '--program' is required");
    }
    if (fontPath_.empty()) {
        return std::string("Option '--font' is required");
    }
    if(dataPath_.empty()) {
        return std::string("Option '--data' is required");
    }

    return std::nullopt;
}
