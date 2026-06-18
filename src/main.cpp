#include "common.h"
#include "clipboard.h"
#include "tray.h"
#include "popup.h"
#include "storage.h"
#include "detector.h"
#include "settings.h"

static void ShowToast(HWND hwnd, const std::wstring& text) {
    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd   = hwnd;
    nid.uID    = TRAY_ICON_ID;
    nid.uFlags = NIF_INFO;
    nid.dwInfoFlags = NIIF_INFO;
    wcscpy_s(nid.szInfoTitle, L"ClipManager");

    std::wstring preview = text.substr(0, 60);
    for (wchar_t& c : preview) if (c == L'\n' || c == L'\r') c = L' ';
    wcscpy_s(nid.szInfo, preview.c_str());

    Shell_NotifyIconW(NIM_MODIFY, &nid);
}

static std::vector<ClipEntry> g_history;
static Tray  g_tray;
static Popup g_popup;
static Settings g_settings;
static bool  g_ignoreNextClipboard = false;

static void OnClipboardUpdate(HWND hwnd);
static void OnPopupSelect(HWND hwnd, int index);
static void ApplyAutoDelete(int days);

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    
    case WM_CREATE:
        RegisterHotKey(hwnd, HOTKEY_SHOW,  MOD_WIN | MOD_NOREPEAT, 'V');
        RegisterHotKey(hwnd, HOTKEY_PLAIN, MOD_WIN | MOD_SHIFT | MOD_NOREPEAT, 'V');
        Clipboard::StartListening(hwnd);
        Storage::LoadHistory(g_history);
        ApplyAutoDelete(g_settings.Current.autoDeleteDays);
        return 0;


    case WM_CLIPBOARDUPDATE:
        if (!g_ignoreNextClipboard)
            OnClipboardUpdate(hwnd);
        g_ignoreNextClipboard = false;
        return 0;

    case WM_HOTKEY:
        if (wParam == HOTKEY_SHOW) {
            if (g_popup.IsVisible()) g_popup.Hide();
            else g_popup.Show(g_history);
        }
        return 0;

    case WM_TRAY:
        if (lParam == WM_RBUTTONUP || lParam == WM_LBUTTONUP)
            g_tray.ShowContextMenu(hwnd);
        return 0;

    case WM_SHOW_POPUP:
        g_popup.Show(g_history);
        return 0;

    case WM_CLOSE:
        if (g_settings.Current.minimizeToTray) {
            ShowWindow(hwnd, SW_HIDE);
            return 0;
        }
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        UnregisterHotKey(hwnd, HOTKEY_SHOW);
        UnregisterHotKey(hwnd, HOTKEY_PLAIN);
        Clipboard::StopListening(hwnd);
        if (g_settings.Current.clearOnExit)
            g_history.clear();
        Storage::SaveHistory(g_history);
        g_tray.Destroy();
        PostQuitMessage(0);
        return 0;
    
    case WM_SHOW_SETTINGS:
        g_settings.Show();
        return 0;

    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static void ApplyAutoDelete(int days) {
    if (days <= 0) return; // -1 = never
    time_t cutoff = time(nullptr) - (days * 24 * 60 * 60);
    g_history.erase(
        std::remove_if(g_history.begin(), g_history.end(),
            [cutoff](const ClipEntry& e) {
                return !e.pinned && e.timestamp < cutoff;
            }),
        g_history.end());
}

static bool IsPasswordManagerActive() {
    HWND fg = GetForegroundWindow();
    if (!fg) return false;

    wchar_t title[256] = {};
    GetWindowTextW(fg, title, 256);

    DWORD pid = 0;
    GetWindowThreadProcessId(fg, &pid);

    wchar_t procName[MAX_PATH] = {};
    HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProc) {
        DWORD size = MAX_PATH;
        QueryFullProcessImageNameW(hProc, 0, procName, &size);
        CloseHandle(hProc);
    }

    std::wstring proc = procName;
    std::transform(proc.begin(), proc.end(), proc.begin(), ::towlower);

    static const wchar_t* blocked[] = {
        L"1password", L"bitwarden", L"keepass", L"lastpass",
        L"dashlane", L"nordpass", L"keeper"
    };
    for (auto* name : blocked) {
        if (proc.find(name) != std::wstring::npos) return true;
    }
    return false;
}

