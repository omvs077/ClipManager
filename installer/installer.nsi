!include "MUI2.nsh"

; App info
Name "ClipManager"
OutFile "..\ClipManager_Setup.exe"
InstallDir "$PROGRAMFILES64\ClipManager"
InstallDirRegKey HKLM "Software\ClipManager" "InstallDir"
RequestExecutionLevel admin

; Modern UI settings
!define MUI_ABORTWARNING
!define MUI_ICON "..\resources\icon.ico"
!define MUI_UNICON "..\resources\icon.ico"
!define MUI_WELCOMEPAGE_TITLE "Welcome to ClipManager Setup"
!define MUI_WELCOMEPAGE_TEXT "ClipManager is a lightweight clipboard history manager.$\n$\nPress Win+V to open your clipboard history at any time."
!define MUI_FINISHPAGE_RUN "$INSTDIR\ClipManager.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Launch ClipManager now"

; Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

; Version info
VIProductVersion "1.0.0.0"
VIAddVersionKey "ProductName"     "ClipManager"
VIAddVersionKey "ProductVersion"  "1.0.0"
VIAddVersionKey "FileDescription" "ClipManager Installer"
VIAddVersionKey "LegalCopyright"  "2024"

;-----------------------------------------
Section "Install"
;-----------------------------------------
    SetOutPath "$INSTDIR"

    ; Copy the executable
    File "..\build\Release\ClipManager.exe"

    ; Copy icon
    File "..\resources\icon.ico"

    ; Write install dir to registry
    WriteRegStr HKLM "Software\ClipManager" "InstallDir" "$INSTDIR"

    ; Add to Windows startup (runs on login)
    WriteRegStr HKCU \
        "Software\Microsoft\Windows\CurrentVersion\Run" \
        "ClipManager" \
        '"$INSTDIR\ClipManager.exe"'

    ; Create Start Menu shortcut
    CreateDirectory "$SMPROGRAMS\ClipManager"
    CreateShortcut "$SMPROGRAMS\ClipManager\ClipManager.lnk" \
        "$INSTDIR\ClipManager.exe" "" "$INSTDIR\icon.ico"
    CreateShortcut "$SMPROGRAMS\ClipManager\Uninstall ClipManager.lnk" \
        "$INSTDIR\Uninstall.exe"

    ; Create Desktop shortcut
    CreateShortcut "$DESKTOP\ClipManager.lnk" \
        "$INSTDIR\ClipManager.exe" "" "$INSTDIR\icon.ico"

    ; Write uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"

    ; Add to Programs & Features (Control Panel)
    WriteRegStr HKLM \
        "Software\Microsoft\Windows\CurrentVersion\Uninstall\ClipManager" \
        "DisplayName" "ClipManager"
    WriteRegStr HKLM \
        "Software\Microsoft\Windows\CurrentVersion\Uninstall\ClipManager" \
        "UninstallString" '"$INSTDIR\Uninstall.exe"'
    WriteRegStr HKLM \
        "Software\Microsoft\Windows\CurrentVersion\Uninstall\ClipManager" \
        "DisplayIcon" "$INSTDIR\icon.ico"
    WriteRegStr HKLM \
        "Software\Microsoft\Windows\CurrentVersion\Uninstall\ClipManager" \
        "Publisher" "ClipManager"
    WriteRegStr HKLM \
        "Software\Microsoft\Windows\CurrentVersion\Uninstall\ClipManager" \
        "DisplayVersion" "1.0.0"
    WriteRegDWORD HKLM \
        "Software\Microsoft\Windows\CurrentVersion\Uninstall\ClipManager" \
        "NoModify" 1
    WriteRegDWORD HKLM \
        "Software\Microsoft\Windows\CurrentVersion\Uninstall\ClipManager" \
        "NoRepair" 1
SectionEnd

;-----------------------------------------
Section "Uninstall"
;-----------------------------------------
    ; Kill running instance
    nsExec::Exec 'taskkill /F /IM ClipManager.exe'

    ; Remove files
    Delete "$INSTDIR\ClipManager.exe"
    Delete "$INSTDIR\icon.ico"
    Delete "$INSTDIR\Uninstall.exe"
    RMDir  "$INSTDIR"

    ; Remove Start Menu
    Delete "$SMPROGRAMS\ClipManager\ClipManager.lnk"
    Delete "$SMPROGRAMS\ClipManager\Uninstall ClipManager.lnk"
    RMDir  "$SMPROGRAMS\ClipManager"

    ; Remove Desktop shortcut
    Delete "$DESKTOP\ClipManager.lnk"

    ; Remove startup entry
    DeleteRegValue HKCU \
        "Software\Microsoft\Windows\CurrentVersion\Run" \
        "ClipManager"

    ; Remove registry entries
    DeleteRegKey HKLM "Software\ClipManager"
    DeleteRegKey HKLM \
        "Software\Microsoft\Windows\CurrentVersion\Uninstall\ClipManager"

    ; Remove saved history
    Delete "$APPDATA\ClipManager\history.txt"
    RMDir  "$APPDATA\ClipManager"
SectionEnd