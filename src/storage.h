#pragma once
#include "common.h"

class Storage {
public:
    static std::wstring GetAppDataPath();
    static bool SaveHistory(const std::vector<ClipEntry>& history);
    static bool LoadHistory(std::vector<ClipEntry>& history);
};