#pragma once
#include "common.h"

class Popup {
public:
    bool Create(HINSTANCE hInst);
    void Show(const std::vector<ClipEntry>& history);
    void Hide();
    bool IsVisible() const;
    HWND GetHwnd() const { return m_hwnd; }
    void SetSnippets(std::vector<Snippet>* snippets); // pointer, owned by main.cpp

    std::function<void()> OnSnippetsChanged;
    std::function<void(int)> OnSelect;
    std::function<void(int)> OnPin;
    std::function<void(int)> OnDelete;
    std::function<void(const std::wstring&)> OnOpenUrl;
    std::function<void(const std::wstring&)> OnOpenPath;
    std::function<void(const std::wstring&)> OnPasteSnippet;

    void SetCompactMode(bool compact) { m_compactMode = compact; }
    void SetShowTimestamps(bool show) { m_showTimestamps = show; }

private:
    void PositionNearCursor();
    void PopulateList(const std::wstring& filter = L"");
    void PopulateSnippets(const std::wstring& filter = L"");
    void ConfirmSelection();
    void TogglePin();
    void DeleteSelected();
    void UpdatePreview(int historyIndex);
    void UpdateSnippetPreview(int selIdx); // selIdx = index into m_filteredSnippets
    void PaintLeftPanel(HDC hdc);
    void PaintRightPanel(HDC hdc);
    void PaintClipPreview(HDC hdc);
    void PaintSnippetPreview(HDC hdc);
    void PaintSearchBar(HDC hdc, const std::wstring& text);
    void HandleQuickAction();
    std::wstring GetTypeName(ClipType type);

    std::vector<Snippet>* m_snippets = nullptr;
    std::vector<int>      m_filteredSnippets;
    std::wstring           m_snippetSearchText;
    bool m_showingSnippets = false;
    bool m_dialogOpen = false;
    void ToggleSnippetsView();
    void PaintSnippetTab(HDC hdc);
    void SnippetEditor(int editIndex, const std::wstring& prefillText = L"");
    void DeleteSnippetSelected();

    std::wstring m_previewSnippetName;
    std::wstring m_previewSnippetText;
    int          m_previewSnippetRealIndex = -1;

    HWND      m_hwnd = nullptr;
    HWND      m_search = nullptr;
    HINSTANCE m_hInst = nullptr;

    std::vector<ClipEntry> m_history;
    std::vector<int>       m_filtered;
    std::vector<std::wstring> m_previewFilePaths;
    int                    m_selected = 0;

    std::wstring m_searchText;

    std::wstring m_previewText;
    ClipType     m_previewType = ClipType::Text;
    bool         m_previewPinned = false;
    int          m_previewIndex = -1;
    time_t       m_previewTimestamp = 0;
    std::wstring m_previewImagePath;

    bool m_compactMode = false;
    bool m_showTimestamps = true;

    RECT m_quickActionRect = {};

    static constexpr int W = 880;
    static constexpr int H = 580;
    static constexpr int LEFT_W = 340;
    static constexpr int SEARCH_H = 52;
    static constexpr int HINT_H = 36;
    static constexpr int ITEM_H = 60;
    static constexpr int TAB_STRIP_H = 36;
    static constexpr int HEADER_H = TAB_STRIP_H + SEARCH_H;

    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    static LRESULT CALLBACK SearchProc(HWND, UINT, WPARAM, LPARAM,
        UINT_PTR, DWORD_PTR);
    static constexpr wchar_t CLASS_NAME[] = L"ClipManagerPopup";
};