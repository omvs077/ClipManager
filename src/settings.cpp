#include "settings.h"
#include <shlobj.h>
#include <uxtheme.h>
#pragma comment(lib, "uxtheme.lib")

constexpr wchar_t Settings::CLASS_NAME[];
constexpr wchar_t Settings::PANEL_CLASS[];

// Control IDs
#define ID_TAB           100
#define ID_CHK_STARTUP   101
#define ID_CHK_MINTRAY   102
#define ID_CHK_NOTIFY    103
#define ID_CMB_LIMIT     104
#define ID_CHK_DUPES     105
#define ID_CHK_IMAGES    106
#define ID_CHK_FILES     107
#define ID_CMB_AUTODEL   108
#define ID_BTN_CLEAR     109
#define ID_CMB_THEME     110
#define ID_CHK_COMPACT   111
#define ID_CHK_TIMESTAMPS 112
#define ID_CHK_PAUSE     113
#define ID_CHK_EXCLPWD   114
#define ID_CHK_CLEAREXIT 115
#define ID_HK_MAIN       116
#define ID_HK_LATEST     117
#define ID_BTN_SAVE      118
#define ID_BTN_CANCEL    119

// ── Palette ──────────────────────────────────────────────────────
static const COLORREF
    C_BG      = RGB(248,250,252),
    C_PANEL   = RGB(252,253,255),
    C_TEXT    = RGB(26, 37, 64),
    C_DIM     = RGB(110,125,150),
    C_BORDER  = RGB(220,228,240),
    C_ACCENT  = RGB(61, 127,232),
    C_BTN     = RGB(236,242,250);

static HBRUSH hBrBg    = nullptr;
static HBRUSH hBrPanel = nullptr;
static HBRUSH hBrBtn   = nullptr;

static void InitBrushes() {
    if (!hBrBg) {
        hBrBg    = CreateSolidBrush(C_BG);
        hBrPanel = CreateSolidBrush(C_PANEL);
        hBrBtn   = CreateSolidBrush(C_BTN);
    }
}

