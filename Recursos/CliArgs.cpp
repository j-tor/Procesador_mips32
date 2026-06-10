#include <iostream>
#include <format>
#include "CliArgs.hpp"

void CliArgs::printUsage()
{
    std::cout
        << "Usage: " << program << " --file <csv file path>"
        << "\n"
        << "Options:\n"
        << "  --file <csv file path>          (Required) Specify the CSV file to work with.\n"
        << "\n";
}

std::optional<std::string> CliArgs::parse()
{
    for (int i = 0; i < argc_; i++) {
        std::string cmd = argv_[i];

        if (cmd == "--file") {
            if (i + 1 == argc_) {
                return std::string("Option '--file' requires a file path");
            }
            filepath_ = argv_[i + 1];
            i++;
        } else {
            return std::format("Invalid option '{}'", cmd);
        }
    }

    if (filepath_.empty()) {
        return std::string("Option '--file' is required");
    }

    return std::nullopt;
}
