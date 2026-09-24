#pragma once
#include "common.h"

// Single source of truth for "launch ClipManager when Windows starts".
// Chooses the correct mechanism at runtime:
//   - Unpackaged (current NSIS/portable build): HKCU\...\Run registry key.
//   - Packaged (MSIX): the StartupTask WinRT API. The startup task ID here
//     MUST match the Id on the <uap5:StartupTask> extension declared in
//     Package.appxmanifest once the MSIX packaging project exists.
namespace Startup {
    void InitApartment();
    constexpr wchar_t kStartupTaskId[] = L"ClipManagerStartupId";

    bool IsPackaged();
    void SetEnabled(bool enable);
    bool IsEnabled();
}

