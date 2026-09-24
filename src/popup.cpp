#include "popup.h"
#include "detector.h"
#include "imaging.h"
#include <cwctype>
#include <string>
#include <algorithm>
#include <shellapi.h>
#include "snippets.h"

constexpr wchar_t Popup::CLASS_NAME[];

// Ã¢â€â‚¬Ã¢â€â‚¬ Palette Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬
#define C(r,g,b) RGB(r,g,b)
static const COLORREF
BG_DARK = C(248, 250, 252),
BG_LEFT = C(252, 253, 255),
BG_RIGHT = C(255, 255, 255),
BG_ITEM = C(252, 253, 255),
BG_ITEM_HOV = C(232, 240, 250),
BG_ITEM_SEL = C(61, 127, 232),
BG_SEARCH = C(236, 242, 250),
BG_HINT = C(242, 246, 251),
CLR_TEXT = C(26, 37, 64),
CLR_DIM = C(110, 125, 150),
CLR_BORDER = C(220, 228, 240),
CLR_SEP = C(232, 238, 248),
CLR_PIN = C(230, 160, 30),
CLR_URL = C(40, 110, 220),
CLR_COLOR_T = C(20, 150, 130),
CLR_PATH = C(180, 120, 30),
CLR_EMAIL = C(120, 70, 180),
CLR_IMAGE = C(91, 163, 240),
CLR_SNIPPET = C(142, 68, 173),
CLR_LABEL_BG = C(232, 240, 250),
CLR_WHITE = C(255, 255, 255),
CLR_HINT_KEY = C(80, 95, 120);

static HBRUSH hBrDark, hBrLeft, hBrRight, hBrSearch,
hBrSel, hBrItem, hBrHov, hBrHint;
static HFONT  hFontUI, hFontMono, hFontSmall, hFontIcon;

static void InitResources() {
    static bool done = false;
    if (done) return;
    done = true;
    hBrDark = CreateSolidBrush(BG_DARK);
    hBrLeft = CreateSolidBrush(BG_LEFT);
    hBrRight = CreateSolidBrush(BG_RIGHT);
    hBrSearch = CreateSolidBrush(BG_SEARCH);
    hBrSel = CreateSolidBrush(BG_ITEM_SEL);
    hBrItem = CreateSolidBrush(BG_ITEM);
    hBrHov = CreateSolidBrush(BG_ITEM_HOV);
    hBrHint = CreateSolidBrush(BG_HINT);

    hFontUI = CreateFontW(18, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    hFontSmall = CreateFontW(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    hFontMono = CreateFontW(15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        FIXED_PITCH | FF_DONTCARE, L"Cascadia Mono");
    hFontIcon = CreateFontW(18, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI Symbol");
}

static COLORREF TypeColor(ClipType t) {
    switch (t) {
    case ClipType::URL:      return CLR_URL;
    case ClipType::Color:    return CLR_COLOR_T;
    case ClipType::FilePath: return CLR_PATH;
    case ClipType::Email:    return CLR_EMAIL;
    case ClipType::Image:    return CLR_IMAGE;
    case ClipType::FileRef:  return C(90, 140, 90);
    default:                 return CLR_DIM;
    }
}

static std::wstring RelativeTime(time_t ts) {
    if (ts == 0) return L"Unknown";
    time_t now = time(nullptr);
    long long diff = (long long)now - (long long)ts;
    if (diff < 0) diff = 0;

    if (diff < 60)          return L"Just now";
    if (diff < 3600)        return std::to_wstring(diff / 60) + L" min ago";
    if (diff < 86400)       return std::to_wstring(diff / 3600) + L" hr ago";
    if (diff < 86400 * 7)     return std::to_wstring(diff / 86400) + L" days ago";

    tm t;
    localtime_s(&t, &ts);
    wchar_t buf[32];
    swprintf_s(buf, L"%02d/%02d/%04d", t.tm_mon + 1, t.tm_mday, t.tm_year + 1900);
    return buf;
}

static std::wstring TypeIcon(ClipType t) {
    switch (t) {
    case ClipType::URL:      return L"\U0001F517";
    case ClipType::Color:    return L"\U0001F3A8";
    case ClipType::FilePath: return L"\U0001F4C1";
    case ClipType::Email:    return L"\U00002709";
    case ClipType::Image:    return L"\U0001F5BC";
    case ClipType::FileRef:  return L"\U0001F4E6";
    default:                 return L"\U0001F4CB";
    }
}

// Ã¢â€â‚¬Ã¢â€â‚¬ Helpers Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬
static void FillRoundRect(HDC hdc, RECT rc, int r, HBRUSH br) {
    HRGN rgn = CreateRoundRectRgn(rc.left, rc.top, rc.right, rc.bottom, r, r);
    FillRgn(hdc, rgn, br);
    DeleteObject(rgn);
}

static void DrawTextLine(HDC hdc, const std::wstring& s,
    RECT rc, COLORREF clr, HFONT font, UINT fmt = DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS) {
    SelectObject(hdc, font);
    SetTextColor(hdc, clr);
    SetBkMode(hdc, TRANSPARENT);
    DrawTextW(hdc, s.c_str(), (int)s.size(), &rc, fmt);
}

static void HLine(HDC hdc, int x1, int x2, int y, COLORREF c) {
    HPEN p = CreatePen(PS_SOLID, 1, c);
    HPEN o = (HPEN)SelectObject(hdc, p);
    MoveToEx(hdc, x1, y, nullptr); LineTo(hdc, x2, y);
    SelectObject(hdc, o); DeleteObject(p);
}

static void VLine(HDC hdc, int x, int y1, int y2, COLORREF c) {
    HPEN p = CreatePen(PS_SOLID, 1, c);
    HPEN o = (HPEN)SelectObject(hdc, p);
    MoveToEx(hdc, x, y1, nullptr); LineTo(hdc, x, y2);
    SelectObject(hdc, o); DeleteObject(p);
}

static void DrawHintKey(HDC hdc, HFONT font, int x, int y,
    const wchar_t* k, const wchar_t* label) {
    SIZE sz; SelectObject(hdc, font);
    GetTextExtentPoint32W(hdc, k, (int)wcslen(k), &sz);
    RECT krc = { x, y + 2, x + sz.cx + 8, y + sz.cy + 4 };
    HBRUSH kb = CreateSolidBrush(C(225, 232, 245));
    FillRect(hdc, &krc, kb); DeleteObject(kb);
    SetTextColor(hdc, CLR_HINT_KEY);
    SetBkMode(hdc, TRANSPARENT);
    TextOutW(hdc, x + 4, y + 3, k, (int)wcslen(k));
    SetTextColor(hdc, CLR_DIM);
    TextOutW(hdc, krc.right + 4, y + 3, label, (int)wcslen(label));
}

// Icon "chip": a light tint of the type color behind the glyph, so icons
// are visible at a glance instead of a flat gray/dim symbol on white.
static void DrawIconBadge(HDC hdc, int x, int y, const std::wstring& icon,
    COLORREF color, HFONT font) {
    RECT badge = { x, y, x + 24, y + 24 };
    HBRUSH tint = CreateSolidBrush(RGB(
        (std::min)(255, GetRValue(color) + (255 - GetRValue(color)) * 3 / 4),
        (std::min)(255, GetGValue(color) + (255 - GetGValue(color)) * 3 / 4),
        (std::min)(255, GetBValue(color) + (255 - GetBValue(color)) * 3 / 4)));
    FillRoundRect(hdc, badge, 6, tint);
    DeleteObject(tint);
    SelectObject(hdc, font);
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    DrawTextW(hdc, icon.c_str(), (int)icon.size(), &badge, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

// Ã¢â€â‚¬Ã¢â€â‚¬ Snippet Editor dialog Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬
namespace {
    struct SnippetDlgCtx {
        HWND nameEdit = nullptr;
        HWND textEdit = nullptr;
        bool confirmed = false;
        std::wstring name;
        std::wstring text;
    };

    LRESULT CALLBACK SnippetDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        SnippetDlgCtx* ctx = nullptr;
        if (msg == WM_NCCREATE) {
            CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
            ctx = reinterpret_cast<SnippetDlgCtx*>(cs->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ctx));
        }
        else {
            ctx = reinterpret_cast<SnippetDlgCtx*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        }

        switch (msg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == 1) {
                if (ctx) {
                    wchar_t nameBuf[128] = {}, textBuf[2048] = {};
                    GetWindowTextW(ctx->nameEdit, nameBuf, 128);
                    GetWindowTextW(ctx->textEdit, textBuf, 2048);
                    ctx->name = nameBuf;
                    ctx->text = textBuf;
                    ctx->confirmed = true;
                }
                DestroyWindow(hwnd);
                return 0;
            }
            if (LOWORD(wParam) == 2) {
                DestroyWindow(hwnd);
                return 0;
            }
            return 0;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) { DestroyWindow(hwnd); return 0; }
            return 0;
        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc; GetClientRect(hwnd, &rc);
            FillRect(hdc, &rc, (HBRUSH)(COLOR_3DFACE + 1));
            return 1;
        }
        }
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
} // namespace

// Ã¢â€â‚¬Ã¢â€â‚¬ Popup::Create Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬Ã¢â€â‚¬
bool Popup::Create(HINSTANCE hInst) {
    m_hInst = hInst;
    InitResources();

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = hBrDark;
    wc.lpszClassName = CLASS_NAME;
    RegisterClassExW(&wc);

    m_hwnd = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
        CLASS_NAME, L"",
        WS_POPUP,
        0, 0, W, H,
        nullptr, nullptr, hInst, this);
    if (!m_hwnd) return false;

    m_search = CreateWindowExW(
        0, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        60, 50, LEFT_W - 80, 24,
        m_hwnd, (HMENU)101, hInst, nullptr);
    SendMessageW(m_search, WM_SETFONT, (WPARAM)hFontUI, TRUE);

    SetWindowSubclass(m_search, SearchProc, 1,
        reinterpret_cast<DWORD_PTR>(this));

    return true;
}

