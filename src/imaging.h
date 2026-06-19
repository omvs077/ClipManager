#pragma once
#include "common.h"

class Imaging {
public:
    static bool HasImage();
    static std::wstring SaveClipboardImage(size_t maxBytes);
    static HBITMAP LoadThumbnail(const std::wstring& path, int maxW, int maxH);
    static std::wstring GetImageDir();
    static void DeleteImage(const std::wstring& path);
    static void SweepOrphans(const std::vector<ClipEntry>& history);
};