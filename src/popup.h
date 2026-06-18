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
    std::function<void(const std::wstring&)> OnOpenUrl;
    std::function<void(const std::wstring&)> OnOpenPath;

    void SetCompactMode(bool compact)     { m_compactMode = compact; }
    void SetShowTimestamps(bool show)     { m_showTimestamps = show; }

private:
    void PositionNearCursor();
    void PopulateList(const std::wstring& filter = L"");
    void ConfirmSelection();
    void TogglePin();
    void DeleteSelected();
    void UpdatePreview(int historyIndex);
    void PaintLeftPanel(HDC hdc);
    void PaintRightPanel(HDC hdc);
    void HandleQuickAction();
    std::wstring GetTypeName(ClipType type);

    HWND      m_hwnd   = nullptr;
    HWND      m_search = nullptr;
    HINSTANCE m_hInst  = nullptr;

    std::vector<ClipEntry> m_history;
    std::vector<int>       m_filtered;
    int                    m_selected = 0;
    time_t m_previewTimestamp = 0;

    RECT m_quickActionRect = {};

    std::wstring m_searchText;

    std::wstring m_previewText;
    ClipType     m_previewType   = ClipType::Text;
    bool         m_previewPinned = false;
    int          m_previewIndex  = -1;
    bool m_compactMode    = false;
    bool m_showTimestamps = true;

    static constexpr int W        = 880;
    static constexpr int H        = 580;
    static constexpr int LEFT_W   = 340;
    static constexpr int SEARCH_H = 52;
    static constexpr int HINT_H   = 36;
    static constexpr int ITEM_H   = 60;

    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    static LRESULT CALLBACK SearchProc(HWND, UINT, WPARAM, LPARAM,
                                        UINT_PTR, DWORD_PTR);
    static constexpr wchar_t CLASS_NAME[] = L"ClipManagerPopup";
};