LRESULT CALLBACK Popup::SearchProc(HWND hwnd, UINT msg, WPARAM wParam,
    LPARAM lParam, UINT_PTR, DWORD_PTR data) {
    Popup* self = reinterpret_cast<Popup*>(data);

    if (msg == WM_KEYDOWN) {
        if (wParam == VK_RETURN) { self->ConfirmSelection(); return 0; }
        if (wParam == VK_ESCAPE) { self->Hide(); return 0; }
        if (wParam == VK_DOWN) {
            int count = self->m_showingSnippets
                ? (int)self->m_filteredSnippets.size()
                : (int)self->m_filtered.size();
            if (self->m_selected < count - 1) {
                self->m_selected++;
                if (self->m_showingSnippets) self->UpdateSnippetPreview(self->m_selected);
                else self->UpdatePreview(self->m_filtered[self->m_selected]);
                InvalidateRect(self->m_hwnd, nullptr, FALSE);
            }
            return 0;
        }
        if (wParam == VK_UP) {
            if (self->m_selected > 0) {
                self->m_selected--;
                if (self->m_showingSnippets) self->UpdateSnippetPreview(self->m_selected);
                else self->UpdatePreview(self->m_filtered[self->m_selected]);
                InvalidateRect(self->m_hwnd, nullptr, FALSE);
            }
            return 0;
        }
        if (wParam == 'P' && (GetKeyState(VK_CONTROL) & 0x8000) && !self->m_showingSnippets) {
            self->TogglePin(); return 0;
        }
        if (wParam == VK_DELETE && (GetKeyState(VK_CONTROL) & 0x8000)) {
            if (self->m_showingSnippets) self->DeleteSnippetSelected();
            else self->DeleteSelected();
            return 0;
        }
        if (wParam == 'N' && (GetKeyState(VK_CONTROL) & 0x8000) && self->m_showingSnippets) {
            self->SnippetEditor(-1);
            return 0;
        }
        if (wParam == 'E' && (GetKeyState(VK_CONTROL) & 0x8000) && self->m_showingSnippets) {
            if (self->m_selected >= 0 && self->m_selected < (int)self->m_filteredSnippets.size())
                self->SnippetEditor(self->m_filteredSnippets[self->m_selected]);
            return 0;
        }
    }
    if (msg == WM_CHAR) {
        if (wParam == VK_RETURN || wParam == VK_ESCAPE) return 0;
        LRESULT r = DefSubclassProc(hwnd, msg, wParam, lParam);
        wchar_t buf[256] = {};
        GetWindowTextW(hwnd, buf, 256);
        if (self->m_showingSnippets) {
            self->m_snippetSearchText = buf;
            self->PopulateSnippets(buf);
        }
        else {
            self->m_searchText = buf;
            self->PopulateList(buf);
        }
        InvalidateRect(self->m_hwnd, nullptr, FALSE);
        return r;
    }
    if (msg == WM_CTLCOLOREDIT) {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, BG_SEARCH);
        SetTextColor(hdc, CLR_TEXT);
        return (LRESULT)hBrSearch;
    }
    return DefSubclassProc(hwnd, msg, wParam, lParam);
}

