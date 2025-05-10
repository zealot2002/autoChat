#pragma once
#include <Windows.h>
#include <string>

// 窗口位置结构
struct WindowPosition {
    int x;
    int y;
    int width;
    int height;
};

// 窗口控制器类，用于操作窗口位置和状态
class WindowsController
{
public:
    WindowsController();
    ~WindowsController();
    
    // 激活窗口并将其置于最前面
    bool ActivateWindow(HWND hwnd);
    
    // 获取窗口位置和大小
    WindowPosition GetWindowPosition(HWND hwnd);
    
    // 设置窗口位置和大小
    bool SetWindowPosition(HWND hwnd, WindowPosition position);
    
    // 使窗口居中显示
    bool CenterWindow(HWND hwnd);
    
    // 最大化窗口
    bool MaximizeWindow(HWND hwnd);
    
    // 最小化窗口
    bool MinimizeWindow(HWND hwnd);
    
    // 恢复窗口
    bool RestoreWindow(HWND hwnd);
    
    // 获取屏幕尺寸
    void GetScreenResolution(int& width, int& height);
    
    // 检查窗口是否处于前台
    bool IsWindowForeground(HWND hwnd);
    
    // 等待窗口激活（前台）
    bool WaitForWindowActive(HWND hwnd, int timeoutMs = 5000);
}; 