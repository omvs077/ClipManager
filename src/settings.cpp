#include "settings.h"
#include <shlobj.h>

constexpr wchar_t Settings::CLASS_NAME[];

#define IDC_SLIDER_LIMIT  201
#define IDC_LABEL_LIMIT   202
#define IDC_CHK_STARTUP   203
#define IDC_BTN_SAVE      204
#define IDC_BTN_CLEAR     205

// Colors — match popup dark theme
static const COLORREF S_BG     = RGB(22, 22, 24);
static const COLORREF S_TEXT   = RGB(220,220,225);
static const COLORREF S_DIM    = RGB(110,110,120);
static const COLORREF S_BORDER = RGB(48, 48, 56);
static const COLORREF S_ACCENT = RGB(50, 100,200);
static HBRUSH hBrBg = nullptr;

bool Settings::Create(HINSTANCE hInst) {
    m_hInst = hInst;
    if (!hBrBg) hBrBg = CreateSolidBrush(S_BG);

    WNDCLASSEXW wc  = {};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = hBrBg;
    wc.lpszClassName = CLASS_NAME;
    RegisterClassExW(&wc);

    m_hwnd = CreateWindowExW(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        CLASS_NAME, L"ClipManager Settings",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        0, 0, 400, 340,
        nullptr, nullptr, hInst, this);
    if (!m_hwnd) return false;

    HFONT hFont = CreateFontW(15,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,
        DEFAULT_PITCH|FF_DONTCARE, L"Segoe UI");

    auto MakeLabel = [&](const wchar_t* text, int x, int y, int w, int h) -> HWND {
        HWND hw = CreateWindowExW(0, L"STATIC", text,
            WS_CHILD|WS_VISIBLE, x, y, w, h, m_hwnd, nullptr, hInst, nullptr);
        SendMessageW(hw, WM_SETFONT, (WPARAM)hFont, TRUE);
        return hw;
    };

    auto MakeButton = [&](const wchar_t* text, int id, int x, int y, int w, int h) -> HWND {
        HWND hw = CreateWindowExW(0, L"BUTTON", text,
            WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, x, y, w, h,
            m_hwnd, (HMENU)(UINT_PTR)id, hInst, nullptr);
        SendMessageW(hw, WM_SETFONT, (WPARAM)hFont, TRUE);
        return hw;
    };

    // ── Section: History ─────────────────────────────────────────
    MakeLabel(L"HISTORY", 24, 20, 340, 18);
    MakeLabel(L"Maximum items to keep:", 24, 48, 200, 20);

    m_labelLimit = MakeLabel(L"50", 310, 48, 40, 20);

    m_sliderLimit = CreateWindowExW(0, TRACKBAR_CLASSW, L"",
        WS_CHILD|WS_VISIBLE|TBS_HORZ|TBS_NOTICKS,
        24, 72, 340, 28,
        m_hwnd, (HMENU)IDC_SLIDER_LIMIT, hInst, nullptr);
    SendMessageW(m_sliderLimit, TBM_SETRANGE, TRUE, MAKELONG(10, 200));
    SendMessageW(m_sliderLimit, TBM_SETPOS,   TRUE, Current.historyLimit);
    SendMessageW(m_sliderLimit, WM_SETFONT, (WPARAM)hFont, TRUE);

    MakeLabel(L"10", 24, 102, 30, 16);
    MakeLabel(L"200", 350, 102, 40, 16);

    // Separator
    MakeLabel(L"", 24, 126, 340, 1);

    // ── Section: Startup ─────────────────────────────────────────
    MakeLabel(L"STARTUP", 24, 140, 340, 18);

    m_chkStartup = CreateWindowExW(0, L"BUTTON",
        L"Launch ClipManager when Windows starts",
        WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
        24, 164, 340, 24,
        m_hwnd, (HMENU)IDC_CHK_STARTUP, hInst, nullptr);
    SendMessageW(m_chkStartup, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessageW(m_chkStartup, BM_SETCHECK,
        Current.startWithWindows ? BST_CHECKED : BST_UNCHECKED, 0);

    // ── Section: Data ────────────────────────────────────────────
    MakeLabel(L"DATA", 24, 206, 340, 18);

    m_btnClear = MakeButton(L"Clear All History", IDC_BTN_CLEAR, 24, 230, 160, 32);

    // ── Bottom buttons ───────────────────────────────────────────
    MakeButton(L"Cancel", IDCANCEL, 188, 278, 88, 32);
    m_btnSave = MakeButton(L"Save", IDC_BTN_SAVE, 288, 278, 88, 32);

    // Center on screen
    RECT rc; GetWindowRect(m_hwnd, &rc);
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    SetWindowPos(m_hwnd, nullptr,
        (sw-(rc.right-rc.left))/2,
        (sh-(rc.bottom-rc.top))/2,
        0, 0, SWP_NOSIZE|SWP_NOZORDER);

    return true;
}

void Settings::Show() {
    PopulateControls();
    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
}

void Settings::Hide() {
    ShowWindow(m_hwnd, SW_HIDE);
}

bool Settings::IsVisible() const {
    return IsWindowVisible(m_hwnd) != FALSE;
}

void Settings::PopulateControls() {
    SendMessageW(m_sliderLimit, TBM_SETPOS, TRUE, Current.historyLimit);
    wchar_t buf[16];
    swprintf_s(buf, L"%d", Current.historyLimit);
    SetWindowTextW(m_labelLimit, buf);
    SendMessageW(m_chkStartup, BM_SETCHECK,
        Current.startWithWindows ? BST_CHECKED : BST_UNCHECKED, 0);
}

void Settings::SaveAndClose() {
    Current.historyLimit = (int)SendMessageW(m_sliderLimit, TBM_GETPOS, 0, 0);
    Current.startWithWindows =
        SendMessageW(m_chkStartup, BM_GETCHECK, 0, 0) == BST_CHECKED;
    ApplyStartup(Current.startWithWindows);
    if (OnSave) OnSave(Current);
    Hide();
}

void Settings::ApplyStartup(bool enable) {
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);

    HKEY hKey;
    RegOpenKeyExW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_SET_VALUE, &hKey);

    if (enable) {
        std::wstring val = L"\"" + std::wstring(exePath) + L"\"";
        RegSetValueExW(hKey, L"ClipManager", 0, REG_SZ,
            (const BYTE*)val.c_str(),
            (DWORD)((val.size()+1)*sizeof(wchar_t)));
    } else {
        RegDeleteValueW(hKey, L"ClipManager");
    }
    RegCloseKey(hKey);
}

