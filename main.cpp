#include "App.h"
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

void LoadConfig(Config& config, const std::string& path) {
#ifdef _WIN32
    config.longPressDuration = GetPrivateProfileIntA("Settings", "LongPressDuration", 200, path.c_str());
    config.customKey = GetPrivateProfileIntA("Settings", "CustomKey", 0x14, path.c_str());
    config.enableOemOsd = GetPrivateProfileIntA("Settings", "EnableOemOsd", 1, path.c_str()) != 0;
#endif
}

void SaveConfig(const Config& config, const std::string& path) {
#ifdef _WIN32
    WritePrivateProfileStringA("Settings", "LongPressDuration", std::to_string(config.longPressDuration).c_str(), path.c_str());
    WritePrivateProfileStringA("Settings", "CustomKey", std::to_string(config.customKey).c_str(), path.c_str());
    WritePrivateProfileStringA("Settings", "EnableOemOsd", config.enableOemOsd ? "1" : "0", path.c_str());
#endif
}

int main(int argc, char* argv[]) {
    Config config;
    std::string configPath = "config.ini";

#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string exePath(buffer);
    std::string::size_type pos = exePath.find_last_of("\\/");
    configPath = exePath.substr(0, pos) + "\\config.ini";
#endif

    LoadConfig(config, configPath);

    std::vector<std::string> args(argv + 1, argv + argc);
    bool save = false;

    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == "--duration" && i + 1 < args.size()) {
            config.longPressDuration = std::stoi(args[i + 1]);
            save = true;
            i++;
        } else if (args[i] == "--key" && i + 1 < args.size()) {
            config.customKey = std::stoi(args[i + 1]);
            save = true;
            i++;
        } else if (args[i] == "--help") {
            std::cout << "Usage: CustomLanguageSwitcher [options]\n"
                      << "Options:\n"
                      << "  --duration <ms>   Set long press duration (default: 200)\n"
                      << "  --key <vk_code>   Set custom key code (default: 20)\n"
                      << "  --help            Show this help message\n";
            return 0;
        }
    }

    if (save) {
        SaveConfig(config, configPath);
    }

    App app;
    app.run(config, configPath);
    return 0;
}