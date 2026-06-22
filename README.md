# ClipManager

A fast, native Windows clipboard manager built in C++/Win32. No Electron, no .NET runtime — just a lightweight tray app.

![ClipManager](resources/screenshot.png)

## Features

- **Clipboard history** — automatically tracks everything you copy: text, images, files/folders
- **Smart detection** — auto-tags URLs, hex colors, file paths, and emails with one-click quick actions (open in browser, open in Explorer, copy hex)
- **Image support** — captures screenshots and copied images with thumbnail previews, auto-cleans orphaned files
- **File & folder support** — copy files in Explorer, paste the real file back later — not just the path
- **Snippets** — save reusable text (signatures, commands, templates) in a dedicated tab, separate from clipboard history
- **Global hotkey** — `Ctrl+Shift+V` opens a searchable two-panel popup near your cursor
- **Pin favorites** — keep important clips at the top forever
- **Privacy controls** — exclude password managers, pause monitoring, clear on exit
- **Auto-delete** — clean up clips older than N days
- **First-run wizard** — quick 3-step setup on first launch
- **System tray** — runs quietly in the background, starts with Windows

## Install

Download the latest `ClipManager_Setup.exe` from [Releases](../../releases) and run it. The installer adds a Start Menu entry, desktop shortcut, and registers ClipManager to launch at login.

## Build from source

Requirements: Visual Studio 2022+ (Desktop C++ workload), CMake 3.20+

```powershell
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

Binary will be at `build\Release\ClipManager.exe`.

## Usage

| Action | Shortcut |
|---|---|
| Open clipboard history | `Ctrl + Shift + V` |
| Paste selected clip | `Enter` |
| Pin / unpin clip | `Ctrl + P` |
| Remove clip | `Ctrl + Del` |
| Close popup | `Esc` |

Right-click the tray icon for Settings, manual history view, or to exit.

## Architecture

```
src/
├── main.cpp        — WinMain, message loop, clipboard orchestration
├── clipboard.cpp    — AddClipboardFormatListener wrapper, text/file-drop read/write
├── popup.cpp        — Two-panel search UI (list + preview), owner-drawn, snippets tab
├── tray.cpp         — Shell_NotifyIcon wrapper, context menu
├── settings.cpp     — Tabbed settings dialog
├── storage.cpp      — Plain-text history persistence
├── detector.cpp     — Regex-based content type detection
├── imaging.cpp      — GDI+ image capture/thumbnail/cleanup
├── snippets.cpp     — Reusable text snippet storage
└── wizard.cpp       — First-run setup wizard
```

Uses `AddClipboardFormatListener` (Vista+) rather than the legacy `SetClipboardViewer` chain, avoiding the classic "one app crashes, clipboard breaks for everyone" bug.

File/folder clips use `CF_HDROP` so pasted items are real files, not just text paths. Images are stored as PNGs in `%APPDATA%\ClipManager\images\`, with automatic orphan cleanup on startup and whenever history is trimmed or cleared.

## Data storage

- History: `%APPDATA%\ClipManager\history.txt`
- Snippets: `%APPDATA%\ClipManager\snippets.txt`
- Images: `%APPDATA%\ClipManager\images\`

Uninstalling removes all of the above.

## License

MIT