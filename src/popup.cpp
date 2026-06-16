#include "popup.h"
#include "detector.h"

constexpr wchar_t Popup::CLASS_NAME[];

bool Popup::Create(HINSTANCE hInst) {
    m_hInst = hInst;

    WNDCLASSEXW wc  = {};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = CLASS_NAME;
    RegisterClassExW(&wc);

    m_hwnd = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        CLASS_NAME, L"",
        WS_POPUP | WS_BORDER,
        0, 0, 480, 400,
        nullptr, nullptr, hInst, this);
    if (!m_hwnd) return false;

    m_search = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        8, 8, 464, 24,
        m_hwnd, (HMENU)101, hInst, nullptr);

    m_list = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"ListBox", L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
        8, 40, 464, 338,
        m_hwnd, (HMENU)102, hInst, nullptr);

    CreateWindowExW(0, L"STATIC",
        L"Enter=Paste   Ctrl+P=Pin   Del=Remove   Esc=Close",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        0, 382, 480, 16,
        m_hwnd, nullptr, hInst, nullptr);

    SetWindowSubclass(m_search, [](HWND hwnd, UINT msg, WPARAM wParam,
        LPARAM lParam, UINT_PTR, DWORD_PTR data) -> LRESULT {
            Popup* self = reinterpret_cast<Popup*>(data);
            if (msg == WM_KEYDOWN) {
                if (wParam == VK_RETURN) { self->ConfirmSelection(); return 0; }
                if (wParam == VK_ESCAPE) { self->Hide(); return 0; }
                if (wParam == VK_DOWN || wParam == VK_UP) {
                    SendMessageW(self->m_list, msg, wParam, lParam);
                    return 0;
                }
                if (wParam == 'P' && (GetKeyState(VK_CONTROL) & 0x8000)) {
                    self->TogglePin(); return 0;
                }
                if (wParam == VK_DELETE && (GetKeyState(VK_CONTROL) & 0x8000)) {
                    self->DeleteSelected(); return 0;
                }
            }
            if (msg == WM_CHAR) {
                LRESULT r = DefSubclassProc(hwnd, msg, wParam, lParam);
                wchar_t buf[256] = {};
                GetWindowTextW(hwnd, buf, 256);
                self->PopulateList(buf);
                return r;
            }
            return DefSubclassProc(hwnd, msg, wParam, lParam);
        }, 1, reinterpret_cast<DWORD_PTR>(this));

    return true;
}

std::wstring Popup::BuildDisplayText(const ClipEntry& entry) {
    std::wstring pin     = entry.pinned ? L"\u2605 " : L"";
    std::wstring label   = Detector::TypeLabel(entry.type);
    std::wstring preview = entry.text.substr(0, 80);
    for (wchar_t& c : preview)
        if (c == L'\n' || c == L'\r') c = L' ';
    return pin + label + preview;
}

void Popup::Show(const std::vector<ClipEntry>& history) {
    m_history = history;
    SetWindowTextW(m_search, L"");
    PopulateList();
    PositionNearCursor();
    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
    SetFocus(m_search);
}

void Popup::Hide() {
    ShowWindow(m_hwnd, SW_HIDE);
}

bool Popup::IsVisible() const {
    return IsWindowVisible(m_hwnd) != FALSE;
}

void Popup::PositionNearCursor() {
    POINT pt; GetCursorPos(&pt);
    RECT rc;  GetWindowRect(m_hwnd, &rc);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;

    MONITORINFO mi = {}; mi.cbSize = sizeof(mi);
    GetMonitorInfoW(MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST), &mi);
    RECT work = mi.rcWork;

    int x = pt.x;
    int y = pt.y - h - 4;
    if (x + w > work.right) x = work.right - w;
    if (x < work.left)       x = work.left;
    if (y < work.top)        y = pt.y + 20;

    SetWindowPos(m_hwnd, HWND_TOPMOST, x, y, 0, 0, SWP_NOSIZE);
}

void Popup::PopulateList(const std::wstring& filter) {
    SendMessageW(m_list, LB_RESETCONTENT, 0, 0);
    m_filtered.clear();

    for (int pass = 0; pass < 2; pass++) {
        for (int i = 0; i < (int)m_history.size(); i++) {
            const ClipEntry& entry = m_history[i];
            if (pass == 0 && !entry.pinned) continue;
            if (pass == 1 &&  entry.pinned) continue;

            bool match = filter.empty();
            if (!match) {
                std::wstring lo = entry.text;
                std::wstring lf = filter;
                std::transform(lo.begin(), lo.end(), lo.begin(), ::towlower);
                std::transform(lf.begin(), lf.end(), lf.begin(), ::towlower);
                match = lo.find(lf) != std::wstring::npos;
            }
            if (match) {
                std::wstring display = BuildDisplayText(entry);
                SendMessageW(m_list, LB_ADDSTRING, 0, (LPARAM)display.c_str());
                m_filtered.push_back(i);
            }
        }
    }
    if (!m_filtered.empty())
        SendMessageW(m_list, LB_SETCURSEL, 0, 0);
}

void Popup::ConfirmSelection() {
    int sel = (int)SendMessageW(m_list, LB_GETCURSEL, 0, 0);
    if (sel == LB_ERR || sel >= (int)m_filtered.size()) return;
    int idx = m_filtered[sel];
    Hide();
    if (OnSelect) OnSelect(idx);
}

void Popup::TogglePin() {
    int sel = (int)SendMessageW(m_list, LB_GETCURSEL, 0, 0);
    if (sel == LB_ERR || sel >= (int)m_filtered.size()) return;
    int idx = m_filtered[sel];
    m_history[idx].pinned = !m_history[idx].pinned;
    wchar_t buf[256] = {};
    GetWindowTextW(m_search, buf, 256);
    PopulateList(buf);
    if (OnPin) OnPin(idx);
}

void Popup::DeleteSelected() {
    int sel = (int)SendMessageW(m_list, LB_GETCURSEL, 0, 0);
    if (sel == LB_ERR || sel >= (int)m_filtered.size()) return;
    int idx = m_filtered[sel];
    if (OnDelete) OnDelete(idx);
    m_history.erase(m_history.begin() + idx);
    wchar_t buf[256] = {};
    GetWindowTextW(m_search, buf, 256);
    PopulateList(buf);
}

LRESULT CALLBACK Popup::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Popup* self = nullptr;
    if (msg == WM_NCCREATE) {
        CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<Popup*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->m_hwnd = hwnd;
    } else {
        self = reinterpret_cast<Popup*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    if (!self) return DefWindowProcW(hwnd, msg, wParam, lParam);

    switch (msg) {
    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE) self->Hide();
        return 0;
    case WM_COMMAND:
        if (HIWORD(wParam) == LBN_DBLCLK) self->ConfirmSelection();
        return 0;
    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) self->Hide();
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}