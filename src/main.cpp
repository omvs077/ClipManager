#include "common.h"
#include "clipboard.h"
#include "tray.h"
#include "popup.h"
#include "storage.h"
#include "detector.h"
#include "settings.h"

static std::vector<ClipEntry> g_history;
static Tray  g_tray;
static Popup g_popup;
static Settings g_settings;
static bool  g_ignoreNextClipboard = false;

static void OnClipboardUpdate(HWND hwnd);
static void OnPopupSelect(HWND hwnd, int index);

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    
    case WM_CREATE:
        RegisterHotKey(hwnd, HOTKEY_SHOW,  MOD_WIN | MOD_NOREPEAT, 'V');
        RegisterHotKey(hwnd, HOTKEY_PLAIN, MOD_WIN | MOD_SHIFT | MOD_NOREPEAT, 'V');
        Clipboard::StartListening(hwnd);
        Storage::LoadHistory(g_history);
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

static void OnClipboardUpdate(HWND hwnd) {
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
    g_settings.Current.historyLimit    = MAX_HISTORY;
    g_settings.Current.startWithWindows = true;

    g_settings.OnSave = [](const AppSettings& s) {
    // Apply history limit
    if (s.historyLimit != -1 && (int)g_history.size() > s.historyLimit)
        g_history.resize(s.historyLimit);

    // Apply pause monitoring
    g_ignoreNextClipboard = s.pauseMonitoring;

    // Apply clear on exit (stored in settings, handled at WM_DESTROY)
    // Apply auto-delete by days
    if (s.autoDeleteDays != -1) {
        // Remove entries older than X days
        // (timestamp support coming in next phase — skip for now)
    }

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

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    CloseHandle(hMutex);
    return (int)msg.wParam;
}