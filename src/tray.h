#pragma once
#include "common.h"

class Tray {
public:
    bool Create(HWND hwnd, HINSTANCE hInst);
    void Destroy();
    void ShowContextMenu(HWND hwnd);

private:
    NOTIFYICONDATAW m_nid = {};
};