void Popup::Show(const std::vector<ClipEntry>& history) {
    m_history = history;
    m_selected = 0;
    m_showingSnippets = false;
    m_searchText.clear();
    m_snippetSearchText.clear();
    SetWindowTextW(m_search, L"");
    PopulateList();
    PopulateSnippets();
    if (!m_filtered.empty())
        UpdatePreview(m_filtered[0]);
    PositionNearCursor();
    ShowWindow(m_hwnd, SW_SHOW);
    SetForegroundWindow(m_hwnd);
    SetFocus(m_search);
    InvalidateRect(m_hwnd, nullptr, TRUE);
}

void Popup::Hide() { ShowWindow(m_hwnd, SW_HIDE); }

bool Popup::IsVisible() const { return IsWindowVisible(m_hwnd) != FALSE; }

void Popup::PositionNearCursor() {
    POINT pt; GetCursorPos(&pt);
    MONITORINFO mi = {}; mi.cbSize = sizeof(mi);
    GetMonitorInfoW(MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST), &mi);
    RECT work = mi.rcWork;

    int x = pt.x - W / 2;
    int y = pt.y - H - 8;
    if (x + W > work.right) x = work.right - W - 8;
    if (x < work.left)       x = work.left + 8;
    if (y < work.top)        y = pt.y + 20;

    SetWindowPos(m_hwnd, HWND_TOPMOST, x, y, W, H, 0);
}

void Popup::PopulateList(const std::wstring& filter) {
    m_filtered.clear();
    for (int pass = 0; pass < 2; pass++) {
        for (int i = 0; i < (int)m_history.size(); i++) {
            const ClipEntry& e = m_history[i];
            if (pass == 0 && !e.pinned) continue;
            if (pass == 1 && e.pinned) continue;
            bool match = filter.empty();
            if (!match) {
                std::wstring lo = e.text, lf = filter;
                std::transform(lo.begin(), lo.end(), lo.begin(), ::towlower);
                std::transform(lf.begin(), lf.end(), lf.begin(), ::towlower);
                match = lo.find(lf) != std::wstring::npos;
            }
            if (match) m_filtered.push_back(i);
        }
    }
    m_selected = 0;
    if (!m_filtered.empty())
        UpdatePreview(m_filtered[0]);
}

void Popup::PopulateSnippets(const std::wstring& filter) {
    m_filteredSnippets.clear();
    if (!m_snippets) { m_previewSnippetRealIndex = -1; return; }
    for (int i = 0; i < (int)m_snippets->size(); i++) {
        if (filter.empty()) { m_filteredSnippets.push_back(i); continue; }
        std::wstring name = (*m_snippets)[i].name, text = (*m_snippets)[i].text, lf = filter;
        std::transform(name.begin(), name.end(), name.begin(), ::towlower);
        std::transform(text.begin(), text.end(), text.begin(), ::towlower);
        std::transform(lf.begin(), lf.end(), lf.begin(), ::towlower);
        if (name.find(lf) != std::wstring::npos || text.find(lf) != std::wstring::npos)
            m_filteredSnippets.push_back(i);
    }
    if (m_selected >= (int)m_filteredSnippets.size())
        m_selected = m_filteredSnippets.empty() ? 0 : (int)m_filteredSnippets.size() - 1;

    if (m_filteredSnippets.empty()) m_previewSnippetRealIndex = -1;
    else UpdateSnippetPreview(m_selected);
}

void Popup::UpdatePreview(int historyIndex) {
    if (historyIndex < 0 || historyIndex >= (int)m_history.size()) return;
    const ClipEntry& e = m_history[historyIndex];
    m_previewText = e.text;
    m_previewType = e.type;
    m_previewPinned = e.pinned;
    m_previewIndex = historyIndex;
    m_previewTimestamp = e.timestamp;
    m_previewImagePath = e.imagePath;
    m_previewFilePaths = e.filePaths;
}

void Popup::UpdateSnippetPreview(int selIdx) {
    if (!m_snippets || selIdx < 0 || selIdx >= (int)m_filteredSnippets.size()) {
        m_previewSnippetRealIndex = -1;
        return;
    }
    int realIdx = m_filteredSnippets[selIdx];
    if (realIdx < 0 || realIdx >= (int)m_snippets->size()) {
        m_previewSnippetRealIndex = -1;
        return;
    }
    m_previewSnippetRealIndex = realIdx;
    m_previewSnippetName = (*m_snippets)[realIdx].name;
    m_previewSnippetText = (*m_snippets)[realIdx].text;
}

void Popup::ConfirmSelection() {
    if (m_showingSnippets) {
        if (m_selected < 0 || m_selected >= (int)m_filteredSnippets.size()) return;
        int realIdx = m_filteredSnippets[m_selected];
        if (!m_snippets || realIdx >= (int)m_snippets->size()) return;
        std::wstring text = (*m_snippets)[realIdx].text;
        Hide();
        if (OnPasteSnippet) OnPasteSnippet(text);
        return;
    }
    if (m_selected < 0 || m_selected >= (int)m_filtered.size()) return;
    int idx = m_filtered[m_selected];
    Hide();
    if (OnSelect) OnSelect(idx);
}

void Popup::TogglePin() {
    if (m_selected < 0 || m_selected >= (int)m_filtered.size()) return;
    int idx = m_filtered[m_selected];
    m_history[idx].pinned = !m_history[idx].pinned;
    PopulateList(m_searchText);
    InvalidateRect(m_hwnd, nullptr, FALSE);
    if (OnPin) OnPin(idx);
}

