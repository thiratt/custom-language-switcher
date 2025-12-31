# Custom Language Switcher

A simple tool to switch your typing language faster by remapping a key (default is Caps Lock).

## Disclaimer
This tool works great on my machine, but since it uses low-level keyboard hooks, things might behave differently on your system.

Use it at your own risk! I am not responsible for any system crashes, input lag, or if your Caps Lock starts acting possessed. Always test it out before relying on it for important work.

## Features

-   **Smart Switching**:
    -   **Short Press**: Switches input language (Simulates `Win + Space`).
    -   **Long Press**: Performs the original key action (e.g., toggles Caps Lock).
-   **Customizable**: Configure the trigger key and the long-press duration.
-   **System Tray Integration**:
    -   **Reload Hook**: Re-applies the keyboard hook if it gets detached.
    -   **Enable OEM OSD**: Toggles support for manufacturer On-Screen Displays (e.g., for Caps Lock).
    -   **Pause**: Temporarily disable the switcher.
-   **Persistent Configuration**: Settings are saved to `config.ini`.
-   **Command Line Interface**: Configure settings via startup arguments.\* **Cross-Platform**:
    -   **Windows**: Full support with installer and auto-run.
    -   **Linux**: 🚧 Planned / Coming Soon.

## Usage

1.  Run `CustomLanguageSwitcher.exe`.
2.  The app runs in the background. Check the system tray icon.
3.  **Tap** Caps Lock to switch language.
4.  **Hold** Caps Lock (default 200ms) to toggle actual Caps Lock.

### Configuration

The application creates a `config.ini` file in the same directory. You can edit this file or use command-line arguments.

**CLI Arguments:**

-   `--duration <ms>`: Set the long-press duration in milliseconds (default: 200).
-   `--key <vk_code>`: Set the custom key using its Virtual Key code (decimal). Default is `20` (0x14) for Caps Lock.
-   `--help`: Show help message.

**Example:**

```cmd
CustomLanguageSwitcher.exe --duration 300 --key 20
```

**Virtual Key Codes:**
Common codes:

-   Caps Lock: `20`
-   Right Alt: `165`
-   F1: `112`
-   (See [Microsoft Virtual-Key Codes](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes) for more).

## Building

### Prerequisites

-   **CMake** (3.10 or later)
-   **C++ Compiler** (MSVC recommended for Windows)
-   **Python 3** (for the build script)

### Build using Script

A helper script `build.py` is provided to automate the build process.

```cmd
python build.py
```

This will create a `build/win` directory and generate the executable in `build/win/Release/CustomLanguageSwitcher.exe`.

### Build using CMake manually

```cmd
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

## Installation

### Windows Installer (Recommended)

For a complete installation with auto-start capability:

1.  **Build the project** first (see [Building](#building)).
2.  Download and install [Inno Setup](https://jrsoftware.org/isinfo.php).
3.  Go to the `iss/` folder, rename `install.iss.example` to `install.iss`, and open it.
4.  **Make it yours**: Find `#define MyAppPublisher` and replace it with your own name.
5.  Press **F9** in Inno Setup. The compiler will handle the rest and pop up the installer for you.

The installer will:

-   Install the application to `Program Files`.
-   Create a **Scheduled Task** to automatically start the app with high privileges when you log in (bypassing UAC prompts).
-   Create desktop and start menu shortcuts.

### Portable (Manual)

You can simply copy the `CustomLanguageSwitcher.exe` to any folder and run it. To have it start automatically with Windows, you can create a shortcut in your Startup folder.