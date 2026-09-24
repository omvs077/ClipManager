#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WINVER
#define WINVER        0x0601
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT  0x0601
#endif

#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <shlobj.h>
#include <ctime>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

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
#endif

#define APP_NAME      L"ClipManager"
#define APP_VERSION   L"1.1.1"
#define WM_TRAY       (WM_APP + 1)
#define WM_SHOW_POPUP (WM_APP + 2)
#define WM_SHOW_SETTINGS (WM_APP + 3)
#define HOTKEY_SHOW   1
#define TRAY_ICON_ID  1

// Absolute hard ceiling regardless of the user's configured historyLimit --
// exists only to bound memory/disk use if settings are ever corrupted or
// unset. The real, user-facing limit is AppSettings::historyLimit; trim
// logic should clamp to min(settings.historyLimit, MAX_HISTORY_CEILING).
#define MAX_HISTORY_CEILING 2000

enum class ClipType {
    Text,
    URL,
    Color,
    FilePath,
    Email,
    Image,
    FileRef
};

struct ClipEntry {
    std::wstring text;
    ClipType     type = ClipType::Text;
    bool         pinned = false;
    time_t       timestamp = 0;
    std::wstring imagePath;
    std::vector<std::wstring> filePaths;
};

struct Snippet {
    std::wstring name;
    std::wstring text;
};

