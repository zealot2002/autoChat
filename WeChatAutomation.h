#pragma once
#include "AppFinder.h"
#include "WindowsController.h"
#include "InputSimulator.h"
#include <string>
#include <functional>

// 微信自动化操作的状态枚举
enum class WeChatAutomationState
{
    Idle,                // 空闲状态
    FindingWeChat,       // 查找微信窗口
    ActivatingWindow,    // 激活窗口
    OpeningSearch,       // 打开搜索
    SearchingContact,    // 搜索联系人
    SelectingContact,    // 选择联系人
    FocusingInputBox,    // 聚焦输入框
    InputtingMessage,    // 输入消息
    SendingMessage,      // 发送消息
    Completed,           // 完成
    Failed               // 失败
};

// 微信自动化操作的回调函数类型
using WeChatAutomationCallback = std::function<void(WeChatAutomationState, const std::wstring&)>;

// 微信自动化操作类
class WeChatAutomation
{
public:
    WeChatAutomation();
    ~WeChatAutomation();
    
    // 初始化
    bool Initialize();
    
    // 查找并激活微信窗口
    bool FindAndActivateWeChat(int timeoutMs = 5000);
    
    // 搜索联系人
    bool SearchContact(const std::wstring& contactName, int timeoutMs = 5000);
    
    // 选择第一个搜索结果
    bool SelectFirstSearchResult();
    
    // 聚焦到输入框
    bool FocusInputBox();
    
    // 发送消息
    bool SendMessage(const std::wstring& message);
    
    // 执行完整的发送消息流程
    bool ExecuteSendMessage(const std::wstring& contactName, const std::wstring& message, WeChatAutomationCallback callback = nullptr);
    
    // 获取当前状态
    WeChatAutomationState GetCurrentState() const;
    
    // 设置状态变更回调
    void SetStateChangeCallback(WeChatAutomationCallback callback);
    
private:
    // 等待时间（毫秒）
    static const int DEFAULT_WAIT_TIME = 1000;
    
    // 微信窗口类名（可能需要更新）
    static const std::wstring WECHAT_WINDOW_CLASS;
    static const std::wstring WECHAT_WINDOW_NAME;
    
    // 微信窗口操作所需的坐标和按键（这些值可能需要根据实际情况调整）
    static const int SEARCH_BUTTON_X_OFFSET = 50;  // 搜索按钮相对窗口左上角的X偏移
    static const int SEARCH_BUTTON_Y_OFFSET = 50;  // 搜索按钮相对窗口左上角的Y偏移
    static const int FIRST_RESULT_X_OFFSET = 200;  // 第一个搜索结果相对窗口左上角的X偏移
    static const int FIRST_RESULT_Y_OFFSET = 150;  // 第一个搜索结果相对窗口左上角的Y偏移
    static const int INPUT_BOX_X_OFFSET = 300;     // 输入框相对窗口左上角的X偏移
    static const int INPUT_BOX_Y_OFFSET = 500;     // 输入框相对窗口左上角的Y偏移
    static const int SEND_BUTTON_X_OFFSET = 500;   // 发送按钮相对窗口左上角的X偏移
    static const int SEND_BUTTON_Y_OFFSET = 500;   // 发送按钮相对窗口左上角的Y偏移
    
    // 状态更新函数
    void SetState(WeChatAutomationState newState, const std::wstring& message = L"");
    
    // 成员变量
    AppFinder m_appFinder;
    WindowsController m_windowController;
    InputSimulator m_inputSimulator;
    HWND m_weChatWindow;
    WeChatAutomationState m_currentState;
    WeChatAutomationCallback m_stateChangeCallback;
}; 