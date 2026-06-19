#include "imaging.h"
#include <shlobj.h>
#include <gdiplus.h>
#include <objidl.h>
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

static ULONG_PTR g_gdiToken = 0;

static void EnsureGdiPlus() {
    if (g_gdiToken) return;
    GdiplusStartupInput input;
    GdiplusStartup(&g_gdiToken, &input, nullptr);
}

std::wstring Imaging::GetImageDir() {
    wchar_t path[MAX_PATH];
    SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path);
    std::wstring dir = std::wstring(path) + L"\\ClipManager\\images";
    CreateDirectoryW((std::wstring(path) + L"\\ClipManager").c_str(), nullptr);
    CreateDirectoryW(dir.c_str(), nullptr);
    return dir;
}

bool Imaging::HasImage() {
    return IsClipboardFormatAvailable(CF_DIB) ||
           IsClipboardFormatAvailable(CF_BITMAP);
}

static bool GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0, size = 0;
    GetImageEncodersSize(&num, &size);
    if (size == 0) return false;

    std::vector<BYTE> buffer(size);
    ImageCodecInfo* info = (ImageCodecInfo*)buffer.data();
    GetImageEncoders(num, size, info);

    for (UINT i = 0; i < num; i++) {
        if (wcscmp(info[i].MimeType, format) == 0) {
            *pClsid = info[i].Clsid;
            return true;
        }
    }
    return false;
}

std::wstring Imaging::SaveClipboardImage(size_t maxBytes) {
    EnsureGdiPlus();

    if (!OpenClipboard(nullptr)) return L"";

    HBITMAP hBitmap = nullptr;
    HANDLE hDib = GetClipboardData(CF_DIB);
    if (hDib) {
        BITMAPINFO* bmi = (BITMAPINFO*)GlobalLock(hDib);
        if (bmi) {
            HDC hdc = GetDC(nullptr);
            hBitmap = CreateDIBitmap(hdc, &bmi->bmiHeader, CBM_INIT,
                (BYTE*)bmi + bmi->bmiHeader.biSize,
                bmi, DIB_RGB_COLORS);
            ReleaseDC(nullptr, hdc);
            GlobalUnlock(hDib);
        }
    }
    CloseClipboard();

    if (!hBitmap) return L"";

    Bitmap bmp(hBitmap, nullptr);
    DeleteObject(hBitmap);

    if (bmp.GetWidth() == 0 || bmp.GetHeight() == 0) return L"";

    wchar_t filename[64];
    swprintf_s(filename, L"clip_%lld.png", (long long)time(nullptr));
    std::wstring fullPath = GetImageDir() + L"\\" + filename;

    CLSID pngClsid;
    if (!GetEncoderClsid(L"image/png", &pngClsid)) return L"";

    Status st = bmp.Save(fullPath.c_str(), &pngClsid, nullptr);
    if (st != Ok) return L"";

    HANDLE hFile = CreateFileW(fullPath.c_str(), GENERIC_READ, FILE_SHARE_READ,
        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER size;
        GetFileSizeEx(hFile, &size);
        CloseHandle(hFile);
        if ((size_t)size.QuadPart > maxBytes) {
            DeleteFileW(fullPath.c_str());
            return L"";
        }
    }

    return fullPath;
}

HBITMAP Imaging::LoadThumbnail(const std::wstring& path, int maxW, int maxH) {
    EnsureGdiPlus();
    if (path.empty()) return nullptr;

    Bitmap src(path.c_str());
    if (src.GetWidth() == 0) return nullptr;

    double scale = std::min((double)maxW / src.GetWidth(),
                             (double)maxH / src.GetHeight());
    int w = (int)(src.GetWidth()  * scale);
    int h = (int)(src.GetHeight() * scale);
    if (w < 1) w = 1;
    if (h < 1) h = 1;

    Bitmap thumb(w, h, PixelFormat32bppARGB);
    Graphics g(&thumb);
    g.SetInterpolationMode(InterpolationModeHighQualityBicubic);
    g.DrawImage(&src, 0, 0, w, h);

    HBITMAP hBitmap = nullptr;
    thumb.GetHBITMAP(Color(255,255,255), &hBitmap);
    return hBitmap;
}

void Imaging::DeleteImage(const std::wstring& path) {
    if (!path.empty())
        DeleteFileW(path.c_str());
}

void Imaging::SweepOrphans(const std::vector<ClipEntry>& history) {
    std::wstring dir = GetImageDir();
    std::wstring pattern = dir + L"\\*.png";

    WIN32_FIND_DATAW fd;
    HANDLE hFind = FindFirstFileW(pattern.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        std::wstring fullPath = dir + L"\\" + fd.cFileName;
        bool referenced = false;
        for (const auto& e : history) {
            if (e.imagePath == fullPath) { referenced = true; break; }
        }
        if (!referenced)
            DeleteFileW(fullPath.c_str());
    } while (FindNextFileW(hFind, &fd));

    FindClose(hFind);
}