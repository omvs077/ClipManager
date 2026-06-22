#include "wizard.h"

constexpr wchar_t Wizard::CLASS_NAME[];

#define ID_CHK_STARTUP  201
#define ID_CMB_LIMIT    202
#define ID_BTN_NEXT      203
#define ID_BTN_SKIP      204

static const COLORREF
    C_BG    = RGB(248,250,252),
    C_TEXT  = RGB(26, 37, 64),
    C_DIM   = RGB(110,125,150),
    C_ACCENT= RGB(61, 127,232);

static HBRUSH hBrBg = nullptr;

bool Wizard::Create(HINSTANCE hInst) {
    m_hInst = hInst;
    if (!hBrBg) hBrBg = CreateSolidBrush(C_BG);

    WNDCLASSEXW wc = {};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = hBrBg;
    wc.lpszClassName = CLASS_NAME;
    RegisterClassExW(&wc);

    m_hwnd = CreateWindowExW(
        WS_EX_TOPMOST,
        CLASS_NAME, L"Welcome to ClipManager",
        WS_POPUP | WS_CAPTION | WS_SYSMENU,
        0, 0, 460, 480,
        nullptr, nullptr, hInst, this);
    if (!m_hwnd) return false;

    m_hFont = CreateFontW(15,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,
        DEFAULT_PITCH|FF_DONTCARE, L"Segoe UI");
    m_hFontBig = CreateFontW(22,0,0,0,FW_SEMIBOLD,0,0,0,DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,
        DEFAULT_PITCH|FF_DONTCARE, L"Segoe UI");

    auto MakeLabel = [&](HWND parent, const wchar_t* text, int x, int y, int w, int h, HFONT f) {
        HWND hw = CreateWindowExW(0, L"STATIC", text,
            WS_CHILD|WS_VISIBLE|SS_LEFT, x, y, w, h, parent, nullptr, hInst, nullptr);
        SendMessageW(hw, WM_SETFONT, (WPARAM)f, TRUE);
        return hw;
    };

    // ── Page 0 ──────────────────────────────────────────────────
    m_pages[0] = CreateWindowExW(0, L"STATIC", L"",
        WS_CHILD|WS_VISIBLE, 0, 0, 460, 300, m_hwnd, nullptr, hInst, nullptr);
    MakeLabel(m_pages[0], L"Welcome to ClipManager", 32, 32, 396, 36, m_hFontBig);
    MakeLabel(m_pages[0],
        L"Press Ctrl+Shift+V anytime to see your clipboard history.\n"
        L"Let's set a couple of quick preferences.",
        32, 80, 396, 50, m_hFont);

    m_chkStartup = CreateWindowExW(0, L"BUTTON",
        L"Launch ClipManager when Windows starts",
        WS_CHILD|WS_VISIBLE|BS_AUTOCHECKBOX|WS_TABSTOP,
        32, 150, 396, 24, m_pages[0], (HMENU)ID_CHK_STARTUP, hInst, nullptr);
    SendMessageW(m_chkStartup, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    SendMessageW(m_chkStartup, BM_SETCHECK, BST_CHECKED, 0);

    // ── Page 1 ──────────────────────────────────────────────────
    m_pages[1] = CreateWindowExW(0, L"STATIC", L"",
        WS_CHILD, 0, 0, 460, 300, m_hwnd, nullptr, hInst, nullptr);
    MakeLabel(m_pages[1], L"How much history?", 32, 32, 396, 36, m_hFontBig);
    MakeLabel(m_pages[1],
        L"Choose how many clips to keep. You can change\n"
        L"this anytime in Settings.",
        32, 80, 396, 40, m_hFont);

    // Combo box must be WS_VISIBLE from creation and use WS_TABSTOP
    m_cmbLimit = CreateWindowExW(0, L"COMBOBOX", L"",
        WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST|WS_VSCROLL|WS_TABSTOP,
        32, 140, 220, 200, m_pages[1], (HMENU)ID_CMB_LIMIT, hInst, nullptr);
    SendMessageW(m_cmbLimit, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    SendMessageW(m_cmbLimit, CB_ADDSTRING, 0, (LPARAM)L"100 items");
    SendMessageW(m_cmbLimit, CB_ADDSTRING, 0, (LPARAM)L"500 items");
    SendMessageW(m_cmbLimit, CB_ADDSTRING, 0, (LPARAM)L"1000 items");
    SendMessageW(m_cmbLimit, CB_ADDSTRING, 0, (LPARAM)L"Unlimited");
    SendMessageW(m_cmbLimit, CB_SETCURSEL, 1, 0);

    // ── Page 2 ──────────────────────────────────────────────────
    m_pages[2] = CreateWindowExW(0, L"STATIC", L"",
        WS_CHILD, 0, 0, 460, 300, m_hwnd, nullptr, hInst, nullptr);
    MakeLabel(m_pages[2], L"You're all set!", 32, 32, 396, 36, m_hFontBig);
    MakeLabel(m_pages[2],
        L"Ctrl + Shift + V  \u2014  open clipboard history\n"
        L"Ctrl + P  \u2014  pin a clip\n"
        L"Ctrl + Del  \u2014  remove a clip\n\n"
        L"ClipManager is now running in your system tray.",
        32, 90, 396, 120, m_hFont);

    // ── Buttons ─────────────────────────────────────────────────
    m_btnSkip = CreateWindowExW(0, L"BUTTON", L"Skip",
        WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
        148, 360, 90, 32, m_hwnd, (HMENU)ID_BTN_SKIP, hInst, nullptr);
    SendMessageW(m_btnSkip, WM_SETFONT, (WPARAM)m_hFont, TRUE);

    m_btnNext = CreateWindowExW(0, L"BUTTON", L"Next",
        WS_CHILD|WS_VISIBLE|BS_DEFPUSHBUTTON,
        248, 360, 90, 32, m_hwnd, (HMENU)ID_BTN_NEXT, hInst, nullptr);
    SendMessageW(m_btnNext, WM_SETFONT, (WPARAM)m_hFont, TRUE);

    ShowPage(0);

    RECT rc; GetWindowRect(m_hwnd, &rc);
    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    SetWindowPos(m_hwnd, nullptr,
        (sw-(rc.right-rc.left))/2,
        (sh-(rc.bottom-rc.top))/2,
        0, 0, SWP_NOSIZE|SWP_NOZORDER);

    return true;
}

void Wizard::ShowPage(int page) {
    m_currentPage = page;
    for (int i = 0; i < 3; i++)
        ShowWindow(m_pages[i], i == page ? SW_SHOW : SW_HIDE);
    SetWindowTextW(m_btnNext, page == 2 ? L"Finish" : L"Next");
    ShowWindow(m_btnSkip, page == 2 ? SW_HIDE : SW_SHOW);
}

void Wizard::NextPage() {
    if (m_currentPage < 2) {
        ShowPage(m_currentPage + 1);
    } else {
        Finish();
    }
}

void Wizard::Finish() {
    bool startup = SendMessageW(m_chkStartup, BM_GETCHECK, 0, 0) == BST_CHECKED;
    int limitSel = (int)SendMessageW(m_cmbLimit, CB_GETCURSEL, 0, 0);
    int limits[] = {100, 500, 1000, -1};
    int limit = limits[limitSel < 4 && limitSel >= 0 ? limitSel : 1];

    MarkFirstRunDone();
    DestroyWindow(m_hwnd);

    if (OnComplete) OnComplete(startup, limit);
}

void Wizard::Show() {
    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
}

LRESULT CALLBACK Wizard::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Wizard* self = nullptr;
    if (msg == WM_NCCREATE) {
        CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<Wizard*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->m_hwnd = hwnd;
    } else {
        self = reinterpret_cast<Wizard*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    if (!self) return DefWindowProcW(hwnd, msg, wParam, lParam);

    switch (msg) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_BTN_NEXT:
            self->NextPage();
            return 0;
        case ID_BTN_SKIP:
            self->Finish();
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
        self->Finish();
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}