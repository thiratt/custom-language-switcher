#include "App.h"
#ifdef __linux__
#include <iostream>

void App::run(const Config& config, const std::string& configPath) {
    std::cout << "Running on Linux (Not implemented yet)" << std::endl;
}
#endif