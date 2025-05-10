#pragma once
#include <Windows.h>
#include <string>
#include <vector>

// 窗口信息结构
struct WindowInfo {
    HWND handle;            // 窗口句柄
    std::wstring title;     // 窗口标题
    std::wstring className; // 窗口类名
};

// 应用窗口查找器类
class AppFinder
{
public:
    AppFinder();
    ~AppFinder();

    // 通过应用名称查找窗口（支持模糊匹配）
    HWND FindWindowByName(const std::wstring& windowName, bool exactMatch = false);
    
    // 通过窗口类名查找窗口
    HWND FindWindowByClass(const std::wstring& className);
    
    // 枚举所有顶级窗口
    std::vector<WindowInfo> EnumerateWindows();
    
    // 获取窗口标题
    std::wstring GetWindowTitle(HWND hwnd);
    
    // 获取窗口类名
    std::wstring GetWindowClassName(HWND hwnd);
    
    // 检查窗口是否可见
    bool IsWindowVisible(HWND hwnd);
    
    // 带超时的窗口查找（在指定时间内等待窗口出现）
    HWND WaitForWindow(const std::wstring& windowName, int timeoutMs = 5000);

private:
    // EnumWindows回调函数所需的内部数据结构
    struct EnumWindowsData {
        std::vector<WindowInfo>* windows;
        bool visibleOnly;
    };

    // EnumWindows回调函数
    static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
};

