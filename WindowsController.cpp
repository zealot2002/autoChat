#include "WindowsController.h"
#include <chrono>
#include <thread>

WindowsController::WindowsController()
{
}

WindowsController::~WindowsController()
{
}

bool WindowsController::ActivateWindow(HWND hwnd)
{
    if (!hwnd || !::IsWindow(hwnd))
        return false;
    
    // 如果窗口被最小化，先恢复它
    if (::IsIconic(hwnd))
        ::ShowWindow(hwnd, SW_RESTORE);
    
    // 将窗口设为前台窗口
    ::SetForegroundWindow(hwnd);
    
    // 确保窗口可见
    ::ShowWindow(hwnd, SW_SHOW);
    
    return IsWindowForeground(hwnd);
}

WindowPosition WindowsController::GetWindowPosition(HWND hwnd)
{
    WindowPosition position = { 0, 0, 0, 0 };
    
    if (!hwnd || !::IsWindow(hwnd))
        return position;
    
    RECT rect;
    if (::GetWindowRect(hwnd, &rect))
    {
        position.x = rect.left;
        position.y = rect.top;
        position.width = rect.right - rect.left;
        position.height = rect.bottom - rect.top;
    }
    
    return position;
}

bool WindowsController::SetWindowPosition(HWND hwnd, WindowPosition position)
{
    if (!hwnd || !::IsWindow(hwnd))
        return false;
    
    return ::MoveWindow(hwnd, position.x, position.y, 
                      position.width, position.height, TRUE) != 0;
}

bool WindowsController::CenterWindow(HWND hwnd)
{
    if (!hwnd || !::IsWindow(hwnd))
        return false;
    
    int screenWidth, screenHeight;
    GetScreenResolution(screenWidth, screenHeight);
    
    WindowPosition currentPos = GetWindowPosition(hwnd);
    
    WindowPosition centeredPos = {
        (screenWidth - currentPos.width) / 2,
        (screenHeight - currentPos.height) / 2,
        currentPos.width,
        currentPos.height
    };
    
    return SetWindowPosition(hwnd, centeredPos);
}

bool WindowsController::MaximizeWindow(HWND hwnd)
{
    if (!hwnd || !::IsWindow(hwnd))
        return false;
    
    ::ShowWindow(hwnd, SW_MAXIMIZE);
    return true;
}

bool WindowsController::MinimizeWindow(HWND hwnd)
{
    if (!hwnd || !::IsWindow(hwnd))
        return false;
    
    ::ShowWindow(hwnd, SW_MINIMIZE);
    return true;
}

bool WindowsController::RestoreWindow(HWND hwnd)
{
    if (!hwnd || !::IsWindow(hwnd))
        return false;
    
    ::ShowWindow(hwnd, SW_RESTORE);
    return true;
}

void WindowsController::GetScreenResolution(int& width, int& height)
{
    width = ::GetSystemMetrics(SM_CXSCREEN);
    height = ::GetSystemMetrics(SM_CYSCREEN);
}

bool WindowsController::IsWindowForeground(HWND hwnd)
{
    if (!hwnd || !::IsWindow(hwnd))
        return false;
    
    return hwnd == ::GetForegroundWindow();
}

bool WindowsController::WaitForWindowActive(HWND hwnd, int timeoutMs)
{
    if (!hwnd || !::IsWindow(hwnd))
        return false;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    while (true)
    {
        if (IsWindowForeground(hwnd))
            return true;
        
        // 检查是否超时
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
        
        if (elapsedMs >= timeoutMs)
            break;
        
        // 再次尝试激活窗口
        ActivateWindow(hwnd);
        
        // 等待一段时间再检查
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    return false;
} 