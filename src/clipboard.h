#pragma once
#include "common.h"

class Clipboard {
public:
    static bool StartListening(HWND hwnd);
    static void StopListening(HWND hwnd);
    static std::wstring ReadText();
    static bool WriteText(HWND hwnd, const std::wstring& text);
    static bool WritePlainText(HWND hwnd, const std::wstring& text);
};