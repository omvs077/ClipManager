#include "tray.h"
#include "common.h"

// Context menu item IDs
#define IDM_SHOW    1001
#define IDM_EXIT    1002
#define IDM_SETTINGS 1003

bool Tray::Create(HWND hwnd, HINSTANCE hInst) {
    ZeroMemory(&m_nid, sizeof(m_nid));
    m_nid.cbSize = sizeof(m_nid);
    m_nid.hWnd   = hwnd;
    m_nid.uID    = TRAY_ICON_ID;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = WM_TRAY;

    // Load icon — falls back to default app icon
    m_nid.hIcon = LoadIconW(hInst, MAKEINTRESOURCEW(1));
    if (!m_nid.hIcon)
        m_nid.hIcon = LoadIconW(nullptr, IDI_APPLICATION);

    wcscpy_s(m_nid.szTip, APP_NAME);

    return Shell_NotifyIconW(NIM_ADD, &m_nid) != FALSE;
}

void Tray::Destroy() {
    Shell_NotifyIconW(NIM_DELETE, &m_nid);
}

void Tray::ShowContextMenu(HWND hwnd) {
    HMENU hMenu = CreatePopupMenu();
    AppendMenuW(hMenu, MF_STRING, 1001, L"Show History");
    AppendMenuW(hMenu, MF_STRING, 1003, L"Settings");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, 1002, L"Exit");

    SetForegroundWindow(hwnd);
    POINT pt; GetCursorPos(&pt);

    int cmd = TrackPopupMenu(hMenu,
        TPM_RETURNCMD|TPM_RIGHTBUTTON|TPM_BOTTOMALIGN,
        pt.x, pt.y, 0, hwnd, nullptr);
    DestroyMenu(hMenu);

    if      (cmd == 1001) SendMessageW(hwnd, WM_SHOW_POPUP, 0, 0);
    else if (cmd == 1002) PostQuitMessage(0);
    else if (cmd == 1003) SendMessageW(hwnd, WM_SHOW_SETTINGS, 0, 0);
}