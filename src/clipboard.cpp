#include "clipboard.h"
#include <shlobj.h>

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

bool Clipboard::HasFileDrop() {
    return IsClipboardFormatAvailable(CF_HDROP) != FALSE;
}

std::vector<std::wstring> Clipboard::ReadFileDrop() {
    std::vector<std::wstring> result;
    if (!OpenClipboard(nullptr)) return result;

    HANDLE hDrop = GetClipboardData(CF_HDROP);
    if (hDrop) {
        HDROP hDropHandle = (HDROP)hDrop;
        UINT count = DragQueryFileW(hDropHandle, 0xFFFFFFFF, nullptr, 0);
        for (UINT i = 0; i < count; i++) {
            wchar_t path[MAX_PATH] = {};
            DragQueryFileW(hDropHandle, i, path, MAX_PATH);
            result.push_back(path);
        }
    }
    CloseClipboard();
    return result;
}

bool Clipboard::WriteFileDrop(HWND hwnd, const std::vector<std::wstring>& paths) {
    if (paths.empty()) return false;
    if (!OpenClipboard(hwnd)) return false;
    EmptyClipboard();

    size_t totalChars = 1;
    for (const auto& p : paths) totalChars += p.size() + 1;

    size_t dropSize = sizeof(DROPFILES) + totalChars * sizeof(wchar_t);
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dropSize);
    if (!hGlobal) { CloseClipboard(); return false; }

    DROPFILES* df = (DROPFILES*)GlobalLock(hGlobal);
    df->pFiles = sizeof(DROPFILES);
    df->fWide  = TRUE;

    wchar_t* dest = (wchar_t*)((BYTE*)df + sizeof(DROPFILES));
    for (const auto& p : paths) {
        wcscpy_s(dest, p.size() + 1, p.c_str());
        dest += p.size() + 1;
    }
    *dest = L'\0';

    GlobalUnlock(hGlobal);
    SetClipboardData(CF_HDROP, hGlobal);
    CloseClipboard();
    return true;
}