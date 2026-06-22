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

        // Encode filePaths as semicolon-joined, then escape
        std::wstring filesJoined;
        for (size_t i = 0; i < entry.filePaths.size(); i++) {
            if (i > 0) filesJoined += L";;";
            filesJoined += entry.filePaths[i];
        }

        file << pin << L"|" << type << L"|" << entry.timestamp << L"|"
             << Encode(entry.imagePath) << L"|"
             << Encode(filesJoined) << L"|"
             << Encode(entry.text) << L"\n";
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
        entry.type   = static_cast<ClipType>(std::stoi(line.substr(p1+1, p2-p1-1)));

        size_t p3 = line.find(L'|', p2 + 1);
        if (p3 == std::wstring::npos) {
            entry.timestamp = time(nullptr);
            entry.text = Decode(line.substr(p2 + 1));
            history.push_back(entry);
            continue;
        }

        size_t p4 = line.find(L'|', p3 + 1);
        if (p4 == std::wstring::npos) {
            entry.timestamp = static_cast<time_t>(std::stoll(line.substr(p2+1, p3-p2-1)));
            entry.text = Decode(line.substr(p3 + 1));
            history.push_back(entry);
            continue;
        }

        size_t p5 = line.find(L'|', p4 + 1);
        if (p5 == std::wstring::npos) {
            // Format: pin|type|timestamp|imagePath|text
            entry.timestamp = static_cast<time_t>(std::stoll(line.substr(p2+1, p3-p2-1)));
            entry.imagePath = Decode(line.substr(p3+1, p4-p3-1));
            entry.text = Decode(line.substr(p4 + 1));
            history.push_back(entry);
            continue;
        }

        // Newest format: pin|type|timestamp|imagePath|filePaths|text
        entry.timestamp = static_cast<time_t>(std::stoll(line.substr(p2+1, p3-p2-1)));
        entry.imagePath = Decode(line.substr(p3+1, p4-p3-1));

        std::wstring filesJoined = Decode(line.substr(p4+1, p5-p4-1));
        if (!filesJoined.empty()) {
            size_t pos = 0;
            while (true) {
                size_t sep = filesJoined.find(L";;", pos);
                if (sep == std::wstring::npos) {
                    entry.filePaths.push_back(filesJoined.substr(pos));
                    break;
                }
                entry.filePaths.push_back(filesJoined.substr(pos, sep - pos));
                pos = sep + 2;
            }
        }

        entry.text = Decode(line.substr(p5 + 1));

        if (!entry.text.empty() || !entry.imagePath.empty() || !entry.filePaths.empty())
            history.push_back(entry);
    }
    return true;
}