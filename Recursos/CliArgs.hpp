#pragma once

#include <string>
#include <optional>

class CliArgs
{
public:
    CliArgs(int argc, const char *argv[])
      : argc_(argc - 1), argv_(argv + 1), program(argv[0])
    {}

    std::string programPath() const
    { return programPath_; }

    std::string fontPath() const
    { return fontPath_; }

    std::string dataPath() const
    { return dataPath_; }

    void printUsage();

    [[nodiscard]] std::optional<std::string> parse();
    
private:
    int argc_;
    const char **argv_;
    std::string program{};
    std::string programPath_{};
    std::string fontPath_{};
    std::string dataPath_{};
};