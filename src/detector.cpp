#include "detector.h"
#include <regex>

ClipType Detector::Detect(const std::wstring& text) {
    if (text.empty()) return ClipType::Text;

    std::wstring t = text;
    size_t s = t.find_first_not_of(L" \t\r\n");
    size_t e = t.find_last_not_of(L" \t\r\n");
    if (s == std::wstring::npos) return ClipType::Text;
    t = t.substr(s, e - s + 1);

    bool multiline = (t.find(L'\n') != std::wstring::npos);
    if (!multiline) {
        if (std::regex_match(t, std::wregex(L"#([0-9a-fA-F]{3}|[0-9a-fA-F]{6})")))
            return ClipType::Color;
        if (std::regex_match(t, std::wregex(L"[\\w._%+\\-]+@[\\w.\\-]+\\.[a-zA-Z]{2,}")))
            return ClipType::Email;
        if (std::regex_match(t, std::wregex(L"https?://[^\\s]+")))
            return ClipType::URL;
        if (std::regex_match(t, std::wregex(L"[a-zA-Z]:\\\\.*")))
            return ClipType::FilePath;
    }
    return ClipType::Text;
}

std::wstring Detector::TypeLabel(ClipType type) {
    switch (type) {
        case ClipType::URL:      return L"[URL] ";
        case ClipType::Color:    return L"[COLOR] ";
        case ClipType::FilePath: return L"[PATH] ";
        case ClipType::Email:    return L"[EMAIL] ";
        default:                 return L"";
    }
}