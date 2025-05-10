#include "InputSimulator.h"
#include <thread>

InputSimulator::InputSimulator()
{
}

InputSimulator::~InputSimulator()
{
}

bool InputSimulator::SendKey(WORD vkCode, bool isExtended)
{
    // 模拟按键按下再释放
    if (!SendKeyDown(vkCode, isExtended))
        return false;
    
    Sleep(DEFAULT_DELAY);
    
    if (!SendKeyUp(vkCode, isExtended))
        return false;
    
    return true;
}

bool InputSimulator::SendKeyCombo(const std::vector<WORD>& vkCodes)
{
    // 首先按下所有键
    for (const auto& vkCode : vkCodes)
    {
        if (!SendKeyDown(vkCode))
            return false;
        
        Sleep(DEFAULT_DELAY / 2);
    }
    
    Sleep(DEFAULT_DELAY);
    
    // 然后以相反顺序释放所有键
    for (auto it = vkCodes.rbegin(); it != vkCodes.rend(); ++it)
    {
        if (!SendKeyUp(*it))
            return false;
        
        Sleep(DEFAULT_DELAY / 2);
    }
    
    return true;
}

bool InputSimulator::SendText(const std::wstring& text)
{
    // 使用SetClipboardText和PasteText更高效地输入文本
    if (!SetClipboardText(text))
        return false;
    
    return PasteText();
}

bool InputSimulator::ClickLeftMouse(int x, int y)
{
    // 首先移动鼠标到指定位置
    if (!MoveMouse(x, y))
        return false;
    
    // 准备鼠标按下事件
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    ::SendInput(1, &input, sizeof(INPUT));
    
    Sleep(DEFAULT_DELAY);
    
    // 准备鼠标释放事件
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    ::SendInput(1, &input, sizeof(INPUT));
    
    return true;
}

bool InputSimulator::ClickRightMouse(int x, int y)
{
    // 首先移动鼠标到指定位置
    if (!MoveMouse(x, y))
        return false;
    
    // 准备鼠标按下事件
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
    ::SendInput(1, &input, sizeof(INPUT));
    
    Sleep(DEFAULT_DELAY);
    
    // 准备鼠标释放事件
    input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
    ::SendInput(1, &input, sizeof(INPUT));
    
    return true;
}

bool InputSimulator::DoubleClickMouse(int x, int y)
{
    // 执行第一次点击
    if (!ClickLeftMouse(x, y))
        return false;
    
    Sleep(DEFAULT_DELAY);
    
    // 执行第二次点击
    return ClickLeftMouse(x, y);
}

bool InputSimulator::MoveMouse(int x, int y)
{
    // 计算相对于屏幕的坐标
    double screenWidth = ::GetSystemMetrics(SM_CXSCREEN) - 1;
    double screenHeight = ::GetSystemMetrics(SM_CYSCREEN) - 1;
    
    // 准备鼠标移动事件
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dx = static_cast<LONG>((x * 65535) / screenWidth);
    input.mi.dy = static_cast<LONG>((y * 65535) / screenHeight);
    input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
    
    return ::SendInput(1, &input, sizeof(INPUT)) == 1;
}

bool InputSimulator::SetClipboardText(const std::wstring& text)
{
    // 打开剪贴板
    if (!::OpenClipboard(NULL))
        return false;
    
    // 清空剪贴板
    if (!::EmptyClipboard())
    {
        ::CloseClipboard();
        return false;
    }
    
    // 分配全局内存
    size_t size = (text.length() + 1) * sizeof(wchar_t);
    HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, size);
    if (!hGlobal)
    {
        ::CloseClipboard();
        return false;
    }
    
    // 复制文本到全局内存
    wchar_t* pGlobal = static_cast<wchar_t*>(::GlobalLock(hGlobal));
    if (pGlobal)
    {
        wcscpy_s(pGlobal, text.length() + 1, text.c_str());
        ::GlobalUnlock(hGlobal);
        
        // 设置剪贴板数据
        HANDLE hResult = ::SetClipboardData(CF_UNICODETEXT, hGlobal);
        ::CloseClipboard();
        
        return hResult != NULL;
    }
    
    // 如果锁定失败，释放内存并关闭剪贴板
    ::GlobalFree(hGlobal);
    ::CloseClipboard();
    return false;
}

std::wstring InputSimulator::GetClipboardText()
{
    std::wstring result;
    
    // 打开剪贴板
    if (!::OpenClipboard(NULL))
        return result;
    
    // 获取剪贴板数据
    HANDLE hData = ::GetClipboardData(CF_UNICODETEXT);
    if (hData)
    {
        // 锁定内存并复制数据
        wchar_t* pText = static_cast<wchar_t*>(::GlobalLock(hData));
        if (pText)
        {
            result = pText;
            ::GlobalUnlock(hData);
        }
    }
    
    ::CloseClipboard();
    return result;
}

bool InputSimulator::PasteText()
{
    // 使用Ctrl+V模拟粘贴
    std::vector<WORD> keys = { VK_CONTROL, 'V' };
    return SendKeyCombo(keys);
}

bool InputSimulator::CopyText()
{
    // 使用Ctrl+C模拟复制
    std::vector<WORD> keys = { VK_CONTROL, 'C' };
    return SendKeyCombo(keys);
}

bool InputSimulator::SendWindowMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (!hwnd || !::IsWindow(hwnd))
        return false;
    
    return ::SendMessage(hwnd, msg, wParam, lParam) != 0;
}

void InputSimulator::Sleep(DWORD milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

bool InputSimulator::SendKeyDown(WORD vkCode, bool isExtended)
{
    INPUT input = {};
    DWORD flags = KEYEVENTF_SCANCODE;
    
    if (isExtended)
        flags |= KEYEVENTF_EXTENDEDKEY;
    
    PrepareInput(input, vkCode, flags);
    
    return ::SendInput(1, &input, sizeof(INPUT)) == 1;
}

bool InputSimulator::SendKeyUp(WORD vkCode, bool isExtended)
{
    INPUT input = {};
    DWORD flags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
    
    if (isExtended)
        flags |= KEYEVENTF_EXTENDEDKEY;
    
    PrepareInput(input, vkCode, flags);
    
    return ::SendInput(1, &input, sizeof(INPUT)) == 1;
}

void InputSimulator::PrepareInput(INPUT& input, WORD vkCode, DWORD flags)
{
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vkCode;
    input.ki.wScan = static_cast<WORD>(::MapVirtualKey(vkCode, MAPVK_VK_TO_VSC));
    input.ki.dwFlags = flags;
    input.ki.time = 0;
    input.ki.dwExtraInfo = 0;
} 