#include "common.h"
#include "clipboard.h"
#include "tray.h"
#include "popup.h"
#include "storage.h"
#include "detector.h"

static std::vector<ClipEntry> g_history;
static Tray  g_tray;
static Popup g_popup;
static bool  g_ignorNextClipboard = false;

static void OnClipboardUpdate(HWND hwnd);
static void OnPopupSelect(HWND hwnd, int index, bool plainText = false);

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        RegisterHotKey(hwnd, HOTKEY_SHOW,  MOD_WIN | MOD_NOREPEAT, 'V');
        RegisterHotKey(hwnd, HOTKEY_PLAIN, MOD_WIN | MOD_SHIFT | MOD_NOREPEAT, 'V');
        Clipboard::StartListening(hwnd);
        Storage::LoadHistory(g_history);
        return 0;

    case WM_CLIPBOARDUPDATE:
        if (!g_ignorNextClipboard)
            OnClipboardUpdate(hwnd);
        g_ignorNextClipboard = false;
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
        Storage::SaveHistory(g_history);
        g_tray.Destroy();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static void OnClipboardUpdate(HWND hwnd) {
    std::wstring text = Clipboard::ReadText();
    if (text.empty()) return;
    if (!g_history.empty() && g_history.front().text == text) return;

    auto it = std::find_if(g_history.begin(), g_history.end(),
        [&](const ClipEntry& e){ return e.text == text; });
    
    bool wasPinned = false;
    if (it != g_history.end()) {
        wasPinned = it->pinned;
        g_history.erase(it);
    }

    ClipEntry entry;
    entry.text   = text;
    entry.type   = Detector::Detect(text);
    entry.pinned = wasPinned;

    // Insert after pinned items
    auto insertPos = std::find_if(g_history.begin(), g_history.end(),
        [](const ClipEntry& e){ return !e.pinned; });
    g_history.insert(insertPos, entry);

    if (g_history.size() > MAX_HISTORY)
        g_history.resize(MAX_HISTORY);

    Storage::SaveHistory(g_history);
}

static void OnPopupSelect(HWND hwnd, int index, bool plainText) {
    if (index < 0 || index >= (int)g_history.size()) return;

    const std::wstring& text = g_history[index];

    g_ignorNextClipboard = true;

    if (plainText)
        Clipboard::WritePlainText(hwnd, text);
    else
        Clipboard::WriteText(hwnd, text);

    // Bubble to top (preserve pin)
    ClipEntry selected = g_history[index];
    g_history.erase(g_history.begin() + index);
    auto insertPos = selected.pinned ? g_history.begin() :
        std::find_if(g_history.begin(), g_history.end(),
            [](const ClipEntry& e){ return !e.pinned; });
    g_history.insert(insertPos, selected);
    Storage::SaveHistory(g_history);

    Sleep(50);
    INPUT inputs[4] = {};
    inputs[0].type = INPUT_KEYBOARD; inputs[0].ki.wVk = VK_CONTROL;
    inputs[1].type = INPUT_KEYBOARD; inputs[1].ki.wVk = 'V';
    inputs[2].type = INPUT_KEYBOARD; inputs[2].ki.wVk = 'V';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[3].type = INPUT_KEYBOARD; inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;
    inputs[3].ki.wVk = VK_CONTROL;
    SendInput(4, inputs, sizeof(INPUT));
}

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR, int) {
    HANDLE hMutex = CreateMutexW(nullptr, TRUE, L"ClipManagerMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) { CloseHandle(hMutex); return 0; }

    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_WIN95_CLASSES };
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"ClipManagerMain";
    RegisterClassExW(&wc);

    HWND hwnd = CreateWindowExW(0, L"ClipManagerMain", APP_NAME,
        WS_OVERLAPPEDWINDOW, 0, 0, 0, 0,
        nullptr, nullptr, hInst, nullptr);
    if (!hwnd) return 1;

    if (!g_tray.Create(hwnd, hInst)) return 1;
    if (!g_popup.Create(hInst)) return 1;

    g_popup.OnSelect = [hwnd](int i) { OnPopupSelect(hwnd, i, false); };
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