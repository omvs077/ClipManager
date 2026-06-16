#pragma once
#include "common.h"

class Detector {
public:
    static ClipType      Detect(const std::wstring& text);
    static std::wstring  TypeLabel(ClipType type);
};