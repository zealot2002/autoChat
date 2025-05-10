#pragma once
#include <Windows.h>
#include <string>
#include <vector>

// 键盘和鼠标操作的模拟器类
class InputSimulator
{
public:
    InputSimulator();
    ~InputSimulator();
    
    // 模拟键盘按键
    bool SendKey(WORD vkCode, bool isExtended = false);
    
    // 模拟组合键 (如Ctrl+C)
    bool SendKeyCombo(const std::vector<WORD>& vkCodes);
    
    // 模拟文本输入
    bool SendText(const std::wstring& text);
    
    // 模拟鼠标左键点击
    bool ClickLeftMouse(int x, int y);
    
    // 模拟鼠标右键点击
    bool ClickRightMouse(int x, int y);
    
    // 模拟鼠标双击
    bool DoubleClickMouse(int x, int y);
    
    // 模拟鼠标移动
    bool MoveMouse(int x, int y);
    
    // 模拟剪贴板操作 - 设置文本
    bool SetClipboardText(const std::wstring& text);
    
    // 模拟剪贴板操作 - 获取文本
    std::wstring GetClipboardText();
    
    // 模拟粘贴操作
    bool PasteText();
    
    // 模拟复制操作
    bool CopyText();
    
    // 发送Windows消息
    bool SendWindowMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
    // 等待操作间隔（毫秒）
    static void Sleep(DWORD milliseconds);
    
private:
    // 模拟按键按下
    bool SendKeyDown(WORD vkCode, bool isExtended = false);
    
    // 模拟按键释放
    bool SendKeyUp(WORD vkCode, bool isExtended = false);
    
    // 准备输入结构
    void PrepareInput(INPUT& input, WORD vkCode, DWORD flags);
    
    // 默认操作间隔（毫秒）
    static const DWORD DEFAULT_DELAY = 50;
}; 