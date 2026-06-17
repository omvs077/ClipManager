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
    void UpdatePreview(int historyIndex);
    void PaintLeftPanel(HDC hdc);
    void PaintRightPanel(HDC hdc);
    std::wstring GetTypeName(ClipType type);

    HWND      m_hwnd   = nullptr;
    HWND      m_search = nullptr;
    HINSTANCE m_hInst  = nullptr;

    std::vector<ClipEntry> m_history;
    std::vector<int>       m_filtered;
    int                    m_selected = 0;
    time_t m_previewTimestamp = 0;

    std::wstring m_searchText;

    std::wstring m_previewText;
    ClipType     m_previewType   = ClipType::Text;
    bool         m_previewPinned = false;
    int          m_previewIndex  = -1;

    static constexpr int W        = 820;
    static constexpr int H        = 540;
    static constexpr int LEFT_W   = 340;
    static constexpr int SEARCH_H = 52;
    static constexpr int HINT_H   = 36;
    static constexpr int ITEM_H   = 52;

    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    static LRESULT CALLBACK SearchProc(HWND, UINT, WPARAM, LPARAM,
                                        UINT_PTR, DWORD_PTR);
    static constexpr wchar_t CLASS_NAME[] = L"ClipManagerPopup";
};