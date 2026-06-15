#include "detector.h"
#include <regex>

ClipType Detector::Detect(const std::wstring& text) {
    if (text.empty()) return ClipType::Text;

    // Trim leading/trailing whitespace for matching
    std::wstring t = text;
    size_t start = t.find_first_not_of(L" \t\r\n");
    size_t end   = t.find_last_not_of(L" \t\r\n");
    if (start == std::wstring::npos) return ClipType::Text;
    t = t.substr(start, end - start + 1);

    // Only match single-line content for URL/color/path/email
    bool multiline = (t.find(L'\n') != std::wstring::npos);

    if (!multiline) {
        // Hex color: #RGB or #RRGGBB
        if (std::regex_match(t, std::wregex(L"#([0-9a-fA-F]{3}|[0-9a-fA-F]{6})")))
            return ClipType::Color;

        // Email
        if (std::regex_match(t, std::wregex(L"[\\w._%+\\-]+@[\\w.\\-]+\\.[a-zA-Z]{2,}")))
            return ClipType::Email;

        // URL
        if (std::regex_match(t, std::wregex(L"https?://[^\\s]+")))
            return ClipType::URL;

        // File path: C:\... or \\server\...
        if (std::regex_match(t, std::wregex(L"([a-zA-Z]:\\\\[^\\*\\?\"<>\\|]*|\\\\\\\\[^\\*\\?\"<>\\|]+)")))
            return ClipType::FilePath;
    }

    return ClipType::Text;
}

std::wstring Detector::TypeLabel(ClipType type) {
    switch (type) {
        case ClipType::URL:      return L"[URL]";
        case ClipType::Color:    return L"[COLOR]";
        case ClipType::FilePath: return L"[PATH]";
        case ClipType::Email:    return L"[EMAIL]";
        default:                 return L"";
    }
}