#include "snippets.h"
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

std::wstring Snippets::GetPath() {
    wchar_t path[MAX_PATH];
    SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path);
    std::wstring dir = std::wstring(path) + L"\\ClipManager";
    CreateDirectoryW(dir.c_str(), nullptr);
    return dir + L"\\snippets.txt";
}

bool Snippets::Save(const std::vector<Snippet>& snippets) {
    std::wofstream file(GetPath(), std::ios::trunc);
    if (!file.is_open()) return false;
    for (const auto& s : snippets) {
        file << Encode(s.name) << L"|" << Encode(s.text) << L"\n";
    }
    return true;
}

bool Snippets::Load(std::vector<Snippet>& snippets) {
    std::wifstream file(GetPath());
    if (!file.is_open()) return false;
    snippets.clear();
    std::wstring line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        size_t p1 = line.find(L'|');
        if (p1 == std::wstring::npos) continue;
        Snippet s;
        s.name = Decode(line.substr(0, p1));
        s.text = Decode(line.substr(p1 + 1));
        if (!s.name.empty())
            snippets.push_back(s);
    }
    return true;
}