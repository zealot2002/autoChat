#include "AppFinder.h"
#include <algorithm>
#include <chrono>
#include <thread>
#include <locale>
#include <codecvt>

AppFinder::AppFinder()
{
}

AppFinder::~AppFinder()
{
}

HWND AppFinder::FindWindowByName(const std::wstring& windowName, bool exactMatch)
{
    std::vector<WindowInfo> windows = EnumerateWindows();
    
    for (const auto& window : windows)
    {
        if (exactMatch)
        {
            if (window.title == windowName)
                return window.handle;
        }
        else
        {
            // 将标题和查找字符串都转为小写进行比较
            std::wstring titleLower = window.title;
            std::wstring searchLower = windowName;
            
            std::transform(titleLower.begin(), titleLower.end(), titleLower.begin(), ::tolower);
            std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
            
            if (titleLower.find(searchLower) != std::wstring::npos)
                return window.handle;
        }
    }
    
    return NULL; // 未找到窗口
}

HWND AppFinder::FindWindowByClass(const std::wstring& className)
{
    std::vector<WindowInfo> windows = EnumerateWindows();
    
    for (const auto& window : windows)
    {
        if (window.className == className)
            return window.handle;
    }
    
    return NULL; // 未找到窗口
}

std::vector<WindowInfo> AppFinder::EnumerateWindows()
{
    std::vector<WindowInfo> windows;
    EnumWindowsData data = { &windows, true }; // 只获取可见窗口
    
    ::EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&data));
    
    return windows;
}

BOOL CALLBACK AppFinder::EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    EnumWindowsData* data = reinterpret_cast<EnumWindowsData*>(lParam);
    
    // 检查窗口是否可见（如果要求只获取可见窗口）
    if (data->visibleOnly && !::IsWindowVisible(hwnd))
        return TRUE; // 继续枚举
    
    // 获取窗口标题
    WCHAR title[256] = { 0 };
    ::GetWindowTextW(hwnd, title, 256);
    
    // 获取窗口类名
    WCHAR className[256] = { 0 };
    ::GetClassNameW(hwnd, className, 256);
    
    // 忽略没有标题的窗口
    if (title[0] != L'\0')
    {
        WindowInfo info = { hwnd, title, className };
        data->windows->push_back(info);
    }
    
    return TRUE; // 继续枚举
}

std::wstring AppFinder::GetWindowTitle(HWND hwnd)
{
    if (!hwnd)
        return L"";
        
    WCHAR title[256] = { 0 };
    ::GetWindowTextW(hwnd, title, 256);
    return title;
}

std::wstring AppFinder::GetWindowClassName(HWND hwnd)
{
    if (!hwnd)
        return L"";
        
    WCHAR className[256] = { 0 };
    ::GetClassNameW(hwnd, className, 256);
    return className;
}

bool AppFinder::IsWindowVisible(HWND hwnd)
{
    if (!hwnd)
        return false;
        
    return ::IsWindowVisible(hwnd) != 0;
}

HWND AppFinder::WaitForWindow(const std::wstring& windowName, int timeoutMs)
{
    auto startTime = std::chrono::high_resolution_clock::now();
    HWND hwnd = NULL;
    
    while (true)
    {
        // 尝试查找窗口
        hwnd = FindWindowByName(windowName, false);
        
        if (hwnd != NULL)
            return hwnd;
            
        // 检查是否超时
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
        
        if (elapsedMs >= timeoutMs)
            break;
            
        // 等待后再次尝试
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    return NULL; // 超时仍未找到窗口
}
