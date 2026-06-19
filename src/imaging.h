#pragma once
#include "common.h"

class Imaging {
public:
    // Returns true if clipboard currently holds a bitmap image
    static bool HasImage();

    // Saves the current clipboard image as a PNG to AppData,
    // returns the saved file path, or empty string on failure/too large
    static std::wstring SaveClipboardImage(size_t maxBytes);

    // Loads a PNG from disk into an HBITMAP for rendering (caller must DeleteObject)
    static HBITMAP LoadThumbnail(const std::wstring& path, int maxW, int maxH);

    static std::wstring GetImageDir();
};