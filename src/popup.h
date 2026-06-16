#pragma once
#include "common.h"

class Popup {
public:
    bool Create(HINSTANCE hInst);
    void Show(const std::vector<ClipEntry>& history);
    void Hide();
    bool IsVisible() const;
    HWND GetHwnd() const { return m_hwnd; }

    std::function<void(int)> OnSelect;
    std::function<void(int)> OnPin;
    std::function<void(int)> OnDelete;

private:
    void PositionNearCursor();
    void PopulateList(const std::wstring& filter = L"");
    void ConfirmSelection();
    void TogglePin();
    void DeleteSelected();
    std::wstring BuildDisplayText(const ClipEntry& entry);

    HWND      m_hwnd   = nullptr;
    HWND      m_list   = nullptr;
    HWND      m_search = nullptr;
    HINSTANCE m_hInst  = nullptr;

    std::vector<ClipEntry> m_history;
    std::vector<int>       m_filtered;

    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    static constexpr wchar_t CLASS_NAME[] = L"ClipManagerPopup";
};