static void OnClipboardUpdate(HWND hwnd) {
    if (g_settings.Current.excludePasswords && IsPasswordManagerActive())
        return;

    std::wstring text = Clipboard::ReadText();
    if (text.empty()) return;
    if (!g_history.empty() && g_history.front().text == text) return;

    // Remove duplicate
    for (int i = 0; i < (int)g_history.size(); i++) {
        if (g_history[i].text == text) {
            g_history.erase(g_history.begin() + i);
            break;
        }
    }

    ClipEntry entry;
    entry.text   = text;
    entry.type   = Detector::Detect(text);
    entry.pinned = false;
    entry.timestamp = time(nullptr);

    // Insert after pinned items
    int insertAt = 0;
    for (int i = 0; i < (int)g_history.size(); i++) {
        if (g_history[i].pinned) insertAt = i + 1;
        else break;
    }
    g_history.insert(g_history.begin() + insertAt, entry);

    if (g_history.size() > MAX_HISTORY)
        g_history.resize(MAX_HISTORY);

    Storage::SaveHistory(g_history);
    if (g_settings.Current.showNotifications)
    ShowToast(hwnd, text);
}

static void OnPopupSelect(HWND hwnd, int index) {
    if (index < 0 || index >= (int)g_history.size()) return;

    std::wstring text = g_history[index].text;
    bool wasPinned    = g_history[index].pinned;

    g_ignoreNextClipboard = true;
    Clipboard::WriteText(hwnd, text);

    // Bubble to top (after pinned items)
    ClipEntry selected = g_history[index];
    g_history.erase(g_history.begin() + index);

    int insertAt = 0;
    if (!selected.pinned) {
        for (int i = 0; i < (int)g_history.size(); i++) {
            if (g_history[i].pinned) insertAt = i + 1;
            else break;
        }
    }
    g_history.insert(g_history.begin() + insertAt, selected);
    Storage::SaveHistory(g_history);

    Sleep(50);
    INPUT inputs[4] = {};
    inputs[0].type = INPUT_KEYBOARD; inputs[0].ki.wVk = VK_CONTROL;
    inputs[1].type = INPUT_KEYBOARD; inputs[1].ki.wVk = 'V';
    inputs[2].type = INPUT_KEYBOARD; inputs[2].ki.wVk = 'V';   inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[3].type = INPUT_KEYBOARD; inputs[3].ki.wVk = VK_CONTROL; inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
    SendInput(4, inputs, sizeof(INPUT));
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR, int) {
    HANDLE hMutex = CreateMutexW(nullptr, TRUE, L"ClipManagerMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) { CloseHandle(hMutex); return 0; }

    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_WIN95_CLASSES };
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc   = {};
    wc.cbSize         = sizeof(wc);
    wc.lpfnWndProc    = WndProc;
    wc.hInstance      = hInst;
    wc.lpszClassName  = L"ClipManagerMain";
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(0, L"ClipManagerMain", APP_NAME,
        WS_OVERLAPPEDWINDOW, 0, 0, 0, 0,
        nullptr, nullptr, hInst, nullptr);
    if (!hwnd) return 1;

    if (!g_tray.Create(hwnd, hInst)) return 1;
    if (!g_popup.Create(hInst))      return 1;
    if (!g_settings.Create(hInst))      return 1;
    g_popup.SetCompactMode(g_settings.Current.compactMode);
    g_popup.SetShowTimestamps(g_settings.Current.showTimestamps);
    g_settings.Current.historyLimit    = MAX_HISTORY;
    g_settings.Current.startWithWindows = true;

    g_settings.OnSave = [](const AppSettings& s) {
        if (s.historyLimit != -1 && (int)g_history.size() > s.historyLimit)
            g_history.resize(s.historyLimit);
        g_ignoreNextClipboard = s.pauseMonitoring;
        ApplyAutoDelete(s.autoDeleteDays);
        g_popup.SetCompactMode(s.compactMode);
        g_popup.SetShowTimestamps(s.showTimestamps);
        Storage::SaveHistory(g_history);
    };

    g_settings.OnClearHistory = []() {
        g_history.clear();
        Storage::SaveHistory(g_history);
    };

    g_settings.OnPauseToggle = [](bool paused) {
        g_ignoreNextClipboard = paused;
    };

    g_popup.OnSelect = [hwnd](int i) { OnPopupSelect(hwnd, i); };
    g_popup.OnPin    = [](int)       { Storage::SaveHistory(g_history); };
    g_popup.OnDelete = [](int)       { Storage::SaveHistory(g_history); };
    g_popup.OnOpenUrl = [](const std::wstring& url) {
        ShellExecuteW(nullptr, L"open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    };

    g_popup.OnOpenPath = [](const std::wstring& path) {
        ShellExecuteW(nullptr, L"open", L"explorer.exe",
            (L"/select,\"" + path + L"\"").c_str(), nullptr, SW_SHOWNORMAL);
    };

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    CloseHandle(hMutex);
    return (int)msg.wParam;
}