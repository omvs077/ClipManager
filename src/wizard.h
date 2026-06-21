#pragma once
#include "common.h"

class Wizard {
public:
    bool Create(HINSTANCE hInst);
    void Show();

    std::function<void(bool startup, int historyLimit)> OnComplete;

private:
    void ShowPage(int page);
    void NextPage();
    void Finish();

    HWND m_hwnd = nullptr;
    HWND m_pages[3] = {};
    int  m_currentPage = 0;
    HINSTANCE m_hInst = nullptr;
    HFONT m_hFont = nullptr;
    HFONT m_hFontBig = nullptr;

    HWND m_chkStartup = nullptr;
    HWND m_cmbLimit   = nullptr;
    HWND m_btnNext    = nullptr;
    HWND m_btnSkip    = nullptr;

    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    static constexpr wchar_t CLASS_NAME[] = L"ClipManagerWizard";
};

inline bool IsFirstRun() {
    wchar_t path[MAX_PATH];
    SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path);
    std::wstring marker = std::wstring(path) + L"\\ClipManager\\.firstrun_done";
    DWORD attr = GetFileAttributesW(marker.c_str());
    return (attr == INVALID_FILE_ATTRIBUTES);
}

inline void MarkFirstRunDone() {
    wchar_t path[MAX_PATH];
    SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path);
    std::wstring dir = std::wstring(path) + L"\\ClipManager";
    CreateDirectoryW(dir.c_str(), nullptr);
    std::wstring marker = dir + L"\\.firstrun_done";
    HANDLE h = CreateFileW(marker.c_str(), GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN, nullptr);
    if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
}