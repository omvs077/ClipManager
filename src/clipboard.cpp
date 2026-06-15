#include "clipboard.h"

bool Clipboard::StartListening(HWND hwnd) {
    return AddClipboardFormatListener(hwnd) != FALSE;
}

void Clipboard::StopListening(HWND hwnd) {
    RemoveClipboardFormatListener(hwnd);
}

std::wstring Clipboard::ReadText() {
    if (!IsClipboardFormatAvailable(CF_UNICODETEXT)) return L"";
    if (!OpenClipboard(nullptr)) return L"";

    std::wstring result;
    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData) {
        wchar_t* p = static_cast<wchar_t*>(GlobalLock(hData));
        if (p) { result = p; GlobalUnlock(hData); }
    }
    CloseClipboard();
    return result;
}

bool Clipboard::WriteText(HWND hwnd, const std::wstring& text) {
    if (!OpenClipboard(hwnd)) return false;
    EmptyClipboard();

    size_t bytes = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (!hMem) { CloseClipboard(); return false; }

    wchar_t* p = static_cast<wchar_t*>(GlobalLock(hMem));
    if (p) { memcpy(p, text.c_str(), bytes); GlobalUnlock(hMem); }

    SetClipboardData(CF_UNICODETEXT, hMem);
    CloseClipboard();
    return true;
}

// Plain text paste: strips to CF_UNICODETEXT only (no RTF/HTML formats)
bool Clipboard::WritePlainText(HWND hwnd, const std::wstring& text) {
    return WriteText(hwnd, text); // CF_UNICODETEXT is already plain
    // The key is: we do NOT write CF_RTF or CF_HTML alongside it
}