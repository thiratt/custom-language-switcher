#include "App.h"
#include "resource.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>

namespace {
    #define WM_TRAYICON (WM_USER + 1)
    constexpr UINT ID_TRAY_EXIT = 1001;
    constexpr UINT ID_TRAY_RELOAD = 1002;
    constexpr UINT ID_TRAY_ENABLE_OEM_OSD = 1003;
    constexpr UINT ID_TRAY_PAUSE = 1004;
    constexpr UINT IDT_TIMER_AUTO_REHOOK = 999;

    HHOOK g_hKeyboardHook = nullptr;
    UINT_PTR g_timerId = 0;
    bool g_ignoreNextKeyRepeat = false;
    bool g_isPaused = false;
    bool g_enableOemOsd = true;
    NOTIFYICONDATA nid = {};
    HWND g_hwnd = nullptr;
    HANDLE g_hMutex = nullptr;
    int g_longPressDuration = 200;
    int g_customKey = VK_CAPITAL;
    std::string g_configPath;
}

static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

static void ReinstallHook() {
    if (g_hKeyboardHook) {
        UnhookWindowsHookEx(g_hKeyboardHook);
        g_hKeyboardHook = nullptr;
    }
    Sleep(100);
    g_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);
}

static void SwitchInputLanguage() noexcept {
    INPUT inputs[4] = {};
    inputs[0].type = INPUT_KEYBOARD; inputs[0].ki.wVk = VK_LWIN;
    inputs[1].type = INPUT_KEYBOARD; inputs[1].ki.wVk = VK_SPACE;
    inputs[2].type = INPUT_KEYBOARD; inputs[2].ki.wVk = VK_SPACE; inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[3].type = INPUT_KEYBOARD; inputs[3].ki.wVk = VK_LWIN; inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(4, inputs, sizeof(INPUT));
}

static void ToggleCapsLock() noexcept {
    if (g_enableOemOsd) {
        UINT scanCode = MapVirtualKey(g_customKey, MAPVK_VK_TO_VSC);
        INPUT inputs[2] = {};
        inputs[0].type = INPUT_KEYBOARD; inputs[0].ki.wScan = (WORD)scanCode; inputs[0].ki.dwFlags = KEYEVENTF_SCANCODE;
        inputs[1].type = INPUT_KEYBOARD; inputs[1].ki.wScan = (WORD)scanCode; inputs[1].ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
        SendInput(2, inputs, sizeof(INPUT));
    }
    else {
        keybd_event((BYTE)g_customKey, 0, 0, 0);
        keybd_event((BYTE)g_customKey, 0, KEYEVENTF_KEYUP, 0);
    }
}

static void CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD) {
    if (g_timerId) { KillTimer(nullptr, g_timerId); g_timerId = 0; }
    ToggleCapsLock();
    g_ignoreNextKeyRepeat = true;
}

static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode != HC_ACTION) return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
    if (g_isPaused) return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);

    const auto* pKeyBoard = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
    if (pKeyBoard->vkCode != (DWORD)g_customKey) return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
    if (pKeyBoard->flags & LLKHF_INJECTED) return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);

    if (wParam == WM_KEYDOWN) {
        if (g_ignoreNextKeyRepeat || g_timerId) return 1;
        g_timerId = SetTimer(nullptr, 0, g_longPressDuration, TimerProc);
        return 1;
    }
    if (wParam == WM_KEYUP) {
        if (g_timerId) { KillTimer(nullptr, g_timerId); g_timerId = 0; SwitchInputLanguage(); }
        g_ignoreNextKeyRepeat = false;
        return 1;
    }
    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}

static void AddTrayIcon(HWND hwnd) {
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;

    nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));

    lstrcpy(nid.szTip, TEXT("Windows Language Switcher"));

    for (int i = 0; i < 5; i++) {
        if (Shell_NotifyIcon(NIM_ADD, &nid)) {
            break;
        }
        Sleep(1000);
    }
}

static void RemoveTrayIcon() {
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        AddTrayIcon(hwnd);
        SetTimer(hwnd, IDT_TIMER_AUTO_REHOOK, 7000, NULL);
        break;

    case WM_TIMER:
        if (wParam == IDT_TIMER_AUTO_REHOOK) {
            KillTimer(hwnd, IDT_TIMER_AUTO_REHOOK);
            ReinstallHook();
        }
        break;

    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP) {
            POINT curPoint;
            GetCursorPos(&curPoint);
            SetForegroundWindow(hwnd);

            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, ID_TRAY_RELOAD, TEXT("Reload Hook"));

            UINT oemFlags = MF_STRING;
            if (g_enableOemOsd) oemFlags |= MF_CHECKED;

            UINT pauseFlags = MF_STRING;
            if (g_isPaused) pauseFlags |= MF_CHECKED;

            AppendMenu(hMenu, oemFlags, ID_TRAY_ENABLE_OEM_OSD, TEXT("Enable OEM OSD"));
            AppendMenu(hMenu, pauseFlags, ID_TRAY_PAUSE, TEXT("Pause"));
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hMenu, MF_STRING, ID_TRAY_EXIT, TEXT("Exit"));

            TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, curPoint.x, curPoint.y, 0, hwnd, NULL);
            DestroyMenu(hMenu);
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_TRAY_RELOAD: ReinstallHook(); break;
        case ID_TRAY_ENABLE_OEM_OSD: 
            g_enableOemOsd = !g_enableOemOsd; 
            WritePrivateProfileStringA("Settings", "EnableOemOsd", g_enableOemOsd ? "1" : "0", g_configPath.c_str());
            break;
        case ID_TRAY_PAUSE: g_isPaused = !g_isPaused; break;
        case ID_TRAY_EXIT: DestroyWindow(hwnd); break;
        }
        break;
    case WM_DESTROY:
        RemoveTrayIcon();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

void App::run(const Config& config, const std::string& configPath) {
    g_longPressDuration = config.longPressDuration;
    g_customKey = config.customKey;
    g_enableOemOsd = config.enableOemOsd;
    g_configPath = configPath;

    HINSTANCE hInstance = GetModuleHandle(NULL);
    
    g_hMutex = CreateMutex(NULL, TRUE, TEXT("Global\\MyCapsLockSwitcherMutex"));
    if (GetLastError() == ERROR_ALREADY_EXISTS) return;

    const TCHAR CLASS_NAME[] = TEXT("CustomLanguageSwitcherWindowsClass");
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    g_hwnd = CreateWindowEx(0, CLASS_NAME, TEXT("Custom Language Switcher"), 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);
    if (!g_hwnd) return;

    g_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (g_hKeyboardHook) UnhookWindowsHookEx(g_hKeyboardHook);
    if (g_timerId) KillTimer(nullptr, g_timerId);
    if (g_hMutex) CloseHandle(g_hMutex);
}

#endif