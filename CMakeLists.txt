cmake_minimum_required(VERSION 3.20)
project(ClipManager VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
    src/main.cpp
    src/clipboard.cpp
    src/tray.cpp
    src/popup.cpp
    src/storage.cpp
    src/detector.cpp
    src/settings.cpp
    src/imaging.cpp
)

add_executable(ClipManager WIN32 ${SOURCES} resources/resource.rc)

target_link_libraries(ClipManager PRIVATE
    user32 shell32 ole32 gdi32 comctl32 shlwapi comdlg32
)

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
    target_compile_options(ClipManager PRIVATE /W4 /MP)
    target_link_options(ClipManager PRIVATE
    /SUBSYSTEM:WINDOWS
    /MANIFESTUAC:"level='asInvoker' uiAccess='false'"
    )

    target_sources(ClipManager PRIVATE resources/app.manifest)
endif()