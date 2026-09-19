cmake_minimum_required(VERSION 3.20)
project(ClipManager VERSION 1.1.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Statically link the MSVC runtime so the app has zero dependency on
# vcredist being present on the target machine (certification VMs are
# clean machines and will otherwise fail with a missing VCRUNTIME140.dll).
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(SOURCES
    src/main.cpp
    src/clipboard.cpp
    src/tray.cpp
    src/popup.cpp
    src/storage.cpp
    src/detector.cpp
    src/settings.cpp
    src/imaging.cpp
    src/wizard.cpp
    src/snippets.cpp
)

add_executable(ClipManager WIN32 ${SOURCES} resources/resource.rc)

target_link_libraries(ClipManager PRIVATE
    user32 shell32 ole32 gdi32 comctl32 shlwapi comdlg32 gdiplus
)

target_include_directories(ClipManager PRIVATE src)

target_compile_definitions(ClipManager PRIVATE
    WINVER=0x0601
    _WIN32_WINNT=0x0601
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

if(MSVC)
    target_compile_options(ClipManager PRIVATE /W4)
    target_link_options(ClipManager PRIVATE /SUBSYSTEM:WINDOWS)
    # Merges PerMonitorV2 DPI-awareness into the linker's auto-generated
    # manifest -- no manual manifest embedding, no /MANIFEST:NO needed,
    # so there's no collision risk with the default automatic manifest.
    set_target_properties(ClipManager PROPERTIES VS_DPI_AWARE "PerMonitor")
endif()

