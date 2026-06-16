#pragma once
#include "common.h"
#include <commctrl.h>

enum class AppTheme { System, Light, Dark };

struct AppSettings {
    // General
    bool startWithWindows  = true;
    bool minimizeToTray    = true;
    bool showNotifications = false;

    // History
    int  historyLimit      = 500;   // -1 = unlimited
    bool ignoreDuplicates  = true;
    bool saveImages        = true;
    bool saveFiles         = true;
    int  autoDeleteDays    = 30;    // -1 = never

    // Hotkeys (stored as strings for display)
    std::wstring hotkeyMain   = L"Win+V";
    std::wstring hotkeyLatest = L"Ctrl+Alt+V";

    // Appearance
    AppTheme theme       = AppTheme::System;
    bool     compactMode = false;
    bool     showTimestamps = true;

    // Privacy
    bool pauseMonitoring    = false;
    bool excludePasswords   = true;
    bool clearOnExit        = false;
};

class Settings {
public:
    bool Create(HINSTANCE hInst);
    void Show();
    void Hide();
    bool IsVisible() const;

    AppSettings Current;

    std::function<void(const AppSettings&)> OnSave;
    std::function<void()>                   OnClearHistory;
    std::function<void(bool)>               OnPauseToggle;

private:
    void BuildTabs();
    void ShowTab(int index);
    void SaveAndClose();
    void ApplyStartup(bool enable);
    void PopulateControls();

    // Tab panels
    HWND CreateTabGeneral(RECT rc);
    HWND CreateTabHistory(RECT rc);
    HWND CreateTabHotkeys(RECT rc);
    HWND CreateTabAppearance(RECT rc);
    HWND CreateTabPrivacy(RECT rc);

    HWND MakeLabel(HWND parent, const wchar_t* text, int x, int y, int w, int h);
    HWND MakeCheck(HWND parent, const wchar_t* text, int id, int x, int y, int w, int h);
    HWND MakeCombo(HWND parent, int id, int x, int y, int w, int h);
    HWND MakeButton(HWND parent, const wchar_t* text, int id, int x, int y, int w, int h);
    HWND MakeHotkeyBox(HWND parent, int id, int x, int y, int w, int h);

    HWND m_hwnd   = nullptr;
    HWND m_tabs   = nullptr;
    HWND m_panels[5] = {};
    int  m_activeTab  = 0;
    HINSTANCE m_hInst = nullptr;
    HFONT m_hFont     = nullptr;
    HFONT m_hFontBold = nullptr;
    HFONT m_hFontSm   = nullptr;

    // General tab controls
    HWND m_chkStartup = nullptr;
    HWND m_chkMinTray = nullptr;
    HWND m_chkNotify  = nullptr;

    // History tab controls
    HWND m_cmbLimit      = nullptr;
    HWND m_chkDupes      = nullptr;
    HWND m_chkImages     = nullptr;
    HWND m_chkFiles      = nullptr;
    HWND m_cmbAutoDel    = nullptr;
    HWND m_btnClear      = nullptr;

    // Appearance tab controls
    HWND m_cmbTheme      = nullptr;
    HWND m_chkCompact    = nullptr;
    HWND m_chkTimestamps = nullptr;

    // Privacy tab controls
    HWND m_chkPause      = nullptr;
    HWND m_chkExclPwd    = nullptr;
    HWND m_chkClearExit  = nullptr;

    // Hotkey tab controls
    HWND m_hkMain   = nullptr;
    HWND m_hkLatest = nullptr;

    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    static LRESULT CALLBACK PanelProc(HWND, UINT, WPARAM, LPARAM);
    static constexpr wchar_t CLASS_NAME[]  = L"ClipManagerSettings";
    static constexpr wchar_t PANEL_CLASS[] = L"ClipManagerPanel";
};