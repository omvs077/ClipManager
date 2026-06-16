#include "storage.h"
#include <shlobj.h>

static std::wstring Encode(const std::wstring& s) {
    std::wstring r;
    for (wchar_t c : s) {
        if      (c == L'\n') r += L"\\n";
        else if (c == L'\r') r += L"\\r";
        else if (c == L'\\') r += L"\\\\";
        else if (c == L'|')  r += L"\\p";
        else r += c;
    }
    return r;
}

static std::wstring Decode(const std::wstring& s) {
    std::wstring r;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == L'\\' && i + 1 < s.size()) {
            wchar_t n = s[i+1];
            if      (n == L'n')  { r += L'\n'; i++; }
            else if (n == L'r')  { r += L'\r'; i++; }
            else if (n == L'\\') { r += L'\\'; i++; }
            else if (n == L'p')  { r += L'|';  i++; }
            else r += s[i];
        } else {
            r += s[i];
        }
    }
    return r;
}

std::wstring Storage::GetAppDataPath() {
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path))) {
        std::wstring dir = std::wstring(path) + L"\\ClipManager";
        CreateDirectoryW(dir.c_str(), nullptr);
        return dir + L"\\history.txt";
    }
    return L"history.txt";
}

bool Storage::SaveHistory(const std::vector<ClipEntry>& history) {
    std::wofstream file(GetAppDataPath(), std::ios::trunc);
    if (!file.is_open()) return false;
    for (const ClipEntry& entry : history) {
        int pin  = entry.pinned ? 1 : 0;
        int type = static_cast<int>(entry.type);
        file << pin << L"|" << type << L"|" << Encode(entry.text) << L"\n";
    }
    return true;
}

bool Storage::LoadHistory(std::vector<ClipEntry>& history) {
    std::wifstream file(GetAppDataPath());
    if (!file.is_open()) return false;
    history.clear();
    std::wstring line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        size_t p1 = line.find(L'|');
        size_t p2 = line.find(L'|', p1 + 1);
        if (p1 == std::wstring::npos || p2 == std::wstring::npos) continue;
        ClipEntry entry;
        entry.pinned = (line.substr(0, p1) == L"1");
        entry.type   = static_cast<ClipType>(std::stoi(line.substr(p1 + 1, p2 - p1 - 1)));
        entry.text   = Decode(line.substr(p2 + 1));
        if (!entry.text.empty())
            history.push_back(entry);
    }
    return true;
}