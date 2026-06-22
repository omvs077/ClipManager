#pragma once
#include "common.h"

class Snippets {
public:
    static std::wstring GetPath();
    static bool Save(const std::vector<Snippet>& snippets);
    static bool Load(std::vector<Snippet>& snippets);
};