LRESULT CALLBACK Settings::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Settings* self = nullptr;
    if (msg == WM_NCCREATE) {
        CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<Settings*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->m_hwnd = hwnd;
    } else {
        self = reinterpret_cast<Settings*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    if (!self) return DefWindowProcW(hwnd, msg, wParam, lParam);

    switch (msg) {

    case WM_HSCROLL: {
        if ((HWND)lParam == self->m_sliderLimit) {
            int pos = (int)SendMessageW(self->m_sliderLimit, TBM_GETPOS, 0, 0);
            wchar_t buf[16];
            swprintf_s(buf, L"%d", pos);
            SetWindowTextW(self->m_labelLimit, buf);
        }
        return 0;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BTN_SAVE:
            self->SaveAndClose();
            return 0;
        case IDCANCEL:
            self->Hide();
            return 0;
        case IDC_BTN_CLEAR:
            if (MessageBoxW(hwnd,
                L"Clear all clipboard history?",
                L"ClipManager",
                MB_YESNO | MB_ICONQUESTION) == IDYES) {
                if (self->OnSave) {
                    AppSettings s = self->Current;
                    s.historyLimit = 0; // signal to clear
                    self->OnSave(s);
                }
            }
            return 0;
        }
        return 0;

    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN: {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, S_TEXT);
        SetBkColor(hdc, S_BG);
        return (LRESULT)hBrBg;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc; GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, hBrBg);

        // Section headers
        HFONT hSmall = CreateFontW(11,0,0,0,FW_BOLD,0,0,0,DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,
            DEFAULT_PITCH|FF_DONTCARE,L"Segoe UI");
        SelectObject(hdc, hSmall);
        SetBkMode(hdc, TRANSPARENT);

        auto DrawSection = [&](const wchar_t* text, int y) {
            SetTextColor(hdc, S_DIM);
            TextOutW(hdc, 24, y, text, (int)wcslen(text));
            HPEN p = CreatePen(PS_SOLID,1,S_BORDER);
            HPEN o = (HPEN)SelectObject(hdc,p);
            MoveToEx(hdc, 24, y+16, nullptr);
            LineTo(hdc, 376, y+16);
            SelectObject(hdc,o); DeleteObject(p);
        };

        DrawSection(L"HISTORY", 20);
        DrawSection(L"STARTUP", 140);
        DrawSection(L"DATA",    206);

        DeleteObject(hSmall);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_CLOSE:
        self->Hide();
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}