void Popup::DeleteSelected() {
    if (m_selected < 0 || m_selected >= (int)m_filtered.size()) return;
    int idx = m_filtered[m_selected];
    if (OnDelete) OnDelete(idx);
    m_history.erase(m_history.begin() + idx);
    PopulateList(m_searchText);
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

std::wstring Popup::GetTypeName(ClipType type) {
    switch (type) {
    case ClipType::URL:      return L"URL";
    case ClipType::Color:    return L"Color";
    case ClipType::FilePath: return L"File Path";
    case ClipType::Email:    return L"Email";
    case ClipType::Image:    return L"Image";
    case ClipType::FileRef:  return L"Files";
    default:                 return L"Text";
    }
}

void Popup::PaintSearchBar(HDC hdc, const std::wstring& text) {
    RECT srch = { 0, TAB_STRIP_H, LEFT_W, HEADER_H };
    FillRect(hdc, &srch, hBrDark);

    SelectObject(hdc, hFontIcon);
    SetTextColor(hdc, CLR_DIM);
    SetBkMode(hdc, TRANSPARENT);
    TextOutW(hdc, 16, TAB_STRIP_H + 16, L"\U0001F50D", 2);

    if (text.empty()) {
        DrawTextLine(hdc, L"Type to search...",
            { 56, TAB_STRIP_H + 14, LEFT_W - 12, HEADER_H - 14 }, CLR_DIM, hFontUI);
    }

    HLine(hdc, 0, LEFT_W, HEADER_H, CLR_BORDER);
}

void Popup::PaintLeftPanel(HDC hdc) {
    RECT rc = { 0, 0, LEFT_W, H };
    FillRect(hdc, &rc, hBrLeft);

    RECT tabStrip = { 0, 0, LEFT_W, TAB_STRIP_H };
    FillRect(hdc, &tabStrip, hBrDark);

    int halfW = LEFT_W / 2;
    RECT clipsTabRc = { 0, 0, halfW, TAB_STRIP_H };
    RECT snipTabRc = { halfW, 0, LEFT_W, TAB_STRIP_H };

    SelectObject(hdc, hFontUI);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, m_showingSnippets ? CLR_DIM : CLR_TEXT);
    DrawTextW(hdc, L"Clips", 5, &clipsTabRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SetTextColor(hdc, m_showingSnippets ? CLR_TEXT : CLR_DIM);
    DrawTextW(hdc, L"Snippets", 8, &snipTabRc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    int underlineY = TAB_STRIP_H - 2;
    if (m_showingSnippets)
        HLine(hdc, halfW, LEFT_W, underlineY, BG_ITEM_SEL);
    else
        HLine(hdc, 0, halfW, underlineY, BG_ITEM_SEL);
    HLine(hdc, 0, LEFT_W, TAB_STRIP_H, CLR_BORDER);

    if (m_showingSnippets) {
        PaintSnippetTab(hdc);
        return;
    }

    PaintSearchBar(hdc, m_searchText);

    int listTop = HEADER_H;
    int itemHeight = m_compactMode ? 40 : ITEM_H;
    int availableHeight = H - listTop - HINT_H;
    int visibleItems = availableHeight / itemHeight;

    int startIdx = 0;
    if (m_selected >= visibleItems)
        startIdx = m_selected - visibleItems + 1;

    for (int vi = 0; vi < visibleItems && (startIdx + vi) < (int)m_filtered.size(); vi++) {
        int i = startIdx + vi;
        int hidx = m_filtered[i];
        const ClipEntry& e = m_history[hidx];
        bool sel = (i == m_selected);

        int y = listTop + vi * itemHeight;
        RECT itemRc = { 0, y, LEFT_W, y + itemHeight };

        if (sel)
            FillRect(hdc, &itemRc, hBrSel);
        else
            FillRect(hdc, &itemRc, hBrItem);

        if (sel) {
            RECT accent = { 0, y, 3, y + itemHeight };
            HBRUSH hAcc = CreateSolidBrush(CLR_WHITE);
            FillRect(hdc, &accent, hAcc);
            DeleteObject(hAcc);
        }

        if (sel) {
            SelectObject(hdc, hFontIcon);
            SetTextColor(hdc, CLR_WHITE);
            SetBkMode(hdc, TRANSPARENT);
            std::wstring icon = TypeIcon(e.type);
            TextOutW(hdc, 10, y + 16, icon.c_str(), (int)icon.size());
        }
        else {
            DrawIconBadge(hdc, 8, y + 12, TypeIcon(e.type), TypeColor(e.type), hFontIcon);
        }

        if (e.pinned) {
            SelectObject(hdc, hFontSmall);
            SetTextColor(hdc, CLR_PIN);
            SetBkMode(hdc, TRANSPARENT);
            TextOutW(hdc, LEFT_W - 22, y + 18, L"\u2605", 1);
        }

        std::wstring preview = e.text.substr(0, 80);
        auto nl = preview.find_first_of(L"\r\n");
        if (nl != std::wstring::npos) preview = preview.substr(0, nl);

        RECT textRc = { 34, y + 10, LEFT_W - 28, y + 30 };
        DrawTextLine(hdc, preview, textRc,
            sel ? CLR_WHITE : CLR_TEXT, hFontUI,
            DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);

        if (!m_compactMode) {
            std::wstring sub = GetTypeName(e.type);
            if (m_showTimestamps) {
                sub += L"  \u2022  " + RelativeTime(e.timestamp);
            }
            RECT subRc = { 34, y + 30, LEFT_W - 28, y + itemHeight - 4 };
            DrawTextLine(hdc, sub, subRc,
                sel ? C(225, 235, 255) : CLR_DIM, hFontSmall,
                DT_LEFT | DT_TOP | DT_SINGLELINE);
        }

        if (!sel)
            HLine(hdc, 8, LEFT_W - 8, y + itemHeight - 1, CLR_SEP);
    }

    RECT hintRc = { 0, H - HINT_H, LEFT_W, H };
    FillRect(hdc, &hintRc, hBrHint);
    HLine(hdc, 0, LEFT_W, H - HINT_H, CLR_BORDER);

    DrawHintKey(hdc, hFontSmall, 8, H - HINT_H + 9, L"\u21B5", L"Paste");
    DrawHintKey(hdc, hFontSmall, 76, H - HINT_H + 9, L"^P", L"Pin");
    DrawHintKey(hdc, hFontSmall, 128, H - HINT_H + 9, L"^Del", L"Remove");
    DrawHintKey(hdc, hFontSmall, 196, H - HINT_H + 9, L"Esc", L"Close");
}

void Popup::PaintSnippetTab(HDC hdc) {
    PaintSearchBar(hdc, m_snippetSearchText);

    int listTop = HEADER_H;
    int itemHeight = 56;

    bool hasAny = m_snippets && !m_snippets->empty();
    if (m_filteredSnippets.empty()) {
        std::wstring msg = hasAny
            ? L"No matches.\nTry a different search."
            : L"No snippets yet.\nRight-click a clip to save one,\nor press Ctrl+N to add.";
        DrawTextLine(hdc, msg,
            { 16, listTop + 20, LEFT_W - 16, listTop + 100 },
            CLR_DIM, hFontSmall, DT_LEFT | DT_TOP | DT_WORDBREAK);
    }
    else {
        for (int vi = 0; vi < (int)m_filteredSnippets.size(); vi++) {
            int realIdx = m_filteredSnippets[vi];
            const Snippet& s = (*m_snippets)[realIdx];
            int y = listTop + vi * itemHeight;
            if (y > H - HINT_H - itemHeight) break;

            bool sel = (vi == m_selected);
            RECT itemRc = { 0, y, LEFT_W, y + itemHeight };
            FillRect(hdc, &itemRc, sel ? hBrSel : hBrItem);

            if (sel) {
                SelectObject(hdc, hFontIcon);
                SetTextColor(hdc, CLR_WHITE);
                SetBkMode(hdc, TRANSPARENT);
                TextOutW(hdc, 10, y + 16, L"\U0001F4CC", 2);
            }
            else {
                DrawIconBadge(hdc, 8, y + 12, L"\U0001F4CC", CLR_SNIPPET, hFontIcon);
            }

            RECT nameRc = { 34, y + 8, LEFT_W - 12, y + 28 };
            DrawTextLine(hdc, s.name, nameRc, sel ? CLR_WHITE : CLR_TEXT, hFontUI,
                DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);

            std::wstring preview = s.text.substr(0, 60);
            for (wchar_t& c : preview) if (c == L'\n' || c == L'\r') c = L' ';
            RECT prevRc = { 34, y + 30, LEFT_W - 12, y + itemHeight - 4 };
            DrawTextLine(hdc, preview, prevRc, sel ? C(225, 235, 255) : CLR_DIM, hFontSmall,
                DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);

            if (!sel)
                HLine(hdc, 8, LEFT_W - 8, y + itemHeight - 1, CLR_SEP);
        }
    }

    RECT hintRc = { 0, H - HINT_H, LEFT_W, H };
    FillRect(hdc, &hintRc, hBrHint);
    HLine(hdc, 0, LEFT_W, H - HINT_H, CLR_BORDER);

    DrawHintKey(hdc, hFontSmall, 8, H - HINT_H + 9, L"^N", L"New");
    DrawHintKey(hdc, hFontSmall, 70, H - HINT_H + 9, L"^E", L"Edit");
    DrawHintKey(hdc, hFontSmall, 132, H - HINT_H + 9, L"^Del", L"Remove");
}

void Popup::SetSnippets(std::vector<Snippet>* snippets) {
    m_snippets = snippets;
    PopulateSnippets(m_snippetSearchText);
}

void Popup::ToggleSnippetsView() {
    m_showingSnippets = !m_showingSnippets;
    m_selected = 0;
    SetWindowTextW(m_search, m_showingSnippets ? m_snippetSearchText.c_str() : m_searchText.c_str());
    if (m_showingSnippets) PopulateSnippets(m_snippetSearchText);
    else                   PopulateList(m_searchText);
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void Popup::SnippetEditor(int editIndex, const std::wstring& prefillText) {
    if (!m_snippets) return;
    bool isEdit = editIndex >= 0 && editIndex < (int)m_snippets->size();

    std::wstring initText = isEdit ? (*m_snippets)[editIndex].text : prefillText;
    std::wstring initName = isEdit ? (*m_snippets)[editIndex].name : L"";
    if (!isEdit && initName.empty() && !prefillText.empty()) {
        size_t nl = prefillText.find_first_of(L"\r\n");
        initName = prefillText.substr(0, nl == std::wstring::npos ? prefillText.size() : nl);
        if (initName.size() > 40) initName = initName.substr(0, 40) + L"\u2026"; // ellipsis marks it as auto-truncated
    }

    static bool classRegistered = false;
    if (!classRegistered) {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = SnippetDlgProc;
        wc.hInstance = m_hInst;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
        wc.lpszClassName = L"ClipManagerSnippetDlg";
        RegisterClassExW(&wc);
        classRegistered = true;
    }

    SnippetDlgCtx ctx;

    const int CLIENT_W = 360;
    const int CLIENT_H = 244;
    DWORD style = WS_POPUP | WS_CAPTION | WS_SYSMENU;
    DWORD exStyle = WS_EX_TOPMOST | WS_EX_DLGMODALFRAME;

    RECT winRc = { 0, 0, CLIENT_W, CLIENT_H };
    AdjustWindowRectEx(&winRc, style, FALSE, exStyle);
    int winW = winRc.right - winRc.left;
    int winH = winRc.bottom - winRc.top;

    HWND dlg = CreateWindowExW(exStyle,
        L"ClipManagerSnippetDlg", isEdit ? L"Edit Snippet" : L"Add Snippet",
        style | WS_VISIBLE,
        0, 0, winW, winH, m_hwnd, nullptr, m_hInst, &ctx);
    if (!dlg) return;

    RECT ownerRc; GetWindowRect(m_hwnd, &ownerRc);
    SetWindowPos(dlg, HWND_TOPMOST,
        ownerRc.left + (W - winW) / 2, ownerRc.top + (H - winH) / 2, 0, 0, SWP_NOSIZE);

    RECT client; GetClientRect(dlg, &client);
    int cw = client.right - 32;

    CreateWindowExW(0, L"STATIC", L"Snippet name:",
        WS_CHILD | WS_VISIBLE, 16, 16, 200, 20, dlg, nullptr, m_hInst, nullptr);
    ctx.nameEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", initName.c_str(),
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 16, 40, cw, 24,
        dlg, nullptr, m_hInst, nullptr);

    CreateWindowExW(0, L"STATIC", L"Snippet text:",
        WS_CHILD | WS_VISIBLE, 16, 74, 200, 20, dlg, nullptr, m_hInst, nullptr);
    ctx.textEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", initText.c_str(),
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_MULTILINE | WS_VSCROLL,
        16, 98, cw, 90, dlg, nullptr, m_hInst, nullptr);

    CreateWindowExW(0, L"BUTTON", isEdit ? L"Save" : L"Add",
        WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, client.right - 176, 200, 80, 28,
        dlg, (HMENU)1, m_hInst, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Cancel",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, client.right - 88, 200, 80, 28,
        dlg, (HMENU)2, m_hInst, nullptr);

    EnumChildWindows(dlg, [](HWND h, LPARAM f) -> BOOL {
        SendMessageW(h, WM_SETFONT, (WPARAM)f, TRUE);
        return TRUE;
        }, (LPARAM)GetStockObject(DEFAULT_GUI_FONT));

    SetFocus(ctx.nameEdit);
    SendMessageW(ctx.nameEdit, EM_SETSEL, 0, -1);

    m_dialogOpen = true;

    MSG msg;
    while (IsWindow(dlg) && GetMessageW(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE &&
            (msg.hwnd == dlg || IsChild(dlg, msg.hwnd))) {
            DestroyWindow(dlg);
            break;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    m_dialogOpen = false;

    if (ctx.confirmed && !ctx.name.empty() && !ctx.text.empty()) {
        if (isEdit) (*m_snippets)[editIndex] = { ctx.name, ctx.text };
        else        m_snippets->push_back({ ctx.name, ctx.text });
        PopulateSnippets(m_snippetSearchText);
        if (OnSnippetsChanged) OnSnippetsChanged();
    }

    SetForegroundWindow(m_hwnd);
    SetFocus(m_search);
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void Popup::DeleteSnippetSelected() {
    if (!m_snippets) return;
    if (m_selected < 0 || m_selected >= (int)m_filteredSnippets.size()) return;
    int realIdx = m_filteredSnippets[m_selected];
    m_snippets->erase(m_snippets->begin() + realIdx);
    PopulateSnippets(m_snippetSearchText);
    if (OnSnippetsChanged) OnSnippetsChanged();
    InvalidateRect(m_hwnd, nullptr, FALSE);
}

void Popup::PaintClipPreview(HDC hdc) {
    int rx = LEFT_W + 1;

    if (m_previewIndex < 0 || m_filtered.empty()) {
        DrawTextLine(hdc, L"Select an item to preview",
            { rx + 20, H / 2 - 10, W - 20, H / 2 + 10 },
            CLR_DIM, hFontUI, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        return;
    }

    int y = 20;

    std::wstring typeName = GetTypeName(m_previewType);
    COLORREF typeClr = TypeColor(m_previewType);
    SelectObject(hdc, hFontSmall);
    SIZE tsz; GetTextExtentPoint32W(hdc, typeName.c_str(), (int)typeName.size(), &tsz);
    RECT badgeRc = { rx + 20, y, rx + 20 + tsz.cx + 16, y + tsz.cy + 6 };
    HBRUSH hBadge = CreateSolidBrush(RGB(
        GetRValue(typeClr) / 4, GetGValue(typeClr) / 4, GetBValue(typeClr) / 4));
    FillRect(hdc, &badgeRc, hBadge); DeleteObject(hBadge);
    SetTextColor(hdc, typeClr);
    SetBkMode(hdc, TRANSPARENT);
    TextOutW(hdc, rx + 28, y + 3, typeName.c_str(), (int)typeName.size());

    m_quickActionRect = { 0,0,0,0 };
    std::wstring actionLabel;
    if (m_previewType == ClipType::URL)      actionLabel = L"Open in Browser";
    else if (m_previewType == ClipType::FilePath)  actionLabel = L"Open in Explorer";
    else if (m_previewType == ClipType::Color)     actionLabel = L"Copy Hex";

    if (!actionLabel.empty()) {
        SIZE asz; GetTextExtentPoint32W(hdc, actionLabel.c_str(), (int)actionLabel.size(), &asz);
        int ax = badgeRc.right + 12;
        RECT actRc = { ax, y, ax + asz.cx + 20, y + tsz.cy + 6 };
        HBRUSH hActBg = CreateSolidBrush(RGB(61, 127, 232));
        FillRoundRect(hdc, actRc, 4, hActBg);
        DeleteObject(hActBg);
        SetTextColor(hdc, RGB(255, 255, 255));
        TextOutW(hdc, ax + 10, y + 3, actionLabel.c_str(), (int)actionLabel.size());
        m_quickActionRect = actRc;
    }

    y += tsz.cy + 16;

    if (m_previewType == ClipType::Image && !m_previewImagePath.empty()) {
        HBITMAP hThumb = Imaging::LoadThumbnail(m_previewImagePath, 380, 260);
        if (hThumb) {
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP old = (HBITMAP)SelectObject(memDC, hThumb);
            BITMAP bm; GetObject(hThumb, sizeof(bm), &bm);

            int imgX = rx + 20 + (380 - bm.bmWidth) / 2;
            BitBlt(hdc, imgX, y, bm.bmWidth, bm.bmHeight, memDC, 0, 0, SRCCOPY);

            SelectObject(memDC, old);
            DeleteDC(memDC);
            DeleteObject(hThumb);
            y += 260 + 10;
        }
        else {
            DrawTextLine(hdc, L"Image preview unavailable",
                { rx + 20, y, W - 20, y + 30 }, CLR_DIM, hFontUI);
            y += 40;
        }
    }
    else if (m_previewType == ClipType::FileRef) {
        SelectObject(hdc, hFontMono ? hFontMono : hFontUI);
        SetTextColor(hdc, CLR_TEXT);
        SetBkMode(hdc, TRANSPARENT);
        int listY = y;
        for (size_t i = 0; i < m_previewFilePaths.size() && i < 8; i++) {
            RECT fileRc = { rx + 20, listY, W - 20, listY + 22 };
            DrawTextLine(hdc, m_previewFilePaths[i], fileRc, CLR_TEXT, hFontSmall,
                DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS | DT_PATH_ELLIPSIS);
            listY += 24;
        }
        if (m_previewFilePaths.size() > 8) {
            std::wstring more = L"+ " + std::to_wstring(m_previewFilePaths.size() - 8) + L" more";
            DrawTextLine(hdc, more, { rx + 20, listY, W - 20, listY + 22 }, CLR_DIM, hFontSmall);
            listY += 24;
        }
        y = listY + 10;
    }
    else {
        std::wstring preview = m_previewText.substr(0, 600);
        RECT previewRc = { rx + 20, y, W - 20, y + 260 };
        SelectObject(hdc, hFontMono ? hFontMono : hFontUI);
        SetTextColor(hdc, CLR_TEXT);
        SetBkMode(hdc, TRANSPARENT);
        DrawTextW(hdc, preview.c_str(), (int)preview.size(), &previewRc,
            DT_LEFT | DT_TOP | DT_WORDBREAK | DT_END_ELLIPSIS);
        y += 270;
    }

    HLine(hdc, rx + 20, W - 20, y, CLR_BORDER);
    y += 16;

    auto MetaRow = [&](const wchar_t* label, const std::wstring& value) {
        RECT lrc = { rx + 20, y, rx + 120, y + 20 };
        RECT vrc = { rx + 130, y, W - 20,  y + 20 };
        DrawTextLine(hdc, label, lrc, CLR_DIM, hFontSmall, DT_LEFT | DT_TOP | DT_SINGLELINE);
        DrawTextLine(hdc, value, vrc, CLR_TEXT, hFontSmall,
            DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);
        y += 22;
        };

    MetaRow(L"Type", GetTypeName(m_previewType));
    MetaRow(L"Copied", RelativeTime(m_previewTimestamp));
    if (m_previewType != ClipType::Image && m_previewType != ClipType::FileRef) {
        MetaRow(L"Characters", std::to_wstring(m_previewText.size()));
        int lines = 1;
        for (wchar_t c : m_previewText) if (c == L'\n') lines++;
        MetaRow(L"Lines", std::to_wstring(lines));
    }

    if (m_previewType == ClipType::FileRef) {
        MetaRow(L"Files", std::to_wstring(m_previewFilePaths.size()));
    }

    if (m_previewPinned)
        MetaRow(L"Pinned", L"\u2605 Yes");

    if (m_previewType == ClipType::Color) {
        y += 8;
        std::wstring hex = m_previewText;
        while (!hex.empty() && hex[0] == L'#') hex = hex.substr(1);
        if (hex.size() == 3) {
            hex = std::wstring(2, hex[0]) +
                std::wstring(2, hex[1]) +
                std::wstring(2, hex[2]);
        }
        if (hex.size() == 6) {
            int r = std::stoi(hex.substr(0, 2), nullptr, 16);
            int g = std::stoi(hex.substr(2, 2), nullptr, 16);
            int b = std::stoi(hex.substr(4, 2), nullptr, 16);
            RECT swatch = { rx + 20, y, rx + 60, y + 30 };
            HBRUSH hSw = CreateSolidBrush(RGB(r, g, b));
            FillRect(hdc, &swatch, hSw);
            DeleteObject(hSw);
            RECT swlbl = { rx + 68, y + 6, W - 20, y + 24 };
            DrawTextLine(hdc, m_previewText, swlbl, CLR_TEXT, hFontUI,
                DT_LEFT | DT_TOP | DT_SINGLELINE);
        }
    }
}

void Popup::PaintSnippetPreview(HDC hdc) {
    int rx = LEFT_W + 1;

    if (m_previewSnippetRealIndex < 0 || m_filteredSnippets.empty()) {
        DrawTextLine(hdc, L"Select a snippet to preview",
            { rx + 20, H / 2 - 10, W - 20, H / 2 + 10 },
            CLR_DIM, hFontUI, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        return;
    }

    int y = 20;

    std::wstring typeName = L"Snippet";
    SelectObject(hdc, hFontSmall);
    SIZE tsz; GetTextExtentPoint32W(hdc, typeName.c_str(), (int)typeName.size(), &tsz);
    RECT badgeRc = { rx + 20, y, rx + 20 + tsz.cx + 16, y + tsz.cy + 6 };
    HBRUSH hBadge = CreateSolidBrush(RGB(
        GetRValue(CLR_SNIPPET) / 4, GetGValue(CLR_SNIPPET) / 4, GetBValue(CLR_SNIPPET) / 4));
    FillRect(hdc, &badgeRc, hBadge); DeleteObject(hBadge);
    SetTextColor(hdc, CLR_SNIPPET);
    SetBkMode(hdc, TRANSPARENT);
    TextOutW(hdc, rx + 28, y + 3, typeName.c_str(), (int)typeName.size());

    y += tsz.cy + 16;

    RECT nameRc = { rx + 20, y, W - 20, y + 30 };
    DrawTextLine(hdc, m_previewSnippetName, nameRc, CLR_TEXT, hFontUI,
        DT_LEFT | DT_TOP | DT_WORDBREAK);
    y += 36;

    HLine(hdc, rx + 20, W - 20, y, CLR_BORDER);
    y += 16;

    std::wstring preview = m_previewSnippetText.substr(0, 800);
    RECT previewRc = { rx + 20, y, W - 20, y + 300 };
    SelectObject(hdc, hFontMono ? hFontMono : hFontUI);
    SetTextColor(hdc, CLR_TEXT);
    SetBkMode(hdc, TRANSPARENT);
    DrawTextW(hdc, preview.c_str(), (int)preview.size(), &previewRc,
        DT_LEFT | DT_TOP | DT_WORDBREAK | DT_END_ELLIPSIS);
    y += 310;

    HLine(hdc, rx + 20, W - 20, y, CLR_BORDER);
    y += 16;

    auto MetaRow = [&](const wchar_t* label, const std::wstring& value) {
        RECT lrc = { rx + 20, y, rx + 120, y + 20 };
        RECT vrc = { rx + 130, y, W - 20,  y + 20 };
        DrawTextLine(hdc, label, lrc, CLR_DIM, hFontSmall, DT_LEFT | DT_TOP | DT_SINGLELINE);
        DrawTextLine(hdc, value, vrc, CLR_TEXT, hFontSmall,
            DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);
        y += 22;
        };
    MetaRow(L"Characters", std::to_wstring(m_previewSnippetText.size()));
    int lines = 1;
    for (wchar_t c : m_previewSnippetText) if (c == L'\n') lines++;
    MetaRow(L"Lines", std::to_wstring(lines));
}

void Popup::PaintRightPanel(HDC hdc) {
    int rx = LEFT_W + 1;
    RECT rc = { rx, 0, W, H };
    FillRect(hdc, &rc, hBrRight);

    if (m_showingSnippets) PaintSnippetPreview(hdc);
    else                   PaintClipPreview(hdc);

    RECT actRc = { rx, H - HINT_H, W, H };
    FillRect(hdc, &actRc, hBrHint);
    HLine(hdc, rx, W, H - HINT_H, CLR_BORDER);

    DrawTextLine(hdc, L"\u21B5  Paste to active app",
        { rx + 20, H - HINT_H + 9, rx + 280, H - 8 },
        CLR_DIM, hFontSmall, DT_LEFT | DT_TOP | DT_SINGLELINE);

    DrawTextLine(hdc, L"Dvvyom Labs",
        { W - 160, H - HINT_H + 9, W - 20, H - 8 },
        CLR_DIM, hFontSmall, DT_RIGHT | DT_TOP | DT_SINGLELINE);
}

void Popup::HandleQuickAction() {
    if (m_previewType == ClipType::URL) {
        if (OnOpenUrl) OnOpenUrl(m_previewText);
    }
    else if (m_previewType == ClipType::FilePath) {
        if (OnOpenPath) OnOpenPath(m_previewText);
    }
    else if (m_previewType == ClipType::Color) {
        if (OpenClipboard(m_hwnd)) {
            EmptyClipboard();
            size_t bytes = (m_previewText.size() + 1) * sizeof(wchar_t);
            HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
            if (hMem) {
                wchar_t* p = (wchar_t*)GlobalLock(hMem);
                memcpy(p, m_previewText.c_str(), bytes);
                GlobalUnlock(hMem);
                SetClipboardData(CF_UNICODETEXT, hMem);
            }
            CloseClipboard();
        }
    }
}

LRESULT CALLBACK Popup::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Popup* self = nullptr;
    if (msg == WM_NCCREATE) {
        CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<Popup*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->m_hwnd = hwnd;
    }
    else {
        self = reinterpret_cast<Popup*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    if (!self) return DefWindowProcW(hwnd, msg, wParam, lParam);

    switch (msg) {

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rc; GetClientRect(hwnd, &rc);
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP bmp = CreateCompatibleBitmap(hdc, W, H);
        HBITMAP old = (HBITMAP)SelectObject(memDC, bmp);

        self->PaintLeftPanel(memDC);
        VLine(memDC, LEFT_W, 0, H, CLR_BORDER);
        self->PaintRightPanel(memDC);

        HPEN bp = CreatePen(PS_SOLID, 1, CLR_BORDER);
        HPEN bo = (HPEN)SelectObject(memDC, bp);
        SelectObject(memDC, GetStockObject(NULL_BRUSH));
        Rectangle(memDC, 0, 0, W, H);
        SelectObject(memDC, bo); DeleteObject(bp);

        BitBlt(hdc, 0, 0, W, H, memDC, 0, 0, SRCCOPY);
        SelectObject(memDC, old);
        DeleteObject(bmp);
        DeleteDC(memDC);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_CTLCOLOREDIT: {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, BG_SEARCH);
        SetTextColor(hdc, CLR_TEXT);
        return (LRESULT)hBrSearch;
    }

    case WM_LBUTTONDOWN: {
        int mx = LOWORD(lParam);
        int my = HIWORD(lParam);

        if (mx < LEFT_W && my < TAB_STRIP_H) {
            bool clickedSnippets = (mx >= LEFT_W / 2);
            if (clickedSnippets != self->m_showingSnippets) {
                self->ToggleSnippetsView();
            }
            SetFocus(self->m_search);
            return 0;
        }

        if (self->m_quickActionRect.right > self->m_quickActionRect.left) {
            RECT& r = self->m_quickActionRect;
            if (mx >= r.left && mx <= r.right && my >= r.top && my <= r.bottom) {
                self->HandleQuickAction();
                return 0;
            }
        }

        if (mx < LEFT_W && my >= HEADER_H && my < H - HINT_H) {
            int itemHeight = self->m_showingSnippets ? 56 : (self->m_compactMode ? 40 : ITEM_H);
            int count = self->m_showingSnippets
                ? (int)self->m_filteredSnippets.size()
                : (int)self->m_filtered.size();
            int visibleItems = (H - HEADER_H - HINT_H) / itemHeight;
            int startIdx = 0;
            if (self->m_selected >= visibleItems)
                startIdx = self->m_selected - visibleItems + 1;
            int clicked = startIdx + (my - HEADER_H) / itemHeight;
            if (clicked >= 0 && clicked < count) {
                self->m_selected = clicked;
                if (self->m_showingSnippets) self->UpdateSnippetPreview(clicked);
                else self->UpdatePreview(self->m_filtered[clicked]);
                InvalidateRect(hwnd, nullptr, FALSE);
            }
        }
        SetFocus(self->m_search);
        return 0;
    }

    case WM_RBUTTONDOWN: {
        int mx = LOWORD(lParam);
        int my = HIWORD(lParam);

        if (!self->m_showingSnippets && mx < LEFT_W && my >= HEADER_H && my < H - HINT_H) {
            int itemHeight = self->m_compactMode ? 40 : ITEM_H;
            int visibleItems = (H - HEADER_H - HINT_H) / itemHeight;
            int startIdx = 0;
            if (self->m_selected >= visibleItems)
                startIdx = self->m_selected - visibleItems + 1;
            int clicked = startIdx + (my - HEADER_H) / itemHeight;
            if (clicked >= 0 && clicked < (int)self->m_filtered.size()) {
                self->m_selected = clicked;
                self->UpdatePreview(self->m_filtered[clicked]);
                InvalidateRect(hwnd, nullptr, FALSE);

                POINT pt = { mx, my };
                ClientToScreen(hwnd, &pt);
                HMENU menu = CreatePopupMenu();
                AppendMenuW(menu, MF_STRING, 1, L"Save as Snippet");
                int cmd = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON,
                    pt.x, pt.y, 0, hwnd, nullptr);
                DestroyMenu(menu);
                if (cmd == 1) {
                    self->SnippetEditor(-1, self->m_history[self->m_filtered[clicked]].text);
                }
            }
        }
        return 0;
    }

    case WM_LBUTTONDBLCLK: {
        int mx = LOWORD(lParam);
        int my = HIWORD(lParam);
        if (mx < LEFT_W && my >= HEADER_H && my < H - HINT_H)
            self->ConfirmSelection();
        return 0;
    }

    case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        int count = self->m_showingSnippets
            ? (int)self->m_filteredSnippets.size()
            : (int)self->m_filtered.size();
        if (delta < 0 && self->m_selected < count - 1) {
            self->m_selected++;
            if (self->m_showingSnippets) self->UpdateSnippetPreview(self->m_selected);
            else self->UpdatePreview(self->m_filtered[self->m_selected]);
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        else if (delta > 0 && self->m_selected > 0) {
            self->m_selected--;
            if (self->m_showingSnippets) self->UpdateSnippetPreview(self->m_selected);
            else self->UpdatePreview(self->m_filtered[self->m_selected]);
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        return 0;
    }

    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE && !self->m_dialogOpen) self->Hide();
        return 0;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) self->Hide();
        return 0;

    case WM_ERASEBKGND:
        return 1;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}


