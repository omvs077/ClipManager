#pragma once
#include "common.h"

struct AppSettings {
    int  historyLimit  = 50;
    bool startWithWindows = true;
    bool darkMode      = true; // auto-follows system if not overridden
};

class Settings {
public:
    bool Create(HINSTANCE hInst);
    void Show();
    void Hide();
    bool IsVisible() const;

    AppSettings Current;

    std::function<void(const AppSettings&)> OnSave;

private:
    void PopulateControls();
    void SaveAndClose();
    void ApplyStartup(bool enable);

    HWND m_hwnd      = nullptr;
    HWND m_sliderLimit = nullptr;
    HWND m_labelLimit  = nullptr;
    HWND m_chkStartup  = nullptr;
    HWND m_btnSave     = nullptr;
    HWND m_btnClear    = nullptr;
    HINSTANCE m_hInst  = nullptr;

    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
    static constexpr wchar_t CLASS_NAME[] = L"ClipManagerSettings";
};