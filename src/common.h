#pragma once

#define WINVER        0x0601
#define _WIN32_WINNT  0x0601
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <shlobj.h>
#include <commdlg.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")

#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <fstream>
#include <sstream>

#ifndef MOD_NOREPEAT
#define MOD_NOREPEAT 0x4000
#endif

#ifndef WC_LISTBOXW
#define WC_LISTBOXW L"ListBox"
// Content types
enum class ClipType {
    Text,
    URL,
    Color,    // #RRGGBB hex
    FilePath,
    Email
};

// Clip entry — now stores type and pin state
struct ClipEntry {
    std::wstring text;
    ClipType     type    = ClipType::Text;
    bool         pinned  = false;
};

// Second hotkey: Win+Shift+V for plain-text paste
#define HOTKEY_PLAIN  2
#endif

// App identity
#define APP_NAME      L"ClipManager"
#define APP_VERSION   L"1.0.0"
#define WM_TRAY       (WM_APP + 1)
#define WM_SHOW_POPUP (WM_APP + 2)

// Hotkey ID
#define HOTKEY_SHOW   1

// Max history entries
#define MAX_HISTORY   50

// Tray icon ID
#define TRAY_ICON_ID  1