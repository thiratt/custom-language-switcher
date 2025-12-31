#pragma once
#include <string>

struct Config {
    int longPressDuration = 200;
    int customKey = 0x14;
    bool enableOemOsd = true;
};

class App {
public:
    void run(const Config& config, const std::string& configPath);
};