// ── Helpers ──────────────────────────────────────────────────────
HWND Settings::MakeLabel(HWND parent, const wchar_t* text,
    int x, int y, int w, int h) {
    HWND hw = CreateWindowExW(0, L"STATIC", text,
        WS_CHILD|WS_VISIBLE|SS_LEFT,
        x, y, w, h, parent, nullptr, m_hInst, nullptr);
    SendMessageW(hw, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    return hw;
}

static HWND MakeSectionHeader(HWND parent, HINSTANCE hInst, HFONT font,
    const wchar_t* text, int x, int y, int w) {
    HWND hw = CreateWindowExW(0, L"STATIC", text,
        WS_CHILD|WS_VISIBLE|SS_LEFT,
        x, y, w, 18, parent, nullptr, hInst, nullptr);
    SendMessageW(hw, WM_SETFONT, (WPARAM)font, TRUE);
    return hw;
}

HWND Settings::MakeCheck(HWND parent, const wchar_t* text, int id,
    int x, int y, int w, int h) {
    HWND hw = CreateWindowExW(0, L"BUTTON", text,
        WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX,
        x, y, w, h, parent, (HMENU)(UINT_PTR)id, m_hInst, nullptr);
    SendMessageW(hw, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    return hw;
}

HWND Settings::MakeCombo(HWND parent, int id,
    int x, int y, int w, int h) {
    HWND hw = CreateWindowExW(0, L"COMBOBOX", L"",
        WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST|WS_VSCROLL,
        x, y, w, h, parent, (HMENU)(UINT_PTR)id, m_hInst, nullptr);
    SendMessageW(hw, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    return hw;
}

HWND Settings::MakeButton(HWND parent, const wchar_t* text, int id,
    int x, int y, int w, int h) {
    HWND hw = CreateWindowExW(0, L"BUTTON", text,
        WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
        x, y, w, h, parent, (HMENU)(UINT_PTR)id, m_hInst, nullptr);
    SendMessageW(hw, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    return hw;
}

HWND Settings::MakeHotkeyBox(HWND parent, int id,
    int x, int y, int w, int h) {
    HWND hw = CreateWindowExW(WS_EX_CLIENTEDGE, HOTKEY_CLASS, L"",
        WS_CHILD|WS_VISIBLE,
        x, y, w, h, parent, (HMENU)(UINT_PTR)id, m_hInst, nullptr);
    SendMessageW(hw, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    return hw;
}

// ── Panel WndProc (dark background for tab panels) ───────────────
LRESULT CALLBACK Settings::PanelProc(HWND hwnd, UINT msg,
    WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN: {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, C_TEXT);
        SetBkColor(hdc, C_PANEL);
        return (LRESULT)hBrPanel;
    }
    case WM_CTLCOLORLISTBOX:
    case WM_CTLCOLOREDIT: {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, C_TEXT);
        SetBkColor(hdc, C_PANEL);
        return (LRESULT)hBrPanel;
    }
    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wParam;
        RECT rc; GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, hBrPanel);
        return 1;
    }
    case WM_COMMAND:
        SendMessageW(GetParent(hwnd), msg, wParam, lParam);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

// ── Tab panels (using the pixel blueprint grid) ───────────────────
HWND Settings::CreateTabGeneral(RECT rc) {
    HWND p = CreateWindowExW(0, PANEL_CLASS, L"",
        WS_CHILD, rc.left, rc.top,
        rc.right-rc.left, rc.bottom-rc.top,
        m_hwnd, nullptr, m_hInst, nullptr);

    MakeSectionHeader(p, m_hInst, m_hFontBold, L"Startup", 24, 16, 400);
    m_chkStartup = MakeCheck(p, L"Launch ClipManager when Windows starts",
        ID_CHK_STARTUP, 32, 42, 400, 24);
    m_chkMinTray = MakeCheck(p, L"Minimize to tray when closed",
        ID_CHK_MINTRAY, 32, 72, 400, 24);

    MakeSectionHeader(p, m_hInst, m_hFontBold, L"Notifications", 24, 116, 400);
    m_chkNotify = MakeCheck(p, L"Show notification when clip is saved",
        ID_CHK_NOTIFY, 32, 142, 400, 24);

    return p;
}

HWND Settings::CreateTabHistory(RECT rc) {
    HWND p = CreateWindowExW(0, PANEL_CLASS, L"",
        WS_CHILD, rc.left, rc.top,
        rc.right-rc.left, rc.bottom-rc.top,
        m_hwnd, nullptr, m_hInst, nullptr);

    MakeSectionHeader(p, m_hInst, m_hFontBold, L"Storage Limits", 24, 16, 400);

    MakeLabel(p, L"Maximum history items:", 32, 50, 220, 20);
    m_cmbLimit = MakeCombo(p, ID_CMB_LIMIT, 270, 47, 160, 24);
    SendMessageW(m_cmbLimit, CB_ADDSTRING, 0, (LPARAM)L"100");
    SendMessageW(m_cmbLimit, CB_ADDSTRING, 0, (LPARAM)L"500");
    SendMessageW(m_cmbLimit, CB_ADDSTRING, 0, (LPARAM)L"1000");
    SendMessageW(m_cmbLimit, CB_ADDSTRING, 0, (LPARAM)L"Unlimited");
    SendMessageW(m_cmbLimit, CB_SETCURSEL, 1, 0);

    MakeLabel(p, L"Auto-delete older than:", 32, 84, 220, 20);
    m_cmbAutoDel = MakeCombo(p, ID_CMB_AUTODEL, 270, 81, 160, 24);
    SendMessageW(m_cmbAutoDel, CB_ADDSTRING, 0, (LPARAM)L"Never");
    SendMessageW(m_cmbAutoDel, CB_ADDSTRING, 0, (LPARAM)L"7 days");
    SendMessageW(m_cmbAutoDel, CB_ADDSTRING, 0, (LPARAM)L"30 days");
    SendMessageW(m_cmbAutoDel, CB_ADDSTRING, 0, (LPARAM)L"90 days");
    SendMessageW(m_cmbAutoDel, CB_SETCURSEL, 2, 0);

    MakeSectionHeader(p, m_hInst, m_hFontBold, L"Content Filters", 24, 130, 400);

    m_chkDupes  = MakeCheck(p, L"Ignore duplicate entries",
        ID_CHK_DUPES,  32, 156, 400, 24);
    m_chkImages = MakeCheck(p, L"Capture copied images",
        ID_CHK_IMAGES, 32, 186, 400, 24);
    m_chkFiles  = MakeCheck(p, L"Capture copied files and folders",
        ID_CHK_FILES,  32, 216, 400, 24);

    MakeSectionHeader(p, m_hInst, m_hFontBold, L"Maintenance", 24, 260, 400);
    m_btnClear = MakeButton(p, L"Clear All History",
        ID_BTN_CLEAR, 32, 286, 160, 32);

    return p;
}

HWND Settings::CreateTabHotkeys(RECT rc) {
    HWND p = CreateWindowExW(0, PANEL_CLASS, L"",
        WS_CHILD, rc.left, rc.top,
        rc.right-rc.left, rc.bottom-rc.top,
        m_hwnd, nullptr, m_hInst, nullptr);

    MakeSectionHeader(p, m_hInst, m_hFontBold, L"Global Hotkeys", 24, 16, 400);

    MakeLabel(p, L"Open Clipboard History:", 32, 50, 220, 20);
    m_hkMain = MakeHotkeyBox(p, ID_HK_MAIN, 270, 47, 160, 24);
    SendMessageW(m_hkMain, HKM_SETHOTKEY, MAKEWORD('V', HOTKEYF_CONTROL | HOTKEYF_SHIFT), 0);

    MakeLabel(p, L"Paste Latest Clip:", 32, 84, 220, 20);
    m_hkLatest = MakeHotkeyBox(p, ID_HK_LATEST, 270, 81, 160, 24);

    MakeLabel(p,
        L"Note: Ctrl+Shift+V is reserved for the main hotkey\n"
        L"and cannot be changed in this version.",
        32, 130, 400, 40);

    return p;
}

HWND Settings::CreateTabAppearance(RECT rc) {
    HWND p = CreateWindowExW(0, PANEL_CLASS, L"",
        WS_CHILD, rc.left, rc.top,
        rc.right-rc.left, rc.bottom-rc.top,
        m_hwnd, nullptr, m_hInst, nullptr);

    MakeSectionHeader(p, m_hInst, m_hFontBold, L"Display", 24, 16, 400);
    m_chkCompact    = MakeCheck(p, L"Compact mode (smaller items)",
        ID_CHK_COMPACT,    32, 42, 400, 24);
    m_chkTimestamps = MakeCheck(p, L"Show timestamps on clips",
        ID_CHK_TIMESTAMPS, 32, 72, 400, 24);

    return p;
}

HWND Settings::CreateTabPrivacy(RECT rc) {
    HWND p = CreateWindowExW(0, PANEL_CLASS, L"",
        WS_CHILD, rc.left, rc.top,
        rc.right-rc.left, rc.bottom-rc.top,
        m_hwnd, nullptr, m_hInst, nullptr);

    MakeSectionHeader(p, m_hInst, m_hFontBold, L"Monitoring", 24, 16, 400);
    m_chkPause   = MakeCheck(p, L"Pause clipboard monitoring",
        ID_CHK_PAUSE,   32, 42, 400, 24);
    m_chkExclPwd = MakeCheck(p, L"Exclude copies from password managers",
        ID_CHK_EXCLPWD, 32, 72, 400, 24);

    MakeSectionHeader(p, m_hInst, m_hFontBold, L"Data", 24, 116, 400);
    m_chkClearExit = MakeCheck(p, L"Clear history when ClipManager exits",
        ID_CHK_CLEAREXIT, 32, 142, 400, 24);

    return p;
}

// ── Create ───────────────────────────────────────────────────────
bool Settings::Create(HINSTANCE hInst) {
    m_hInst = hInst;
    InitBrushes();

    WNDCLASSEXW pc = {};
    pc.cbSize        = sizeof(pc);
    pc.lpfnWndProc   = PanelProc;
    pc.hInstance     = hInst;
    pc.hbrBackground = hBrPanel;
    pc.lpszClassName = PANEL_CLASS;
    RegisterClassExW(&pc);

    m_hFont = CreateFontW(15,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,
        DEFAULT_PITCH|FF_DONTCARE, L"Segoe UI");
    m_hFontBold = CreateFontW(16,0,0,0,FW_SEMIBOLD,0,0,0,DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,
        DEFAULT_PITCH|FF_DONTCARE, L"Segoe UI");
    m_hFontSm = CreateFontW(13,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,
        DEFAULT_PITCH|FF_DONTCARE, L"Segoe UI");

    WNDCLASSEXW wc = {};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = hBrBg;
    wc.lpszClassName = CLASS_NAME;
    RegisterClassExW(&wc);

    // ── Main window: 520 x 480 per blueprint ──────────────────────
    m_hwnd = CreateWindowExW(
        WS_EX_DLGMODALFRAME,
        CLASS_NAME, L"ClipManager Settings",
        WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU,
        0, 0, 520, 480,
        nullptr, nullptr, hInst, this);
    if (!m_hwnd) return false;

    INITCOMMONCONTROLSEX icc = {sizeof(icc), ICC_TAB_CLASSES|ICC_HOTKEY_CLASS};
    InitCommonControlsEx(&icc);

    // ── Tab control: 496 x 390 at (12,12) ──────────────────────────
    m_tabs = CreateWindowExW(0, WC_TABCONTROLW, L"",
        WS_CHILD|WS_VISIBLE|TCS_FLATBUTTONS,
        12, 12, 496, 390,
        m_hwnd, (HMENU)ID_TAB, hInst, nullptr);
    SendMessageW(m_tabs, WM_SETFONT, (WPARAM)m_hFont, TRUE);

    auto AddTab = [&](const wchar_t* text) {
        static int idx = 0;
        TCITEMW ti = {}; ti.mask = TCIF_TEXT;
        ti.pszText = const_cast<wchar_t*>(text);
        SendMessageW(m_tabs, TCM_INSERTITEM, idx++, (LPARAM)&ti);
    };
    AddTab(L"General");
    AddTab(L"History");
    AddTab(L"Hotkeys");
    AddTab(L"Appearance");
    AddTab(L"Privacy");

    // Per checklist: panel sits
    RECT tabRc = {16, 44, 16+488, 44+320};

    m_panels[0] = CreateTabGeneral(tabRc);
    m_panels[1] = CreateTabHistory(tabRc);
    m_panels[2] = CreateTabHotkeys(tabRc);
    m_panels[3] = CreateTabAppearance(tabRc);
    m_panels[4] = CreateTabPrivacy(tabRc);

    // ── Action buttons: direct children of main window, in the runway ──
    MakeButton(m_hwnd, L"Save",   ID_BTN_SAVE,   286, 380, 90, 32);
    MakeButton(m_hwnd, L"Cancel", ID_BTN_CANCEL, 388, 380, 90, 32);

    ShowTab(0);
    PopulateControls();

    RECT rc; GetWindowRect(m_hwnd, &rc);
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    SetWindowPos(m_hwnd, nullptr,
        (sw-(rc.right-rc.left))/2,
        (sh-(rc.bottom-rc.top))/2,
        0, 0, SWP_NOSIZE|SWP_NOZORDER);

    return true;
}

void Settings::ShowTab(int index) {
    for (int i = 0; i < 5; i++)
        ShowWindow(m_panels[i], i == index ? SW_SHOW : SW_HIDE);
    m_activeTab = index;
}

void Settings::PopulateControls() {
    SendMessageW(m_chkStartup, BM_SETCHECK,
        Current.startWithWindows ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(m_chkMinTray, BM_SETCHECK,
        Current.minimizeToTray ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(m_chkNotify, BM_SETCHECK,
        Current.showNotifications ? BST_CHECKED : BST_UNCHECKED, 0);

    int limitIdx = 1;
    if      (Current.historyLimit == 100)  limitIdx = 0;
    else if (Current.historyLimit == 500)  limitIdx = 1;
    else if (Current.historyLimit == 1000) limitIdx = 2;
    else if (Current.historyLimit == -1)   limitIdx = 3;
    SendMessageW(m_cmbLimit, CB_SETCURSEL, limitIdx, 0);

    int delIdx = 2;
    if      (Current.autoDeleteDays == -1) delIdx = 0;
    else if (Current.autoDeleteDays == 7)  delIdx = 1;
    else if (Current.autoDeleteDays == 30) delIdx = 2;
    else if (Current.autoDeleteDays == 90) delIdx = 3;
    SendMessageW(m_cmbAutoDel, CB_SETCURSEL, delIdx, 0);

    SendMessageW(m_chkDupes,  BM_SETCHECK,
        Current.ignoreDuplicates ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(m_chkImages, BM_SETCHECK,
        Current.saveImages ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(m_chkFiles,  BM_SETCHECK,
        Current.saveFiles ? BST_CHECKED : BST_UNCHECKED, 0);

    SendMessageW(m_chkCompact, BM_SETCHECK,
        Current.compactMode ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(m_chkTimestamps, BM_SETCHECK,
        Current.showTimestamps ? BST_CHECKED : BST_UNCHECKED, 0);

    SendMessageW(m_chkPause,     BM_SETCHECK,
        Current.pauseMonitoring ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(m_chkExclPwd,   BM_SETCHECK,
        Current.excludePasswords ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(m_chkClearExit, BM_SETCHECK,
        Current.clearOnExit ? BST_CHECKED : BST_UNCHECKED, 0);
}

void Settings::SaveAndClose() {
    Current.startWithWindows  =
        SendMessageW(m_chkStartup, BM_GETCHECK, 0, 0) == BST_CHECKED;
    Current.minimizeToTray    =
        SendMessageW(m_chkMinTray, BM_GETCHECK, 0, 0) == BST_CHECKED;
    Current.showNotifications =
        SendMessageW(m_chkNotify,  BM_GETCHECK, 0, 0) == BST_CHECKED;

    int limitSel = (int)SendMessageW(m_cmbLimit, CB_GETCURSEL, 0, 0);
    int limits[] = {100, 500, 1000, -1};
    Current.historyLimit = limits[limitSel < 4 ? limitSel : 1];

    int delSel = (int)SendMessageW(m_cmbAutoDel, CB_GETCURSEL, 0, 0);
    int days[] = {-1, 7, 30, 90};
    Current.autoDeleteDays = days[delSel < 4 ? delSel : 2];

    Current.ignoreDuplicates =
        SendMessageW(m_chkDupes,  BM_GETCHECK, 0, 0) == BST_CHECKED;
    Current.saveImages =
        SendMessageW(m_chkImages, BM_GETCHECK, 0, 0) == BST_CHECKED;
    Current.saveFiles =
        SendMessageW(m_chkFiles,  BM_GETCHECK, 0, 0) == BST_CHECKED;

    Current.compactMode =
        SendMessageW(m_chkCompact,    BM_GETCHECK, 0, 0) == BST_CHECKED;
    Current.showTimestamps =
        SendMessageW(m_chkTimestamps, BM_GETCHECK, 0, 0) == BST_CHECKED;

    Current.pauseMonitoring =
        SendMessageW(m_chkPause,     BM_GETCHECK, 0, 0) == BST_CHECKED;
    Current.excludePasswords =
        SendMessageW(m_chkExclPwd,   BM_GETCHECK, 0, 0) == BST_CHECKED;
    Current.clearOnExit =
        SendMessageW(m_chkClearExit, BM_GETCHECK, 0, 0) == BST_CHECKED;

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

void Settings::Show() {
    PopulateControls();
    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
}

void Settings::Hide() { ShowWindow(m_hwnd, SW_HIDE); }

bool Settings::IsVisible() const {
    return IsWindowVisible(m_hwnd) != FALSE;
}

LRESULT CALLBACK Settings::WndProc(HWND hwnd, UINT msg,
    WPARAM wParam, LPARAM lParam) {
    Settings* self = nullptr;
    if (msg == WM_NCCREATE) {
        CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<Settings*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->m_hwnd = hwnd;
    } else {
        self = reinterpret_cast<Settings*>(
            GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    if (!self) return DefWindowProcW(hwnd, msg, wParam, lParam);

    switch (msg) {

    case WM_NOTIFY: {
        NMHDR* nm = reinterpret_cast<NMHDR*>(lParam);
        if (nm->idFrom == ID_TAB && nm->code == TCN_SELCHANGE) {
            int sel = (int)SendMessageW(self->m_tabs, TCM_GETCURSEL, 0, 0);
            self->ShowTab(sel);
        }
        return 0;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_BTN_SAVE:
            self->SaveAndClose();
            return 0;
        case ID_BTN_CANCEL:
            self->Hide();
            return 0;
        case ID_BTN_CLEAR:
            if (MessageBoxW(hwnd,
                L"Clear all clipboard history? This cannot be undone.",
                L"ClipManager", MB_YESNO|MB_ICONQUESTION) == IDYES) {
                if (self->OnClearHistory) self->OnClearHistory();
            }
            return 0;
        case ID_CHK_PAUSE:
            if (self->OnPauseToggle) {
                bool paused =
                    SendMessageW(self->m_chkPause, BM_GETCHECK, 0, 0)
                    == BST_CHECKED;
                self->OnPauseToggle(paused);
            }
            return 0;
        }
        return 0;

    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN: {
        HDC hdc = (HDC)wParam;
        SetTextColor(hdc, C_TEXT);
        SetBkColor(hdc, C_BG);
        return (LRESULT)hBrBg;
    }

    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wParam;
        RECT rc; GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, hBrBg);
        return 1;
    }

    case WM_CLOSE:
        self->Hide();
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}