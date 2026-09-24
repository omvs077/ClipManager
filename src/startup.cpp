#include "startup.h"
#include <appmodel.h>
#pragma comment(lib, "kernel32.lib")

// C++/WinRT is header-only and ships with the Windows SDK -- no extra
// dependency to install. Only pulled in for the packaged (MSIX) path.
#include <winrt/base.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.ApplicationModel.h>
#pragma comment(lib, "windowsapp.lib")

using namespace winrt::Windows::ApplicationModel;

static const wchar_t* kRunKeyPath =
    L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
static const wchar_t* kRunValueName = L"ClipManager";

void Startup::InitApartment() {
    winrt::init_apartment(winrt::apartment_type::single_threaded);
}

bool Startup::IsPackaged() {
    UINT32 len = 0;
    LONG rc = GetCurrentPackageFullName(&len, nullptr);
    // APPMODEL_ERROR_NO_PACKAGE means "running unpackaged" -- that's the
    // expected, non-error case for the current NSIS/portable build.
    return rc != APPMODEL_ERROR_NO_PACKAGE;
}

static void SetEnabledUnpackaged(bool enable) {
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kRunKeyPath, 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS)
        return;
    if (enable) {
        std::wstring val = L"\"" + std::wstring(exePath) + L"\"";
        RegSetValueExW(hKey, kRunValueName, 0, REG_SZ,
            (const BYTE*)val.c_str(), (DWORD)((val.size() + 1) * sizeof(wchar_t)));
    } else {
        RegDeleteValueW(hKey, kRunValueName);
    }
    RegCloseKey(hKey);
}

static bool IsEnabledUnpackaged() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, kRunKeyPath, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
        return false;
    DWORD type = 0, size = 0;
    LONG rc = RegQueryValueExW(hKey, kRunValueName, nullptr, &type, nullptr, &size);
    RegCloseKey(hKey);
    return rc == ERROR_SUCCESS;
}

static void SetEnabledPackaged(bool enable) {
    try {
        auto task = StartupTask::GetAsync(Startup::kStartupTaskId).get();
        if (enable) {
            auto state = task.RequestEnableAsync().get();
            // DisabledByUser means the user turned it off in Task Manager's
            // Startup Apps tab -- Windows deliberately gives no programmatic
            // override for that. Nothing more we can/should do here.
            (void)state;
        } else {
            task.Disable();
        }
    } catch (...) {
        // Startup-task registration is a convenience, not core functionality --
        // never let a WinRT failure here take down the app.
    }
}

static bool IsEnabledPackaged() {
    try {
        auto task = StartupTask::GetAsync(Startup::kStartupTaskId).get();
        return task.State() == StartupTaskState::Enabled;
    } catch (...) {
        return false;
    }
}

void Startup::SetEnabled(bool enable) {
    if (IsPackaged()) SetEnabledPackaged(enable);
    else              SetEnabledUnpackaged(enable);
}

bool Startup::IsEnabled() {
    return IsPackaged() ? IsEnabledPackaged() : IsEnabledUnpackaged();
}


